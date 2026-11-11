#include "Input/GastroAbilityInputComponent.h"
#include "Input/GastroInputConfig.h"
#include "GAS/GastroAbilitySystemComponent.h"
#include "Components/GastroAbilitySystemBindingComponent.h"
#include "GastroFrameworkLog.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UGastroAbilityInputComponent::UGastroAbilityInputComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGastroAbilityInputComponent::OnRegister()
{
    Super::OnRegister();
    if (auto* Controller = Cast<APlayerController>(GetOwner()))
    {
        Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::PawnChanged);
        if (bFollowPawn) PawnChanged(nullptr, Controller->GetPawn());
    }
}

void UGastroAbilityInputComponent::BindInputs(UEnhancedInputComponent* Input, const UGastroInputConfig* Config)
{
    UnbindInputs();
    if (!IsValid(Input) || !IsValid(Config)) return;
    BoundInput = Input;
    InputConfig = Config;
    TSet<const UInputAction*> Actions;
    TSet<FGameplayTag> Tags;
    for (const FGastroAbilityInputAction& Mapping : Config->AbilityInputActions)
    {
        if (!Mapping.IsValid() || Actions.Contains(Mapping.InputAction) || Tags.Contains(Mapping.InputTag))
        {
            UE_LOG(LogGastroFramework, Warning, TEXT("InputConfig %s: skipping invalid or duplicate ability input mapping."), *GetNameSafe(Config));
            continue;
        }
        Actions.Add(Mapping.InputAction);
        Tags.Add(Mapping.InputTag);
        BindingHandles.Add(Input->BindAction(Mapping.InputAction, ETriggerEvent::Started, this, &ThisClass::Press, Mapping.InputTag).GetHandle());
        BindingHandles.Add(Input->BindAction(Mapping.InputAction, ETriggerEvent::Completed, this, &ThisClass::Release, Mapping.InputTag).GetHandle());
        BindingHandles.Add(Input->BindAction(Mapping.InputAction, ETriggerEvent::Canceled, this, &ThisClass::Release, Mapping.InputTag).GetHandle());
    }
}

void UGastroAbilityInputComponent::ChangeASC(UGastroAbilitySystemComponent* ASC)
{
    if (TargetASC.Get() == ASC) return;
    ReleaseAllInputs();
    if (auto* Previous = TargetASC.Get()) Previous->OnAvatarUninitializing.RemoveAll(this);
    TargetASC = ASC;
    if (ASC && !bFollowPawn) ASC->OnAvatarUninitializing.AddUObject(this, &ThisClass::ExplicitAvatarUninitializing);
}

void UGastroAbilityInputComponent::ExplicitAvatarUninitializing() { ReleaseAllInputs(); }

void UGastroAbilityInputComponent::DisconnectPawn()
{
    if (auto* Binding = PawnBinding.Get())
    {
        Binding->OnAbilitySystemReady.RemoveDynamic(this, &ThisClass::AbilitySystemReady);
        Binding->OnAbilitySystemUninitializing.RemoveDynamic(this, &ThisClass::AbilitySystemUninitializing);
    }
    PawnBinding.Reset();
}

void UGastroAbilityInputComponent::SetAbilitySystemComponent(UGastroAbilitySystemComponent* ASC)
{
    ReleaseAllInputs();
    DisconnectPawn();
    bFollowPawn = false;
    // Replacing automatic tracking with the same target still needs the ASC-level listener.
    if (auto* Previous = TargetASC.Get()) Previous->OnAvatarUninitializing.RemoveAll(this);
    TargetASC.Reset();
    ChangeASC(ASC);
    if (IsValid(ASC))
    {
        PawnBinding = ASC->GetAvatarBinding();
        if (auto* Binding = PawnBinding.Get())
        {
            Binding->OnAbilitySystemReady.AddUniqueDynamic(this, &ThisClass::AbilitySystemReady);
            Binding->OnAbilitySystemUninitializing.AddUniqueDynamic(this, &ThisClass::AbilitySystemUninitializing);
        }
    }
}

