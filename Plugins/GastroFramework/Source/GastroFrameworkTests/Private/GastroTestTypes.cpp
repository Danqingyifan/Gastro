#include "GastroTestTypes.h"

#include "GAS/GastroAbilitySystemComponent.h"
#include "Components/GastroAbilitySystemBindingComponent.h"
#include "Input/GastroAbilityInputComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Net/UnrealNetwork.h"

void UGastroTestAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UGastroTestAttributes, Value, COND_None, REPNOTIFY_Always);
}

void UGastroTestAttributes::OnRep_Value(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGastroTestAttributes, Value, OldValue);
}

UGastroTestInitEffect::UGastroTestInitEffect()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;
    FGameplayModifierInfo Modifier;
    Modifier.Attribute = UGastroTestAttributes::GetValueAttribute();
    Modifier.ModifierOp = EGameplayModOp::Additive;
    Modifier.ModifierMagnitude = FScalableFloat(25.0f);
    Modifiers.Add(Modifier);
}

UGastroTestDurationEffect::UGastroTestDurationEffect()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;
}

UGastroTestAbility::UGastroTestAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGastroTestAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    ++Activations;
    CachedAvatar = ActorInfo->AvatarActor;
}

void UGastroTestAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    ++Presses;
}

void UGastroTestAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    ++Releases;
}

void UGastroTestAbility::OnAvatarUninitializing(AActor* OldAvatar)
{
    ++AvatarExits;
    CachedAvatar.Reset();
    Super::OnAvatarUninitializing(OldAvatar);
}

void UGastroTestAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ++Ends;
    if (OnEnding) { OnEnding(); }
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGastroTestAbility::StartInputTasks()
{
    PressTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
    PressTask->OnPress.AddDynamic(this, &ThisClass::PressTaskFired);
    PressTask->ReadyForActivation();
    ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
    ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::ReleaseTaskFired);
    ReleaseTask->ReadyForActivation();
}

void UGastroTestAbility::PressTaskFired(float Time) { ++PressTaskEvents; }
void UGastroTestAbility::ReleaseTaskFired(float Time) { ++ReleaseTaskEvents; }

AGastroTestActor::AGastroTestActor()
{
    ASC = CreateDefaultSubobject<UGastroAbilitySystemComponent>(TEXT("ASC"));
    Binding = CreateDefaultSubobject<UGastroAbilitySystemBindingComponent>(TEXT("Binding"));
    Attributes = CreateDefaultSubobject<UGastroTestAttributes>(TEXT("Attributes"));
}
UAbilitySystemComponent* AGastroTestActor::GetAbilitySystemComponent() const { return ASC; }

AGastroConsumerCharacter::AGastroConsumerCharacter()
{
    ASC = CreateDefaultSubobject<UGastroAbilitySystemComponent>(TEXT("ASC"));
    Binding = CreateDefaultSubobject<UGastroAbilitySystemBindingComponent>(TEXT("Binding"));
    Attributes = CreateDefaultSubobject<UGastroTestAttributes>(TEXT("Attributes"));
}
UAbilitySystemComponent* AGastroConsumerCharacter::GetAbilitySystemComponent() const { return ASC; }

AGastroConsumerExternalCharacter::AGastroConsumerExternalCharacter()
{
    Binding = CreateDefaultSubobject<UGastroAbilitySystemBindingComponent>(TEXT("Binding"));
}
UAbilitySystemComponent* AGastroConsumerExternalCharacter::GetAbilitySystemComponent() const
{
    return Binding->GetAbilitySystemComponent();
}

AGastroConsumerPlayerState::AGastroConsumerPlayerState()
{
    ASC = CreateDefaultSubobject<UGastroAbilitySystemComponent>(TEXT("ASC"));
    Attributes = CreateDefaultSubobject<UGastroTestAttributes>(TEXT("Attributes"));
}
UAbilitySystemComponent* AGastroConsumerPlayerState::GetAbilitySystemComponent() const { return ASC; }

AGastroConsumerController::AGastroConsumerController()
{
    AbilityInput = CreateDefaultSubobject<UGastroAbilityInputComponent>(TEXT("AbilityInput"));
}
