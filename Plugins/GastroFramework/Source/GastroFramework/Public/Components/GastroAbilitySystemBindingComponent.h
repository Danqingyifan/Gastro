#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/GastroAbilitySet.h"
#include "GastroAbilitySystemBindingComponent.generated.h"

class UGastroAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGastroAbilitySystemBindingEvent, UGastroAbilitySystemComponent*, ASC);

/** Composable avatar lifecycle; does not require a Gastro Actor superclass. */
UCLASS(ClassGroup=(Gastro), meta=(BlueprintSpawnableComponent))
class GASTROFRAMEWORK_API UGastroAbilitySystemBindingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UGastroAbilitySystemBindingComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gastro|Startup")
    TArray<TObjectPtr<UGastroAbilitySet>> AvatarAbilitySets;

    UPROPERTY(BlueprintAssignable, Category="Gastro|Lifecycle")
    FGastroAbilitySystemBindingEvent OnAbilitySystemReady;
    UPROPERTY(BlueprintAssignable, Category="Gastro|Lifecycle")
    FGastroAbilitySystemBindingEvent OnAbilitySystemUninitializing;

    /** Returns false when dependencies are unavailable or configuration is invalid. Retry on dependency changes. */
    UFUNCTION(BlueprintCallable, Category="Gastro|Lifecycle")
    bool InitializeAbilitySystem(UGastroAbilitySystemComponent* ASC, AActor* OwnerActor);
    UFUNCTION(BlueprintCallable, Category="Gastro|Lifecycle")
    void UninitializeAbilitySystem();
    UFUNCTION(BlueprintPure, Category="Gastro|Lifecycle")
    UGastroAbilitySystemComponent* GetAbilitySystemComponent() const;
    UFUNCTION(BlueprintPure, Category="Gastro|Lifecycle")
    AActor* GetAbilitySystemOwner() const;
    UFUNCTION(BlueprintPure, Category="Gastro|Lifecycle")
    bool IsAbilitySystemReady() const;

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnUnregister() override;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<UGastroAbilitySystemComponent> BoundASC;
    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> BoundOwner;
    UPROPERTY(Transient)
    FGastroAbilitySetGrantedHandles AvatarGrantedHandles;
    uint64 BoundGeneration = 0;
    bool bReady = false;
    bool bTransitioning = false;
    bool bUninitializeRequested = false;
};
