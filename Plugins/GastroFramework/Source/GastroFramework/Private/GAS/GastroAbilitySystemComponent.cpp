#include "GAS/GastroAbilitySystemComponent.h"
#include "GAS/GastroGameplayAbility.h"
#include "GAS/GastroStartupValidation.h"
#include "Components/GastroAbilitySystemBindingComponent.h"
#include "GameplayEffect.h"

UGastroAbilitySystemComponent::UGastroAbilitySystemComponent()
{
    SetIsReplicatedByDefault(true);
    SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

bool UGastroAbilitySystemComponent::InitializeOwnerState()
{
    if (bOwnerStateInitialized || !IsOwnerActorAuthoritative()) return true;
    if (bInitializingOwnerState || !IsRegistered() || !HasBeenInitialized()) return false;
    if (!GastroValidateStartupSets(OwnerAbilitySets)) return false;
    if (OwnerInitializationEffect && (OwnerInitializationEffect->HasAnyClassFlags(CLASS_Abstract) ||
        OwnerInitializationEffect->GetDefaultObject<UGameplayEffect>()->DurationPolicy != EGameplayEffectDurationType::Instant))
    {
        UE_LOG(LogGastroFramework, Warning, TEXT("Owner initialization effect must be Instant and concrete."));
        return false;
    }

    TGuardValue<bool> Guard(bInitializingOwnerState, true);
    FGameplayEffectSpecHandle InitializationSpec;
    if (OwnerInitializationEffect)
    {
        FGameplayEffectContextHandle Context = MakeEffectContext();
        Context.AddSourceObject(GetOwner());
        InitializationSpec = MakeOutgoingSpec(OwnerInitializationEffect, 1.0f, Context);
        if (!InitializationSpec.IsValid()) return false;
    }

    TSet<const UGastroAbilitySet*> Applied;
    FGastroAbilitySetGrantedHandles Pending;
    for (const UGastroAbilitySet* Set : OwnerAbilitySets)
    {
        if (!Applied.Contains(Set))
        {
            if (!Set->GiveToAbilitySystem(this, Pending, GetOwner()))
            {
                Pending.TakeFromAbilitySystem();
                return false;
            }
            Applied.Add(Set);
        }
    }
    OwnerGrantedHandles = MoveTemp(Pending);
    // Mark before attribute-change callbacks can reenter initialization.
    bOwnerStateInitialized = true;
    if (InitializationSpec.IsValid()) ApplyGameplayEffectSpecToSelf(*InitializationSpec.Data.Get());
    return true;
}

UGastroAbilitySystemBindingComponent* UGastroAbilitySystemComponent::GetAvatarBinding() const
{
    return AvatarBinding.Get();
}

bool UGastroAbilitySystemComponent::IsAvatarReady() const
{
    const auto* Binding = AvatarBinding.Get();
    return Binding && Binding->IsAbilitySystemReady();
}

void UGastroAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (!InputTag.IsValid() || !IsAvatarReady()) return;
    ABILITYLIST_SCOPE_LOCK();
    for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (Spec.Ability && !Spec.PendingRemove && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            Spec.InputPressed = true;
            if (Spec.IsActive()) AbilitySpecInputPressed(Spec);
            else TryActivateAbility(Spec.Handle);
            if (!IsAvatarReady()) break;
        }
    }
}

void UGastroAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (!InputTag.IsValid()) return;
    ABILITYLIST_SCOPE_LOCK();
    for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (Spec.Ability && !Spec.PendingRemove && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            Spec.InputPressed = false;
            if (Spec.IsActive()) AbilitySpecInputReleased(Spec);
        }
    }
}

void UGastroAbilitySystemComponent::SendInputEvent(FGameplayAbilitySpecHandle Handle, EAbilityGenericReplicatedEvent::Type Event)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
    if (!Spec || !Spec->IsActive()) return;
    TArray<FPredictionKey> Keys;
    for (UGameplayAbility* Instance : Spec->GetAbilityInstances())
    {
        if (IsValid(Instance) && Instance->IsActive()) Keys.AddUnique(Instance->GetCurrentActivationInfo().GetActivationPredictionKey());
    }
    for (const FPredictionKey& Key : Keys) InvokeReplicatedEvent(Event, Handle, Key);
}

void UGastroAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
    ABILITYLIST_SCOPE_LOCK();
    const auto Handle = Spec.Handle;
    Super::AbilitySpecInputPressed(Spec);
    SendInputEvent(Handle, EAbilityGenericReplicatedEvent::InputPressed);
}

void UGastroAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
    ABILITYLIST_SCOPE_LOCK();
    const auto Handle = Spec.Handle;
    Super::AbilitySpecInputReleased(Spec);
    SendInputEvent(Handle, EAbilityGenericReplicatedEvent::InputReleased);
}

void UGastroAbilitySystemComponent::NotifyAvatarUninitializing(AActor* OldAvatar)
{
    ABILITYLIST_SCOPE_LOCK();
    for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if (Spec.InputPressed) AbilitySpecInputReleased(Spec);
        Spec.InputPressed = false;
        const auto Instances = Spec.GetAbilityInstances();
        for (UGameplayAbility* Instance : Instances)
        {
            if (auto* Ability = Cast<UGastroGameplayAbility>(Instance)) Ability->OnAvatarUninitializing(OldAvatar);
        }
    }
}

void UGastroAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (auto* Binding = AvatarBinding.Get()) Binding->UninitializeAbilitySystem();
    OwnerGrantedHandles.TakeFromAbilitySystem();
    Super::EndPlay(EndPlayReason);
}
