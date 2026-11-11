#pragma once

#include "Characters/GastroBaseCharacter.h"
#include "GastroCharacterWithExternalASC.generated.h"

UCLASS()
class GASTROFRAMEWORK_API AGastroCharacterWithExternalASC : public AGastroBaseCharacter
{
    GENERATED_BODY()
public:
    /** Null restores the default PlayerState lookup. */
    UFUNCTION(BlueprintCallable, Category="Gastro|Lifecycle")
    void SetExternalAbilitySystemComponent(UGastroAbilitySystemComponent* ASC);
protected:
    virtual UGastroAbilitySystemComponent* ResolveAbilitySystemComponent() const override;
private:
    UPROPERTY(Transient)
    TWeakObjectPtr<UGastroAbilitySystemComponent> ExternalASC;
};
