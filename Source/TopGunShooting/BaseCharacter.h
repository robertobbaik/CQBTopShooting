#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseCharacter.generated.h"

class UFogOfWarRevealComponent;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class ETopGunMovementState : uint8
{
	Default UMETA(DisplayName = "Default"),
	Precision UMETA(DisplayName = "Precision"),
	Fast UMETA(DisplayName = "Fast")
};

UCLASS(Abstract, Blueprintable)
class TOPGUNSHOOTING_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void MoveFromInput(FVector2D MovementVector);

	UFUNCTION(BlueprintCallable, Category = "Character|Aim")
	void LookFromInput(FVector2D LookVector);

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void SetPrecisionMoveHeld(bool bNewHeld);

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void SetFastMoveHeld(bool bNewHeld);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Character|Action")
	void PrimaryAction();
	virtual void PrimaryAction_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Character|Action")
	void SecondaryAction();
	virtual void SecondaryAction_Implementation();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual FRotator GetMovementReferenceRotation() const;
	virtual void UpdateAim(float DeltaSeconds);
	virtual void UpdateAimFromCursor();
	virtual void UpdateAimFromLookInput();
	virtual void ApplyAimRotation();
	void UpdateMovementSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFogOfWarRevealComponent> FogOfWarRevealComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float PrecisionMoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float FastMoveSpeed = 575.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ETopGunMovementState CurrentMovementState = ETopGunMovementState::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War")
	bool bEnableFogRevealOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim")
	bool bEnableAimRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "100.0"))
	float GamepadAimDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraArmLength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FRotator CameraArmRotation = FRotator(-60.0f, 0.0f, 0.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim")
	FVector CachedAimPoint = FVector::ZeroVector;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PrecisionMoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FastMoveAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void BeginPrecisionMove();
	void EndPrecisionMove();
	void BeginFastMove();
	void EndFastMove();

	FVector2D CachedMoveInput = FVector2D::ZeroVector;
	FVector2D CachedLookInput = FVector2D::ZeroVector;

	bool bPrecisionMoveHeld = false;
	bool bFastMoveHeld = false;
	bool bUseLookInputAim = false;
};
