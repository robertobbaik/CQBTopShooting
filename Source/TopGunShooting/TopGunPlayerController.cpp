#include "TopGunPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "InputCoreTypes.h"

void ATopGunPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	CurrentMouseCursor = EMouseCursor::Default;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (IsLocalController() && HUDWidgetClass && !HUDWidget)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		}
	}
}

void ATopGunPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsLocalController() || !HUDWidget)
	{
		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	FVector2D MousePosition(MouseX, MouseY);
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (!FMath::IsNearlyZero(ViewportScale))
	{
		MousePosition /= ViewportScale;
	}
	HUDWidget->SetPositionInViewport(MousePosition, false);

	if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		const FVector MouseLog(MousePosition.X, MousePosition.Y, 0.0f);
		const FVector AimPointLog(MousePosition.X, MousePosition.Y, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("MousePosition: %s"), *MouseLog.ToString());
		UE_LOG(LogTemp, Warning, TEXT("AimPointPosition: %s"), *AimPointLog.ToString());
	}
}
