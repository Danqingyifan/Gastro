#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GastroGameplayAbility.generated.h"

class UGastroAbilitySystemComponent;

UCLASS(Abstract, Blueprintable)
class GASTROFRAMEWORK_API UGastroGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UGastroGameplayAbility();

    /** Opt out only for abilities that can run without an avatar. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gastro|Lifecycle")
    bool bEndOnAvatarUnbound = true;

    UFUNCTION(BlueprintPure, Category="Gastro|Ability")
    UGastroAbilitySystemComponent* GetGastroAbilitySystemComponent() const;

    /** Called on every instantiated ability, including inactive instances. Clear cached avatar references here. */
    virtual void OnAvatarUninitializing(AActor* OldAvatar);
    UFUNCTION(BlueprintImplementableEvent, Category="Gastro|Lifecycle", meta=(DisplayName="On Avatar Uninitializing"))
    void BP_OnAvatarUninitializing(AActor* OldAvatar);
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
        FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
