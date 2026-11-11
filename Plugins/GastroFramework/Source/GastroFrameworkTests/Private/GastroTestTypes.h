#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/GastroGameplayAbility.h"
#include "GAS/GastroAttributeSet.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GastroTestTypes.generated.h"

class UGastroAbilitySystemComponent;
class UGastroAbilitySystemBindingComponent;
class UGastroAbilityInputComponent;
class UAbilityTask_WaitInputPress;
class UAbilityTask_WaitInputRelease;

UCLASS()
class UGastroTestObserver : public UObject
{
    GENERATED_BODY()
public:
    int32 ReadyCount = 0;
    int32 UnbindingCount = 0;
    TFunction<void()> OnReady;
    TFunction<void()> OnUnbinding;
    UFUNCTION()
    void Ready(UGastroAbilitySystemComponent* ASC) { ++ReadyCount; if (OnReady) { OnReady(); } }
    UFUNCTION()
    void Unbinding(UGastroAbilitySystemComponent* ASC) { ++UnbindingCount; if (OnUnbinding) { OnUnbinding(); } }
};

UCLASS()
class UGastroTestAttributes : public UGastroAttributeSet
{
    GENERATED_BODY()
public:
    UPROPERTY(ReplicatedUsing=OnRep_Value)
    FGameplayAttributeData Value;
    ATTRIBUTE_ACCESSORS_BASIC(UGastroTestAttributes, Value);
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UFUNCTION()
    void OnRep_Value(const FGameplayAttributeData& OldValue);
};

UCLASS()
class UGastroTestInitEffect : public UGameplayEffect
{
    GENERATED_BODY()
public:
    UGastroTestInitEffect();
};

UCLASS()
class UGastroTestDurationEffect : public UGameplayEffect
{
    GENERATED_BODY()
public:
    UGastroTestDurationEffect();
};

UCLASS()
class UGastroTestAbility : public UGastroGameplayAbility
{
    GENERATED_BODY()
public:
    UGastroTestAbility();
    int32 Activations = 0;
    int32 Presses = 0;
    int32 Releases = 0;
    int32 AvatarExits = 0;
    int32 Ends = 0;
    int32 PressTaskEvents = 0;
    int32 ReleaseTaskEvents = 0;
    TWeakObjectPtr<AActor> CachedAvatar;
    TFunction<void()> OnEnding;
    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitInputPress> PressTask;
    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitInputRelease> ReleaseTask;
    void StartInputTasks();
    void Finish() { K2_EndAbility(); }
    UFUNCTION()
    void PressTaskFired(float Time);
    UFUNCTION()
    void ReleaseTaskFired(float Time);
    virtual void OnAvatarUninitializing(AActor* OldAvatar) override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

UCLASS()
class UGastroTestPersistentAbility : public UGastroTestAbility
{
    GENERATED_BODY()
public:
    UGastroTestPersistentAbility() { bEndOnAvatarUnbound = false; }
};

UCLASS()
class AGastroTestActor : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AGastroTestActor();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UPROPERTY()
    TObjectPtr<UGastroAbilitySystemComponent> ASC;
    UPROPERTY()
    TObjectPtr<UGastroAbilitySystemBindingComponent> Binding;
    UPROPERTY()
    TObjectPtr<UGastroTestAttributes> Attributes;
};

/** All consumer Actors below inherit UE types directly. */
UCLASS()
class AGastroConsumerCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AGastroConsumerCharacter();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UPROPERTY()
    TObjectPtr<UGastroAbilitySystemComponent> ASC;
    UPROPERTY()
    TObjectPtr<UGastroAbilitySystemBindingComponent> Binding;
    UPROPERTY()
    TObjectPtr<UGastroTestAttributes> Attributes;
};

UCLASS()
class AGastroConsumerExternalCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AGastroConsumerExternalCharacter();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UPROPERTY()
    TObjectPtr<UGastroAbilitySystemBindingComponent> Binding;
};

UCLASS()
class AGastroConsumerPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AGastroConsumerPlayerState();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UPROPERTY()
    TObjectPtr<UGastroAbilitySystemComponent> ASC;
    UPROPERTY()
    TObjectPtr<UGastroTestAttributes> Attributes;
};

UCLASS()
class AGastroConsumerController : public APlayerController
{
    GENERATED_BODY()
public:
    AGastroConsumerController();
    UPROPERTY()
    TObjectPtr<UGastroAbilityInputComponent> AbilityInput;
};
