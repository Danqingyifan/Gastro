#include "Debug/FalloutWeaponTuningWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UFalloutWeaponTuningWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    SetIsFocusable(false);
    if (!WidgetTree) return;

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LessonPanel"));
    Panel->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.028f, 0.93f));
    Panel->SetPadding(FMargin(18.f));
    Panel->SetVisibility(ESlateVisibility::HitTestInvisible);
    WidgetTree->RootWidget = Panel;

    UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LessonContent"));
    Panel->SetContent(Content);

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LessonTitle"));
    Title->SetText(NSLOCTEXT("TTFLesson1", "Title", "第一课：固定持枪"));
    Title->SetColorAndOpacity(FLinearColor(0.88f, 0.91f, 0.95f));
    Content->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));

    UTextBlock* Notice = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LessonNotice"));
    Notice->SetText(NSLOCTEXT("TTFLesson1", "Notice", "本课没有可调参数。"));
    Notice->SetColorAndOpacity(FLinearColor(0.62f, 0.68f, 0.76f));
    Content->AddChildToVerticalBox(Notice);
}

void UFalloutWeaponTuningWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetDesiredSizeInViewport(FVector2D(320.f, 120.f));
}
