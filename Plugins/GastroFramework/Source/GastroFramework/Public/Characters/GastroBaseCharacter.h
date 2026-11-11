#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GastroBaseCharacter.generated.h"

class UGastroAbilitySystemComponent;
class UGastroAbilitySystemBindingComponent;

/** Convenience forwarding only. Components can also be used on ordinary UE Actors. */
UCLASS(Abstract)
class GASTROFRAMEWORK_API AGastroBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AGastroBaseCharacter();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UGastroAbilitySystemBindingComponent* GetAbilitySystemBinding() const { return AbilityBinding; }
    void TryInitializeAbilitySystem();
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
    virtual void OnRep_Controller() override;
    virtual void OnRep_PlayerState() override;
    virtual void PawnClientRestart() override;

protected:
    virtual void BeginPlay() override;
    virtual UGastroAbilitySystemComponent* ResolveAbilitySystemComponent() const PURE_VIRTUAL(AGastroBaseCharacter::ResolveAbilitySystemComponent, return nullptr;);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gastro")
    TObjectPtr<UGastroAbilitySystemBindingComponent> AbilityBinding;
};
