#include "BaseCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "FogOfWarRevealComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "Math/Plane.h"
#include "Math/RotationMatrix.h"
#include "UObject/ConstructorHelpers.h"

ABaseCharacter::ABaseCharacter()
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

	FogOfWarRevealComponent = CreateDefaultSubobject<UFogOfWarRevealComponent>(TEXT("FogOfWarRevealComponent"));

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

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
	CharacterMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}

void ABaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bEnableAimRotation)
	{
		UpdateAim(DeltaSeconds);
		ApplyAimRotation();
	}
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateMovementSpeed();
	CachedAimPoint = GetActorLocation() + GetActorForwardVector() * GamepadAimDistance;
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = CameraArmLength;
		CameraBoom->SetRelativeRotation(CameraArmRotation);
	}

	if (FogOfWarRevealComponent)
	{
		FogOfWarRevealComponent->SetRevealEnabled(bEnableFogRevealOnBeginPlay);
	}
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABaseCharacter::Move);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ABaseCharacter::Look);
	}

	if (PrecisionMoveAction)
	{
		EnhancedInputComponent->BindAction(PrecisionMoveAction, ETriggerEvent::Started, this, &ABaseCharacter::BeginPrecisionMove);
		EnhancedInputComponent->BindAction(PrecisionMoveAction, ETriggerEvent::Completed, this, &ABaseCharacter::EndPrecisionMove);
	}

	if (FastMoveAction)
	{
		EnhancedInputComponent->BindAction(FastMoveAction, ETriggerEvent::Started, this, &ABaseCharacter::BeginFastMove);
		EnhancedInputComponent->BindAction(FastMoveAction, ETriggerEvent::Completed, this, &ABaseCharacter::EndFastMove);
	}
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	MoveFromInput(Value.Get<FVector2D>());
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	LookFromInput(Value.Get<FVector2D>());
}

void ABaseCharacter::BeginPrecisionMove()
{
	SetPrecisionMoveHeld(true);
}

void ABaseCharacter::EndPrecisionMove()
{
	SetPrecisionMoveHeld(false);
}

void ABaseCharacter::BeginFastMove()
{
	SetFastMoveHeld(true);
}

void ABaseCharacter::EndFastMove()
{
	SetFastMoveHeld(false);
}

void ABaseCharacter::MoveFromInput(FVector2D MovementVector)
{
	CachedMoveInput = MovementVector;

	if (MovementVector.IsNearlyZero())
	{
		return;
	}

	const FRotator MovementRotation(0.0f, GetMovementReferenceRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABaseCharacter::LookFromInput(FVector2D LookVector)
{
	CachedLookInput = LookVector;
	bUseLookInputAim = !CachedLookInput.IsNearlyZero();
}

void ABaseCharacter::SetPrecisionMoveHeld(bool bNewHeld)
{
	bPrecisionMoveHeld = bNewHeld;
	UpdateMovementSpeed();
}

void ABaseCharacter::SetFastMoveHeld(bool bNewHeld)
{
	bFastMoveHeld = bNewHeld;
	UpdateMovementSpeed();
}

void ABaseCharacter::PrimaryAction_Implementation()
{
}

void ABaseCharacter::SecondaryAction_Implementation()
{
}

FRotator ABaseCharacter::GetMovementReferenceRotation() const
{
	return CameraBoom ? CameraBoom->GetComponentRotation() : GetActorRotation();
}

void ABaseCharacter::UpdateAim(float DeltaSeconds)
{
	if (bUseLookInputAim)
	{
		UpdateAimFromLookInput();
		return;
	}

	UpdateAimFromCursor();
}

void ABaseCharacter::UpdateAimFromCursor()
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

void ABaseCharacter::UpdateAimFromLookInput()
{
	if (CachedLookInput.IsNearlyZero())
	{
		return;
	}

	const FRotator AimRotation(0.0f, GetMovementReferenceRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::Y);
	const FVector AimDirection = ((ForwardDirection * CachedLookInput.Y) + (RightDirection * CachedLookInput.X)).GetSafeNormal();

	if (!AimDirection.IsNearlyZero())
	{
		CachedAimPoint = GetActorLocation() + (AimDirection * GamepadAimDistance);
	}
}

void ABaseCharacter::ApplyAimRotation()
{
	const FVector AimDirection = CachedAimPoint - GetActorLocation();
	if (AimDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = AimDirection.Rotation();
	SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
}

void ABaseCharacter::UpdateMovementSpeed()
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
