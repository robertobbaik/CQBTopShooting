#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TopGunPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;

UCLASS()
class TOPGUNSHOOTING_API ATopGunPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATopGunPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void SetupInputComponent() override;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> HUDWidget;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void BeginPrecisionMove();
	void EndPrecisionMove();
	void BeginFastMove();
	void EndFastMove();
};
