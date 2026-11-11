#pragma once

#include "GAS/GastroAbilitySet.h"
#include "GastroFrameworkLog.h"

inline bool GastroValidateStartupSets(const TArray<TObjectPtr<UGastroAbilitySet>>& Sets)
{
    for (const UGastroAbilitySet* Set : Sets)
    {
        if (!IsValid(Set) || Set->GrantedAbilities.ContainsByPredicate([](const FGastroAbilitySetEntry& Entry) { return !Entry.IsValid(); }))
        {
            UE_LOG(LogGastroFramework, Warning, TEXT("An invalid startup AbilitySet was configured."));
            return false;
        }
    }
    return true;
}
