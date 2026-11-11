#pragma once

#include "Characters/GastroBaseCharacter.h"
#include "GastroCharacterWithOwnedASC.generated.h"

UCLASS()
class GASTROFRAMEWORK_API AGastroCharacterWithOwnedASC : public AGastroBaseCharacter
{
    GENERATED_BODY()
public:
    AGastroCharacterWithOwnedASC();
protected:
    virtual UGastroAbilitySystemComponent* ResolveAbilitySystemComponent() const override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Gastro")
    TObjectPtr<UGastroAbilitySystemComponent> OwnedASC;
};
