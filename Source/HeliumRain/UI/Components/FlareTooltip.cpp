
#include "HeliumRain/UI/Components/FlareTooltip.h"
#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include <Engine/UserInterfaceSettings.h>
#include <Widgets/Layout/SBackgroundBlur.h>

#define LOCTEXT_NAMESPACE "FlareTooltip"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTooltip::Construct(const FArguments& InArgs)
{
	// Data
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	MenuManager = InArgs._MenuManager;
	TooltipVisible = false;
	TooltipDelay = 1.0;
	TooltipFadeDuration = 0.2;
	TooltipCurrentTime = 0;
	TooltipCurrentAlpha = 0;
	TooltipTarget = NULL;

	// Create the layout
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	[
		SNew(SBox)
		.Padding(this, &SFlareTooltip::GetTooltipPosition)
		[
			SAssignNew(ContentBox, SBox)
			.WidthOverride(Theme.ContentWidth / 2)
			[
				SNew(SBackgroundBlur)
				.BlurRadius(this, &SFlareTooltip::GetToolTipBlurRadius)
				.BlurStrength(this, &SFlareTooltip::GetToolTipBlurStrength)
				[
					SNew(SBorder)
					.BorderImage(&Theme.BackgroundBrush)
					.BorderBackgroundColor(this, &SFlareTooltip::GetTooltipColor)
					[
						SNew(SBorder)
						.BorderImage(&Theme.BackgroundBrush)
						.BorderBackgroundColor(this, &SFlareTooltip::GetTooltipColor)
						[
							SNew(SVerticalBox)

							// Title
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTooltip::GetTitleText)
								.TextStyle(&Theme.NameFont)
								.ShadowColorAndOpacity(this, &SFlareTooltip::GetTooltipShadowColor)
								.ColorAndOpacity(this, &SFlareTooltip::GetTooltipColor)
								.WrapTextAt(Theme.ContentWidth / 2 - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
							]

							// Content
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.ContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTooltip::GetHelpText)
								.TextStyle(&Theme.TextFont)
								.ShadowColorAndOpacity(this, &SFlareTooltip::GetTooltipShadowColor)
								.ColorAndOpacity(this, &SFlareTooltip::GetTooltipColor)
								.WrapTextAt(Theme.ContentWidth / 2 - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right)
							]
						]
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::HitTestInvisible);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTooltip::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	CurrentMousePos = AllottedGeometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos());

	// Hide
	if (MenuManager->GetPC()->IsGameBusy())
	{
		HideTooltipForce();
	}

	// Animate
	else
	{
		float CurrentDelay = TooltipVisible ? TooltipDelay : 0;

		TooltipCurrentTime += (TooltipVisible ? InDeltaTime : -InDeltaTime);
		TooltipCurrentTime = FMath::Clamp(TooltipCurrentTime, 0.0f, 2 * CurrentDelay + TooltipFadeDuration);
		TooltipCurrentAlpha = FMath::Clamp((TooltipCurrentTime - TooltipDelay) / TooltipFadeDuration, 0.0f, 1.0f);
	}
}

void SFlareTooltip::ShowTooltip(SWidget* TargetWidget, FText Title, FText Content)
{
	if (Title.ToString().Len())
	{
		if (TooltipTarget != TargetWidget)
		{
			SetVisibility(EVisibility::HitTestInvisible);
			TooltipTarget = TargetWidget;
			TooltipTitle = Title;
			TooltipContent = Content;
			TooltipVisible = !TooltipContent.IsEmptyOrWhitespace();
		}
	}
	else
	{
		TooltipVisible = false;
		TooltipTarget = NULL;
	}
}

void SFlareTooltip::HideTooltip(SWidget* TargetWidget)
{
	if (TooltipTarget == TargetWidget)
	{
		TooltipVisible = false;
		TooltipTarget = NULL;
	}
}

void SFlareTooltip::HideTooltipForce()
{
	TooltipVisible = false;
	TooltipTarget = NULL;
	TooltipCurrentTime = TooltipFadeDuration;
	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

TOptional<int32> SFlareTooltip::GetToolTipBlurRadius() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return Theme.BlurRadius * TooltipCurrentAlpha;
}

float SFlareTooltip::GetToolTipBlurStrength() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	return Theme.BlurStrength * TooltipCurrentAlpha;
}

FMargin SFlareTooltip::GetTooltipPosition() const
{
	AFlarePlayerController* PC = MenuManager->GetPC();

	if (PC)
	{
		FVector2D MousePos = CurrentMousePos;
		FVector2D WidgetSize = ContentBox->GetDesiredSize();
		FVector2D ScreenSize = PC->GetNavHUD()->GetViewportSize();
		
		MousePos.X = (MousePos.X > ScreenSize.X - WidgetSize.X) ? MousePos.X - WidgetSize.X : MousePos.X;
		MousePos.Y = (MousePos.Y > ScreenSize.Y - WidgetSize.Y) ? MousePos.Y - WidgetSize.Y : MousePos.Y;

		return FMargin(MousePos.X + 30, MousePos.Y + 40, 0, 0);
	}

	return FMargin(0);
}

FSlateColor SFlareTooltip::GetTooltipColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.NameFont.ColorAndOpacity.GetSpecifiedColor();
	Color.A *= TooltipCurrentAlpha;
	return Color;
}

FLinearColor SFlareTooltip::GetTooltipShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.NameFont.ColorAndOpacity.GetSpecifiedColor();
	Color.A *= TooltipCurrentAlpha;
	return Color;
}

FText SFlareTooltip::GetTitleText() const
{
	return TooltipTitle;
}

FText SFlareTooltip::GetHelpText() const
{
	return TooltipContent;
}


#undef LOCTEXT_NAMESPACE
