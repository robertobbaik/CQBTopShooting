#include "TopGunPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputCoreTypes.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "TopGunPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATopGunPlayerController::ATopGunPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextFinder(TEXT("/Game/Input/IMC_Player.IMC_Player"));
	if (MappingContextFinder.Succeeded())
	{
		DefaultMappingContext = MappingContextFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionFinder(TEXT("/Game/Input/IA_Move.IA_Move"));
	if (MoveActionFinder.Succeeded())
	{
		MoveAction = MoveActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionFinder(TEXT("/Game/Input/IA_Look.IA_Look"));
	if (LookActionFinder.Succeeded())
	{
		LookAction = LookActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> PrecisionMoveActionFinder(TEXT("/Game/Input/IA_PrecisionMove.IA_PrecisionMove"));
	if (PrecisionMoveActionFinder.Succeeded())
	{
		PrecisionMoveAction = PrecisionMoveActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> FastMoveActionFinder(TEXT("/Game/Input/IA_FastMove.IA_FastMove"));
	if (FastMoveActionFinder.Succeeded())
	{
		FastMoveAction = FastMoveActionFinder.Object;
	}
}

void ATopGunPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;
	CurrentMouseCursor = EMouseCursor::Default;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

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

void ATopGunPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATopGunPlayerController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATopGunPlayerController::Move);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATopGunPlayerController::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ATopGunPlayerController::Look);
	}

	if (PrecisionMoveAction)
	{
		EnhancedInputComponent->BindAction(PrecisionMoveAction, ETriggerEvent::Started, this, &ATopGunPlayerController::BeginPrecisionMove);
		EnhancedInputComponent->BindAction(PrecisionMoveAction, ETriggerEvent::Completed, this, &ATopGunPlayerController::EndPrecisionMove);
	}

	if (FastMoveAction)
	{
		EnhancedInputComponent->BindAction(FastMoveAction, ETriggerEvent::Started, this, &ATopGunPlayerController::BeginFastMove);
		EnhancedInputComponent->BindAction(FastMoveAction, ETriggerEvent::Completed, this, &ATopGunPlayerController::EndFastMove);
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

	// if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
	// {
	// 	const FVector MouseLog(MousePosition.X, MousePosition.Y, 0.0f);
	// 	const FVector AimPointLog(MousePosition.X, MousePosition.Y, 0.0f);
	// 	UE_LOG(LogTemp, Warning, TEXT("MousePosition: %s"), *MouseLog.ToString());
	// 	UE_LOG(LogTemp, Warning, TEXT("AimPointPosition: %s"), *AimPointLog.ToString());
	// }
}

void ATopGunPlayerController::Move(const FInputActionValue& Value)
{
	if (ATopGunPlayerCharacter* PlayerCharacter = Cast<ATopGunPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->MoveFromInput(Value.Get<FVector2D>());
	}
}

void ATopGunPlayerController::Look(const FInputActionValue& Value)
{
	if (ATopGunPlayerCharacter* PlayerCharacter = Cast<ATopGunPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->LookFromInput(Value.Get<FVector2D>());
	}
}

void ATopGunPlayerController::BeginPrecisionMove()
{
	if (ATopGunPlayerCharacter* PlayerCharacter = Cast<ATopGunPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->SetPrecisionMoveHeld(true);
	}
}

void ATopGunPlayerController::EndPrecisionMove()
{
	if (ATopGunPlayerCharacter* PlayerCharacter = Cast<ATopGunPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->SetPrecisionMoveHeld(false);
	}
}

void ATopGunPlayerController::BeginFastMove()
{
	if (ATopGunPlayerCharacter* PlayerCharacter = Cast<ATopGunPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->SetFastMoveHeld(true);
	}
}

void ATopGunPlayerController::EndFastMove()
{
	if (ATopGunPlayerCharacter* PlayerCharacter = Cast<ATopGunPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->SetFastMoveHeld(false);
	}
}
