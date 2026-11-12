#include "GAS/GastroGameplayAbility.h"
#include "GAS/GastroAbilitySystemComponent.h"

UGastroGameplayAbility::UGastroGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

UGastroAbilitySystemComponent* UGastroGameplayAbility::GetGastroAbilitySystemComponent() const
{
    const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
    return Info ? Cast<UGastroAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
}

void UGastroGameplayAbility::OnAvatarUninitializing(AActor* OldAvatar)
{
    BP_OnAvatarUninitializing(OldAvatar);
    if (bEndOnAvatarUnbound && IsActive())
    {
        // Lifecycle termination is stronger than temporary gameplay cancellation restrictions.
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
    }
}

bool UGastroGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    const UGastroAbilitySystemComponent* ASC = ActorInfo ? Cast<UGastroAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()) : nullptr;
    return ASC && ASC->IsAvatarReady() && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}
