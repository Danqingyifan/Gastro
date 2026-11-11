#include "Characters/GastroCharacterWithOwnedASC.h"
#include "GAS/GastroAbilitySystemComponent.h"

AGastroCharacterWithOwnedASC::AGastroCharacterWithOwnedASC()
{
    OwnedASC = CreateDefaultSubobject<UGastroAbilitySystemComponent>(TEXT("ASC"));
}
UGastroAbilitySystemComponent* AGastroCharacterWithOwnedASC::ResolveAbilitySystemComponent() const { return OwnedASC; }
