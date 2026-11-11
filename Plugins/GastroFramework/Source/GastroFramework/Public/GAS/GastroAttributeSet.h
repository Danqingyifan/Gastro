#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GastroAttributeSet.generated.h"

class UGastroAbilitySystemComponent;

/** Consumer-defined attributes belong on default subobjects of the ASC's owning Actor. */
UCLASS(Abstract)
class GASTROFRAMEWORK_API UGastroAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category="Gastro|Attributes")
    UGastroAbilitySystemComponent* GetGastroAbilitySystemComponent() const;
};
