#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GastroPlayerState.generated.h"

class UGastroAbilitySystemComponent;

UCLASS()
class GASTROFRAMEWORK_API AGastroPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    AGastroPlayerState();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gastro")
    TObjectPtr<UGastroAbilitySystemComponent> AbilitySystem;
};
