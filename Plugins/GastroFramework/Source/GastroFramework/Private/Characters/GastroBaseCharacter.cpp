#include "Characters/GastroBaseCharacter.h"
#include "Components/GastroAbilitySystemBindingComponent.h"
#include "GAS/GastroAbilitySystemComponent.h"

AGastroBaseCharacter::AGastroBaseCharacter()
{
    AbilityBinding = CreateDefaultSubobject<UGastroAbilitySystemBindingComponent>(TEXT("AbilityBinding"));
}

UAbilitySystemComponent* AGastroBaseCharacter::GetAbilitySystemComponent() const { return ResolveAbilitySystemComponent(); }

void AGastroBaseCharacter::TryInitializeAbilitySystem()
{
    if (auto* ASC = ResolveAbilitySystemComponent()) AbilityBinding->InitializeAbilitySystem(ASC, ASC->GetOwner());
    else AbilityBinding->UninitializeAbilitySystem();
}

void AGastroBaseCharacter::BeginPlay() { Super::BeginPlay(); TryInitializeAbilitySystem(); }
void AGastroBaseCharacter::PossessedBy(AController* NewController) { Super::PossessedBy(NewController); TryInitializeAbilitySystem(); }
void AGastroBaseCharacter::OnRep_Controller() { Super::OnRep_Controller(); TryInitializeAbilitySystem(); }
void AGastroBaseCharacter::OnRep_PlayerState() { Super::OnRep_PlayerState(); TryInitializeAbilitySystem(); }
void AGastroBaseCharacter::PawnClientRestart() { Super::PawnClientRestart(); TryInitializeAbilitySystem(); }

void AGastroBaseCharacter::UnPossessed()
{
    // An externally-owned ASC belongs to the player; a self-owned ASC remains with the body.
    auto* ASC = ResolveAbilitySystemComponent();
    if (ASC && ASC->GetOwner() != this) AbilityBinding->UninitializeAbilitySystem();
    Super::UnPossessed();
    if (ASC && ASC->GetOwner() == this) TryInitializeAbilitySystem();
}
