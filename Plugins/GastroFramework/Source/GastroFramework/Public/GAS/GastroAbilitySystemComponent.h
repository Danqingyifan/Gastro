#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/GastroAbilitySet.h"
#include "GastroAbilitySystemComponent.generated.h"

class UGastroAbilitySystemBindingComponent;
DECLARE_MULTICAST_DELEGATE(FGastroAvatarUninitializing);

/** Owns persistent GAS state. Avatar attachment is coordinated by the binding component. */
UCLASS(ClassGroup=(Gastro), meta=(BlueprintSpawnableComponent))
class GASTROFRAMEWORK_API UGastroAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()
public:
    UGastroAbilitySystemComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gastro|Startup")
    TArray<TObjectPtr<UGastroAbilitySet>> OwnerAbilitySets;

    /** Optional Instant GE, applied once on authority after attributes are registered. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gastro|Startup")
    TSubclassOf<UGameplayEffect> OwnerInitializationEffect;

    bool InitializeOwnerState();
    bool IsOwnerStateInitialized() const { return bOwnerStateInitialized; }
    bool IsAvatarReady() const;
    UGastroAbilitySystemBindingComponent* GetAvatarBinding() const;
    uint64 GetBindingGeneration() const { return BindingGeneration; }
    /** Explicit consumers can observe detach even when injected before an Avatar was available. */
    FGastroAvatarUninitializing OnAvatarUninitializing;

    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void AbilityInputTagPressed(FGameplayTag InputTag);
    UFUNCTION(BlueprintCallable, Category="Gastro|Input")
    void AbilityInputTagReleased(FGameplayTag InputTag);

    virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
    virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    friend class UGastroAbilitySystemBindingComponent;
    void NotifyAvatarUninitializing(AActor* OldAvatar);
    void SendInputEvent(FGameplayAbilitySpecHandle Handle, EAbilityGenericReplicatedEvent::Type Event);

    UPROPERTY(Transient)
    FGastroAbilitySetGrantedHandles OwnerGrantedHandles;
    UPROPERTY(Transient)
    TWeakObjectPtr<UGastroAbilitySystemBindingComponent> AvatarBinding;
    uint64 BindingGeneration = 0;
    bool bOwnerStateInitialized = false;
    bool bInitializingOwnerState = false;
};
