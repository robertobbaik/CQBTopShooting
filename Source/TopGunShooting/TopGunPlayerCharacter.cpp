#include "TopGunPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Math/Plane.h"
#include "Math/RotationMatrix.h"
#include "UObject/ConstructorHelpers.h"

ATopGunPlayerCharacter::ATopGunPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = 2200.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraArmLength;
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(CameraArmRotation);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
	CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

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

void ATopGunPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAim(DeltaSeconds);
	ApplyAimRotation();
}

void ATopGunPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CameraBoom->TargetArmLength = CameraArmLength;
	CameraBoom->SetRelativeRotation(CameraArmRotation);
	UpdateMovementSpeed();
	CachedAimPoint = GetActorLocation() + GetActorForwardVector() * GamepadAimDistance;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}

		PlayerController->bShowMouseCursor = true;
		PlayerController->CurrentMouseCursor = EMouseCursor::Default;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;
	}
}

void ATopGunPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATopGunPlayerCharacter::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATopGunPlayerCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATopGunPlayerCharacter::Look);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ATopGunPlayerCharacter::Look);
		}

		if (PrecisionMoveAction)
		{
			EnhancedInputComponent->BindAction(PrecisionMoveAction, ETriggerEvent::Started, this, &ATopGunPlayerCharacter::BeginPrecisionMove);
			EnhancedInputComponent->BindAction(PrecisionMoveAction, ETriggerEvent::Completed, this, &ATopGunPlayerCharacter::EndPrecisionMove);
		}

		if (FastMoveAction)
		{
			EnhancedInputComponent->BindAction(FastMoveAction, ETriggerEvent::Started, this, &ATopGunPlayerCharacter::BeginFastMove);
			EnhancedInputComponent->BindAction(FastMoveAction, ETriggerEvent::Completed, this, &ATopGunPlayerCharacter::EndFastMove);
		}
	}
}

void ATopGunPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	CachedMoveInput = MovementVector;

	if (MovementVector.IsNearlyZero())
	{
		return;
	}

	const FRotator MovementRotation(0.0f, CameraBoom->GetComponentRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ATopGunPlayerCharacter::Look(const FInputActionValue& Value)
{
	CachedLookInput = Value.Get<FVector2D>();
	bUseLookInputAim = !CachedLookInput.IsNearlyZero();
}

void ATopGunPlayerCharacter::BeginPrecisionMove()
{
	bPrecisionMoveHeld = true;
	UpdateMovementSpeed();
}

void ATopGunPlayerCharacter::EndPrecisionMove()
{
	bPrecisionMoveHeld = false;
	UpdateMovementSpeed();
}

void ATopGunPlayerCharacter::BeginFastMove()
{
	bFastMoveHeld = true;
	UpdateMovementSpeed();
}

void ATopGunPlayerCharacter::EndFastMove()
{
	bFastMoveHeld = false;
	UpdateMovementSpeed();
}

void ATopGunPlayerCharacter::UpdateMovementSpeed()
{
	float TargetSpeed = WalkSpeed;
	CurrentMovementState = ETopGunMovementState::Default;

	if (bPrecisionMoveHeld)
	{
		TargetSpeed = PrecisionMoveSpeed;
		CurrentMovementState = ETopGunMovementState::Precision;
	}
	else if (bFastMoveHeld)
	{
		TargetSpeed = FastMoveSpeed;
		CurrentMovementState = ETopGunMovementState::Fast;
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ATopGunPlayerCharacter::UpdateAim(float DeltaSeconds)
{
	if (bUseLookInputAim)
	{
		UpdateAimFromLookInput();
		return;
	}

	UpdateAimFromCursor();
}

void ATopGunPlayerCharacter::UpdateAimFromCursor()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	FVector WorldLocation;
	FVector WorldDirection;
	if (!PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return;
	}

	const FVector ActorLocation = GetActorLocation();
	const FPlane GroundPlane(ActorLocation, FVector::UpVector);
	const FVector TraceEnd = WorldLocation + (WorldDirection * 50000.0f);
	const FVector AimLocation = FMath::LinePlaneIntersection(WorldLocation, TraceEnd, GroundPlane);

	if (!AimLocation.ContainsNaN())
	{
		CachedAimPoint = AimLocation;
	}
}

void ATopGunPlayerCharacter::UpdateAimFromLookInput()
{
	if (CachedLookInput.IsNearlyZero())
	{
		return;
	}

	const FRotator AimRotation(0.0f, CameraBoom->GetComponentRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::Y);
	const FVector AimDirection = ((ForwardDirection * CachedLookInput.Y) + (RightDirection * CachedLookInput.X)).GetSafeNormal();

	if (!AimDirection.IsNearlyZero())
	{
		CachedAimPoint = GetActorLocation() + (AimDirection * GamepadAimDistance);
	}
}

void ATopGunPlayerCharacter::ApplyAimRotation()
{
	const FVector AimDirection = CachedAimPoint - GetActorLocation();
	if (AimDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = AimDirection.Rotation();
	SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
}
