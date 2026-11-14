#include "Abilities/FalloutAimAbility.h"
#include "Character/FalloutCharacter.h"

UFalloutAimAbility::UFalloutAimAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UFalloutAimAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    auto* Character = ActorInfo ? Cast<AFalloutCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
    if (!Character || !CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    Character->SetAiming(true);
}

void UFalloutAimAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UFalloutAimAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo)
        if (auto* Character = Cast<AFalloutCharacter>(ActorInfo->AvatarActor.Get())) Character->SetAiming(false);
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