void UGastroAbilityInputComponent::FollowControllerPawn()
{
    ReleaseAllInputs();
    DisconnectPawn();
    if (auto* Previous = TargetASC.Get()) Previous->OnAvatarUninitializing.RemoveAll(this);
    bFollowPawn = true;
    auto* Controller = Cast<APlayerController>(GetOwner());
    PawnChanged(nullptr, Controller ? Controller->GetPawn() : nullptr);
}

void UGastroAbilityInputComponent::PawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    if (!bFollowPawn) return;
    ReleaseAllInputs();
    DisconnectPawn();
    ChangeASC(nullptr);
    if (IsValid(NewPawn))
    {
        PawnBinding = NewPawn->FindComponentByClass<UGastroAbilitySystemBindingComponent>();
        if (auto* Binding = PawnBinding.Get())
        {
            Binding->OnAbilitySystemReady.AddUniqueDynamic(this, &ThisClass::AbilitySystemReady);
            Binding->OnAbilitySystemUninitializing.AddUniqueDynamic(this, &ThisClass::AbilitySystemUninitializing);
            if (Binding->IsAbilitySystemReady()) ChangeASC(Binding->GetAbilitySystemComponent());
        }
    }
}

void UGastroAbilityInputComponent::AbilitySystemReady(UGastroAbilitySystemComponent* ASC)
{
    auto* Binding = PawnBinding.Get();
    if (Binding && Binding->IsAbilitySystemReady() && Binding->GetAbilitySystemComponent() == ASC &&
        (bFollowPawn || TargetASC.Get() == ASC)) ChangeASC(ASC);
}

void UGastroAbilityInputComponent::AbilitySystemUninitializing(UGastroAbilitySystemComponent* ASC)
{
    if (TargetASC.Get() == ASC)
    {
        ReleaseAllInputs();
        if (bFollowPawn) TargetASC.Reset();
    }
}

void UGastroAbilityInputComponent::Press(FGameplayTag Tag)
{
    auto* ASC = TargetASC.Get();
    if (!ASC || !ASC->IsAvatarReady() || PressedInputs.Contains(Tag)) return;
    PressedInputs.Add(Tag, ASC);
    ASC->AbilityInputTagPressed(Tag);
}

void UGastroAbilityInputComponent::Release(FGameplayTag Tag)
{
    TWeakObjectPtr<UGastroAbilitySystemComponent> OriginalASC;
    if (PressedInputs.RemoveAndCopyValue(Tag, OriginalASC))
    {
        if (auto* ASC = OriginalASC.Get()) ASC->AbilityInputTagReleased(Tag);
    }
}

void UGastroAbilityInputComponent::ReleaseAllInputs()
{
    const auto Inputs = MoveTemp(PressedInputs);
    PressedInputs.Reset();
    for (const auto& Entry : Inputs)
    {
        if (auto* ASC = Entry.Value.Get()) ASC->AbilityInputTagReleased(Entry.Key);
    }
}

void UGastroAbilityInputComponent::UnbindInputs()
{
    ReleaseAllInputs();
    if (auto* Input = BoundInput.Get())
    {
        for (uint32 Handle : BindingHandles) Input->RemoveBindingByHandle(Handle);
    }
    BindingHandles.Reset();
    BoundInput.Reset();
    InputConfig = nullptr;
}

void UGastroAbilityInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindInputs();
    DisconnectPawn();
    if (auto* Previous = TargetASC.Get()) Previous->OnAvatarUninitializing.RemoveAll(this);
    TargetASC.Reset();
    Super::EndPlay(EndPlayReason);
}

void UGastroAbilityInputComponent::OnUnregister()
{
    UnbindInputs();
    DisconnectPawn();
    if (auto* Previous = TargetASC.Get()) Previous->OnAvatarUninitializing.RemoveAll(this);
    TargetASC.Reset();
    if (auto* Controller = Cast<APlayerController>(GetOwner()))
        Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::PawnChanged);
    Super::OnUnregister();
}
