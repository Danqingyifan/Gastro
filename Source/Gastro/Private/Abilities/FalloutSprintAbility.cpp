#include "Abilities/FalloutSprintAbility.h"
#include "Character/FalloutCharacter.h"

UFalloutSprintAbility::UFalloutSprintAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UFalloutSprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    const auto* Character = ActorInfo ? Cast<AFalloutCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
    return Character && Character->CanStartSprint() &&
        Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UFalloutSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    auto* Character = ActorInfo ? Cast<AFalloutCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
    if (!Character || !CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    Character->SetSprintRequested(true);
}

void UFalloutSprintAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UFalloutSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo)
        if (auto* Character = Cast<AFalloutCharacter>(ActorInfo->AvatarActor.Get())) Character->SetSprintRequested(false);
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
