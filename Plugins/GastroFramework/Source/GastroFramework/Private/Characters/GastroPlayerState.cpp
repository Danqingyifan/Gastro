#include "Characters/GastroPlayerState.h"
#include "GAS/GastroAbilitySystemComponent.h"

AGastroPlayerState::AGastroPlayerState()
{
    AbilitySystem = CreateDefaultSubobject<UGastroAbilitySystemComponent>(TEXT("ASC"));
}
UAbilitySystemComponent* AGastroPlayerState::GetAbilitySystemComponent() const { return AbilitySystem; }
