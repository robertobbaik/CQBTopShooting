#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TopGunPlayerController.generated.h"

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> HUDWidget;
};
