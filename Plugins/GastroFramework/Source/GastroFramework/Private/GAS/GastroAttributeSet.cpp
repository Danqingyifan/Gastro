#include "GAS/GastroAttributeSet.h"
#include "GAS/GastroAbilitySystemComponent.h"

UGastroAbilitySystemComponent* UGastroAttributeSet::GetGastroAbilitySystemComponent() const
{
    return Cast<UGastroAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
