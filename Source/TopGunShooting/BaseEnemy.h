#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEnemy.generated.h"

class UBehaviorTree;
class UBlackboardData;
class USkeletalMeshComponent;

UCLASS(Abstract, Blueprintable)
class TOPGUNSHOOTING_API ABaseEnemy : public AActor
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Fog Of War")
	void UpdateFogRenderVisibility();

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	FVector GetFogVisibilitySampleLocation() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBlackboardData> BlackboardData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War")
	bool bRenderOnlyWhenCurrentlyVisibleByFog = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War")
	bool bRenderWhenFogManagerMissing = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War", meta = (ClampMin = "0.0"))
	float FogVisibilityUpdateInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War")
	FVector FogVisibilitySampleOffset = FVector(0.0f, 0.0f, 50.0f);

private:
	bool bLastFogRenderVisible = true;
};
