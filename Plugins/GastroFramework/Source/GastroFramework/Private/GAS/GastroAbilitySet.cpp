#include "GAS/GastroAbilitySet.h"

#include "GAS/GastroAbilitySystemComponent.h"
#include "GAS/GastroGameplayAbility.h"
#include "GastroFrameworkLog.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

bool FGastroAbilitySetEntry::IsValid() const
{
	return Ability && AbilityLevel > 0 &&
		!Ability->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
}

bool UGastroAbilitySet::GiveToAbilitySystem(UGastroAbilitySystemComponent* ASC, FGastroAbilitySetGrantedHandles& OutHandles, UObject* SourceObject) const
{
	if (!IsValid(ASC) || !ASC->IsOwnerActorAuthoritative())
	{
		return false;
	}
	if (!OutHandles.IsEmpty() && OutHandles.GrantedASC.Get() != ASC)
	{
		UE_LOG(LogGastroFramework, Warning, TEXT("AbilitySet %s: a grant receipt cannot be shared between different ASCs."), *GetName());
		return false;
	}
	// Validate the whole set before granting anything, so invalid data cannot produce a partial grant.
	for (const FGastroAbilitySetEntry& Entry : GrantedAbilities)
	{
		if (!Entry.IsValid())
		{
			UE_LOG(LogGastroFramework, Warning, TEXT("AbilitySet %s contains an invalid ability class or level."), *GetName());
			return false;
		}
	}

	OutHandles.GrantedASC = ASC;
	for (const FGastroAbilitySetEntry& Entry : GrantedAbilities)
	{
		FGameplayAbilitySpec Spec(Entry.Ability, Entry.AbilityLevel, INDEX_NONE, SourceObject);
		if (Entry.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
		}
		OutHandles.AbilitySpecHandles.Add(ASC->GiveAbility(Spec));
	}
	return true;
}

void FGastroAbilitySetGrantedHandles::TakeFromAbilitySystem()
{
	UGastroAbilitySystemComponent* ASC = GrantedASC.Get();
	if (ASC && !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}
	// Clear this receipt before cancellation callbacks run; only remove the recorded specs.
	const TArray<FGameplayAbilitySpecHandle> HandlesToRemove = MoveTemp(AbilitySpecHandles);
	GrantedASC.Reset();
	if (ASC)
	{
		for (const FGameplayAbilitySpecHandle& Handle : HandlesToRemove)
		{
			ASC->ClearAbility(Handle);
		}
	}
}

void UGastroAbilitySet::RemoveGrantedAbilities(FGastroAbilitySetGrantedHandles& Handles)
{
	Handles.TakeFromAbilitySystem();
}

#if WITH_EDITOR
EDataValidationResult UGastroAbilitySet::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);
	bool bValid = SuperResult != EDataValidationResult::Invalid;
	for (int32 Index = 0; Index < GrantedAbilities.Num(); ++Index)
	{
		if (!GrantedAbilities[Index].IsValid())
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("GrantedAbilities[%d] needs a concrete ability class and a positive level; an input tag is optional."), Index)));
			bValid = false;
		}
	}
	return bValid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
#endif

