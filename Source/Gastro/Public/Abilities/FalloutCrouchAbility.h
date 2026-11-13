#pragma once
#include "CoreMinimal.h"
#include "GAS/GastroGameplayAbility.h"
#include "FalloutCrouchAbility.generated.h"

UCLASS()
class GASTRO_API UFalloutCrouchAbility : public UGastroGameplayAbility
{
    GENERATED_BODY()
public:
    UFalloutCrouchAbility();
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Data) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, bool bReplicateEnd, bool bCancelled) override;
};
