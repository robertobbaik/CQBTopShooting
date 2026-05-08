#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TopGunPlayerCharacter.generated.h"

class UCameraComponent;
class UFogOfWarRevealComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class ETopGunMovementState : uint8
{
	Default UMETA(DisplayName = "Default"),
	Precision UMETA(DisplayName = "Precision"),
	Fast UMETA(DisplayName = "Fast")
};

UCLASS()
class TOPGUNSHOOTING_API ATopGunPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATopGunPlayerCharacter();
	virtual void Tick(float DeltaSeconds) override;

	void MoveFromInput(FVector2D MovementVector);
	void LookFromInput(FVector2D LookVector);
	void SetPrecisionMoveHeld(bool bNewHeld);
	void SetFastMoveHeld(bool bNewHeld);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFogOfWarRevealComponent> FogOfWarRevealComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float PrecisionMoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float FastMoveSpeed = 575.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraArmLength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FRotator CameraArmRotation = FRotator(-60.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "100.0"))
	float GamepadAimDistance = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	FVector CachedAimPoint = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ETopGunMovementState CurrentMovementState = ETopGunMovementState::Default;

private:
	void UpdateMovementSpeed();
	void UpdateAim(float DeltaSeconds);
	void UpdateAimFromCursor();
	void UpdateAimFromLookInput();
	void ApplyAimRotation();

	FVector2D CachedMoveInput = FVector2D::ZeroVector;
	FVector2D CachedLookInput = FVector2D::ZeroVector;

	bool bPrecisionMoveHeld = false;
	bool bFastMoveHeld = false;
	bool bUseLookInputAim = false;
};
