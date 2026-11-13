#pragma once
#include "CoreMinimal.h"
#include "GAS/GastroGameplayAbility.h"
#include "FalloutJumpAbility.generated.h"

UCLASS()
class GASTRO_API UFalloutJumpAbility : public UGastroGameplayAbility
{
    GENERATED_BODY()
public:
    UFalloutJumpAbility();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source = nullptr, const FGameplayTagContainer* Target = nullptr,
        FGameplayTagContainer* Relevant = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Data) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, bool bReplicateEnd, bool bCancelled) override;
};
