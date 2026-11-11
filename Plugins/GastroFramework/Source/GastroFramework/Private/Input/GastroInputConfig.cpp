#include "Input/GastroInputConfig.h"

#include "GastroFrameworkLog.h"
#include "InputAction.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

bool FGastroAbilityInputAction::IsValid() const
{
	return ::IsValid(InputAction.Get()) && InputAction->ValueType == EInputActionValueType::Boolean &&
		InputTag.IsValid();
}

#if WITH_EDITOR
EDataValidationResult UGastroInputConfig::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);
	bool bValid = SuperResult != EDataValidationResult::Invalid;
	TSet<const UInputAction*> Actions;
	TSet<FGameplayTag> Tags;
	for (int32 Index = 0; Index < AbilityInputActions.Num(); ++Index)
	{
		const FGastroAbilityInputAction& Mapping = AbilityInputActions[Index];
		if (!Mapping.IsValid() || Actions.Contains(Mapping.InputAction) || Tags.Contains(Mapping.InputTag))
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("AbilityInputActions[%d] needs a Boolean InputAction and an input tag; actions and tags must be unique."), Index)));
			bValid = false;
		}
		Actions.Add(Mapping.InputAction);
		Tags.Add(Mapping.InputTag);
	}
	return bValid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
#endif

