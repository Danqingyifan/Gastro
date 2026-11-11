#include "Characters/GastroCharacterWithExternalASC.h"
#include "GAS/GastroAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

void AGastroCharacterWithExternalASC::SetExternalAbilitySystemComponent(UGastroAbilitySystemComponent* ASC)
{
    ExternalASC = ASC;
    TryInitializeAbilitySystem();
}

UGastroAbilitySystemComponent* AGastroCharacterWithExternalASC::ResolveAbilitySystemComponent() const
{
    if (auto* ASC = ExternalASC.Get()) return ASC;
    const APlayerState* State = GetPlayerState();
    // Old Pawn replication callbacks can arrive after the PlayerState has moved to a new body.
    if (State && State->GetPawn() && State->GetPawn() != this) return nullptr;
    const auto* Provider = Cast<IAbilitySystemInterface>(State);
    return Provider ? Cast<UGastroAbilitySystemComponent>(Provider->GetAbilitySystemComponent()) : nullptr;
}
