#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TopGunPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
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

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PrecisionMoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FastMoveAction;

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
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void BeginPrecisionMove();
	void EndPrecisionMove();
	void BeginFastMove();
	void EndFastMove();
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
