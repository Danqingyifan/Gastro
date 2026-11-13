#include "Abilities/FalloutJumpAbility.h"
#include "Character/FalloutCharacter.h"

UFalloutJumpAbility::UFalloutJumpAbility() { NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; }

bool UFalloutJumpAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source, const FGameplayTagContainer* Target, FGameplayTagContainer* Relevant) const
{
    const auto* Character = Info ? Cast<AFalloutCharacter>(Info->AvatarActor.Get()) : nullptr;
    return Character && Character->CanJump() && Super::CanActivateAbility(Handle, Info, Source, Target, Relevant);
}

void UFalloutJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Data)
{
    auto* Character = Cast<AFalloutCharacter>(Info->AvatarActor.Get());
    if (!Character || !CommitAbility(Handle, Info, Activation)) { EndAbility(Handle, Info, Activation, true, true); return; }
    Character->Jump();
}

void UFalloutJumpAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation) { EndAbility(Handle, Info, Activation, true, false); }

void UFalloutJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, bool bReplicateEnd, bool bCancelled)
{
    if (Info) if (auto* Character = Cast<AFalloutCharacter>(Info->AvatarActor.Get())) Character->StopJumping();
    Super::EndAbility(Handle, Info, Activation, bReplicateEnd, bCancelled);
}
