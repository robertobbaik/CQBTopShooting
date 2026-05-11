#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "FogOfWarRevealComponent.generated.h"

class AFogOfWarManager;

UCLASS(ClassGroup = (FogOfWar), meta = (BlueprintSpawnableComponent))
class TOPGUNSHOOTING_API UFogOfWarRevealComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFogOfWarRevealComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Fog Of War")
	void SetRevealEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	bool IsRevealEnabled() const { return bRevealEnabled; }

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	FVector GetRevealOrigin() const;

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	FVector GetRevealDirection() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War")
	bool bRevealEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War", meta = (ClampMin = "0.0"))
	float NearSightRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War", meta = (ClampMin = "0.0"))
	float SightDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War", meta = (ClampMin = "1.0", ClampMax = "360.0"))
	float SightAngleDegrees = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War", meta = (ClampMin = "3", ClampMax = "512"))
	int32 RayCount = 96;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War")
	bool bUseLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War")
	TEnumAsByte<ECollisionChannel> LineOfSightChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War")
	bool bLineOfSightTraceComplex = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War", meta = (ClampMin = "0.0"))
	float TraceHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War|Debug")
	bool bDrawLineOfSightDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War|Debug", meta = (ClampMin = "0.0"))
	float LineOfSightDebugDuration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War|Debug", meta = (ClampMin = "0.0"))
	float LineOfSightDebugThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War|Debug")
	FColor LineOfSightClearDebugColor = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War|Debug")
	FColor LineOfSightBlockedDebugColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Of War|Debug")
	bool bLogLineOfSightHits = false;

private:
	UPROPERTY(Transient)
	TObjectPtr<AFogOfWarManager> RegisteredManager;
};
