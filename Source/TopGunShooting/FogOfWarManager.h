#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogOfWarManager.generated.h"

class UFogOfWarRevealComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPostProcessComponent;
class UTexture2D;
class USceneComponent;

UCLASS()
class TOPGUNSHOOTING_API AFogOfWarManager : public AActor
{
	GENERATED_BODY()

public:
	AFogOfWarManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	static AFogOfWarManager* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Fog Of War")
	void InitializeFog();

	UFUNCTION(BlueprintCallable, Category = "Fog Of War")
	void ResizeFogTexture(FIntPoint NewTextureSize);

	UFUNCTION(BlueprintCallable, Category = "Fog Of War")
	void RegisterRevealSource(UFogOfWarRevealComponent* RevealSource);

	UFUNCTION(BlueprintCallable, Category = "Fog Of War")
	void UnregisterRevealSource(UFogOfWarRevealComponent* RevealSource);

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	UTexture2D* GetFogTexture() const { return FogTexture; }

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	bool IsWorldLocationCurrentlyVisible(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	bool WasWorldLocationExplored(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Fog Of War")
	bool WorldToFogPixel(FVector WorldLocation, FIntPoint& OutPixel) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War")
	FVector2D FogWorldOrigin = FVector2D(-4000.0f, -4000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War")
	FVector2D FogWorldSize = FVector2D(8000.0f, 8000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Debug")
	bool bUseDebugTextureColors = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process")
	bool bEnableFogPostProcess = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process")
	TObjectPtr<UMaterialInterface> FogPostProcessMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process")
	FName FogTextureParameterName = TEXT("FogTexture");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process")
	FName FogWorldOriginParameterName = TEXT("FogWorldOrigin");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process")
	FName FogWorldSizeParameterName = TEXT("FogWorldSize");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process")
	FName FogTextureSizeParameterName = TEXT("FogTextureSize");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War", meta = (ClampMin = "64", ClampMax = "4096"))
	FIntPoint FogTextureSize = FIntPoint(1024, 1024);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War", meta = (ClampMin = "0.01"))
	float UpdateInterval = 0.033f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Performance", meta = (ClampMin = "64", ClampMax = "4096"))
	int32 MaxRuntimeTextureDimension = 2048;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Performance", meta = (ClampMin = "0.001"))
	float MinRuntimeUpdateInterval = 0.016f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Performance", meta = (ClampMin = "3", ClampMax = "512"))
	int32 MaxRuntimeRevealRayCount = 192;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Quality")
	bool bUseBilinearFogTextureFiltering = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War|Quality", meta = (ClampMin = "0.0"))
	float RevealFeatherWorldDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War", meta = (ClampMin = "0", ClampMax = "255"))
	uint8 HiddenAlpha = 255;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War", meta = (ClampMin = "0", ClampMax = "255"))
	uint8 ExploredAlpha = 160;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fog Of War", meta = (ClampMin = "0", ClampMax = "255"))
	uint8 VisibleAlpha = 0;

private:
	void ApplyPerformanceLimits();
	void UpdatePostProcessMaterial();
	void UpdateFog();
	void DecayVisibleToExplored();
	void RevealFromSource(const UFogOfWarRevealComponent& RevealSource);
	void RevealCircle(FVector WorldOrigin, float Radius);
	void RevealTriangle(FVector WorldA, FVector WorldB, FVector WorldC, bool bFeatherEdgeAB, bool bFeatherEdgeCA);
	void RevealPixel(int32 X, int32 Y, uint8 NewAlpha);
	uint8 GetFeatheredRevealAlpha(float DistanceToEdge) const;
	void UploadTexture();
	bool IsValidPixel(int32 X, int32 Y) const;
	int32 PixelIndex(int32 X, int32 Y) const;
	FVector2D FogPixelToWorld(int32 X, int32 Y) const;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> FogTexture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog Of War|Debug", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog Of War|Post Process", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPostProcessComponent> FogPostProcessComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FogPostProcessMaterialInstance;

	bool bWarnedMissingPostProcessMaterial = false;
	bool bWarnedTextureSizeClamped = false;
	bool bWarnedUpdateIntervalClamped = false;
	bool bWarnedHighUploadCost = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFogOfWarRevealComponent>> RevealSources;

	TArray<uint8> FogAlpha;
	TArray<FColor> TexturePixels;
	FUpdateTextureRegion2D TextureUpdateRegion;
	float TimeUntilNextUpdate = 0.0f;
};
