#include "Components/GastroAbilitySystemBindingComponent.h"
#include "GAS/GastroAbilitySystemComponent.h"
#include "GAS/GastroStartupValidation.h"

UGastroAbilitySystemBindingComponent::UGastroAbilitySystemBindingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UGastroAbilitySystemBindingComponent::InitializeAbilitySystem(UGastroAbilitySystemComponent* ASC, AActor* OwnerActor)
{
    if (bTransitioning || !IsRegistered() || !IsValid(ASC) || !IsValid(OwnerActor) || !IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed() ||
        ASC->GetOwner() != OwnerActor || !ASC->IsRegistered() || !ASC->HasBeenInitialized()) return false;

    if (IsAbilitySystemReady() && BoundASC == ASC && BoundOwner == OwnerActor)
    {
        ASC->RefreshAbilityActorInfo();
        return true;
    }
    if (OwnerActor->HasAuthority() && !GastroValidateStartupSets(AvatarAbilitySets)) return false;
    UninitializeAbilitySystem();
    TGuardValue<bool> Guard(bTransitioning, true);
    bUninitializeRequested = false;
    if (auto* Previous = ASC->GetAvatarBinding())
    {
        Previous->UninitializeAbilitySystem();
        // A transition callback may have refused cleanup or installed another association.
        if (ASC->GetAvatarBinding()) return false;
    }

    BoundASC = ASC;
    BoundOwner = OwnerActor;
    BoundGeneration = ++ASC->BindingGeneration;
    ASC->AvatarBinding = this;
    ASC->InitAbilityActorInfo(OwnerActor, GetOwner());
    bool bInitialized = ASC->InitializeOwnerState();
    if (bInitialized && !bUninitializeRequested && IsValid(OwnerActor) && OwnerActor->HasAuthority())
    {
        TSet<const UGastroAbilitySet*> Applied;
        for (const UGastroAbilitySet* Set : AvatarAbilitySets)
        {
            if (!Applied.Contains(Set))
            {
                if (!Set->GiveToAbilitySystem(ASC, AvatarGrantedHandles, GetOwner())) { bInitialized = false; break; }
                Applied.Add(Set);
            }
        }
    }
    if (!bInitialized || bUninitializeRequested || !IsValid(GetOwner()) || GetOwner()->IsActorBeingDestroyed() ||
        !IsValid(ASC) || ASC->GetAvatarBinding() != this)
    {
        AvatarGrantedHandles.TakeFromAbilitySystem();
        if (IsValid(ASC) && ASC->GetAvatarBinding() == this)
        {
            ASC->AvatarBinding.Reset();
            ASC->SetAvatarActor(nullptr);
        }
        BoundASC.Reset();
        BoundOwner.Reset();
        return false;
    }
    bReady = true;
    // Ready callbacks may immediately unbind. Permit that, but never report stale readiness.
    bTransitioning = false;
    const uint64 ThisGeneration = BoundGeneration;
    OnAbilitySystemReady.Broadcast(ASC);
    return IsAbilitySystemReady() && BoundGeneration == ThisGeneration;
}

void UGastroAbilitySystemBindingComponent::UninitializeAbilitySystem()
{
    if (bTransitioning) { bUninitializeRequested = true; return; }
    TGuardValue<bool> Guard(bTransitioning, true);
    auto* ASC = BoundASC.Get();
    const uint64 ThisGeneration = BoundGeneration;
    const bool bOwnsAssociation = ASC && ASC->GetAvatarBinding() == this && ASC->BindingGeneration == ThisGeneration;
    const bool bWasReady = bReady;
    bReady = false;
    if (bOwnsAssociation) ASC->OnAvatarUninitializing.Broadcast();
    if (bWasReady && ASC) OnAbilitySystemUninitializing.Broadcast(ASC);
    if (bOwnsAssociation && IsValid(ASC)) ASC->NotifyAvatarUninitializing(GetOwner());
    AvatarGrantedHandles.TakeFromAbilitySystem();
    if (IsValid(ASC) && ASC->GetAvatarBinding() == this && ASC->BindingGeneration == ThisGeneration)
    {
        ASC->AvatarBinding.Reset();
        ++ASC->BindingGeneration;
        if (ASC->GetAvatarActor() == GetOwner()) ASC->SetAvatarActor(nullptr);
    }
    BoundASC.Reset();
    BoundOwner.Reset();
    bUninitializeRequested = false;
}

UGastroAbilitySystemComponent* UGastroAbilitySystemBindingComponent::GetAbilitySystemComponent() const { return BoundASC.Get(); }
AActor* UGastroAbilitySystemBindingComponent::GetAbilitySystemOwner() const { return BoundOwner.Get(); }

bool UGastroAbilitySystemBindingComponent::IsAbilitySystemReady() const
{
    const auto* ASC = BoundASC.Get();
    return bReady && IsValid(GetOwner()) && !GetOwner()->IsActorBeingDestroyed() && ASC && ASC->GetAvatarBinding() == this && ASC->GetBindingGeneration() == BoundGeneration &&
        ASC->GetAvatarActor() == GetOwner() && ASC->GetOwnerActor() == BoundOwner.Get();
}

void UGastroAbilitySystemBindingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UninitializeAbilitySystem();
    Super::EndPlay(EndPlayReason);
}

void UGastroAbilitySystemBindingComponent::OnUnregister()
{
    UninitializeAbilitySystem();
    Super::OnUnregister();
}
