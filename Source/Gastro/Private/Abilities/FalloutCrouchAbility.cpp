#include "Abilities/FalloutCrouchAbility.h"
#include "Character/FalloutCharacter.h"

UFalloutCrouchAbility::UFalloutCrouchAbility() { NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; }

void UFalloutCrouchAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Data)
{
    auto* Character = Cast<AFalloutCharacter>(Info->AvatarActor.Get());
    // Re-pressing while a low ceiling blocks standing must restore the crouch intention.
    if (!Character || (!Character->bIsCrouched && !Character->CanCrouch()) || !CommitAbility(Handle, Info, Activation))
    { EndAbility(Handle, Info, Activation, true, true); return; }
    Character->Crouch();
}

void UFalloutCrouchAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation) { EndAbility(Handle, Info, Activation, true, false); }

void UFalloutCrouchAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo Activation, bool bReplicateEnd, bool bCancelled)
{
    if (Info) if (auto* Character = Cast<AFalloutCharacter>(Info->AvatarActor.Get())) Character->UnCrouch();
    Super::EndAbility(Handle, Info, Activation, bReplicateEnd, bCancelled);
}
