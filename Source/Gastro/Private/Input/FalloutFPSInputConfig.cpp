#include "Input/FalloutFPSInputConfig.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
EDataValidationResult UFalloutFPSInputConfig::IsDataValid(FDataValidationContext& Context) const
{
    bool bValid = Super::IsDataValid(Context) != EDataValidationResult::Invalid;
    if (!MappingContext || !MoveAction || !MouseLookAction || MoveAction == MouseLookAction ||
        MoveAction->ValueType != EInputActionValueType::Axis2D || MouseLookAction->ValueType != EInputActionValueType::Axis2D)
    {
        Context.AddError(FText::FromString(TEXT("FPS input needs a MappingContext and distinct Axis2D Move/MouseLook actions.")));
        bValid = false;
    }
    return bValid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
#endif
