#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GastroAbilityInputComponent.generated.h"

class UGastroAbilitySystemComponent;
class UGastroAbilitySystemBindingComponent;
class UGastroInputConfig;
class UEnhancedInputComponent;
class APawn;

/** Optional Controller integration plus explicit ASC injection for any consumer. */
UCLASS(ClassGroup=(Gastro), meta=(BlueprintSpawnableComponent))
class GASTROFRAMEWORK_API UGastroAbilityInputComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UGastroAbilityInputComponent();
    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void BindInputs(UEnhancedInputComponent* Input, const UGastroInputConfig* Config);
    /** Explicit injection disables automatic Pawn tracking until FollowControllerPawn is called. */
    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void SetAbilitySystemComponent(UGastroAbilitySystemComponent* ASC);
    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void FollowControllerPawn();
    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void ReleaseAllInputs();
    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void UnbindInputs();

protected:
    virtual void OnRegister() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnUnregister() override;

private:
    void DisconnectPawn();
    void ChangeASC(UGastroAbilitySystemComponent* ASC);
    void ExplicitAvatarUninitializing();
    void Press(FGameplayTag Tag);
    void Release(FGameplayTag Tag);
    UFUNCTION()
    void PawnChanged(APawn* OldPawn, APawn* NewPawn);
    UFUNCTION()
    void AbilitySystemReady(UGastroAbilitySystemComponent* ASC);
    UFUNCTION()
    void AbilitySystemUninitializing(UGastroAbilitySystemComponent* ASC);

    UPROPERTY(Transient)
    TObjectPtr<const UGastroInputConfig> InputConfig;
    UPROPERTY(Transient)
    TWeakObjectPtr<UEnhancedInputComponent> BoundInput;
    UPROPERTY(Transient)
    TWeakObjectPtr<UGastroAbilitySystemComponent> TargetASC;
    UPROPERTY(Transient)
    TWeakObjectPtr<UGastroAbilitySystemBindingComponent> PawnBinding;
    TArray<uint32> BindingHandles;
    TMap<FGameplayTag, TWeakObjectPtr<UGastroAbilitySystemComponent>> PressedInputs;
    bool bFollowPawn = true;
};
