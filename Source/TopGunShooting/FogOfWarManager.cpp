#include "FogOfWarManager.h"

#include "Components/SceneComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "FogOfWarRevealComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/PostProcessComponent.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr int32 HighUploadCostPixelCount = 2048 * 2048;

float DistanceToSegment(FVector2D Point, FVector2D SegmentStart, FVector2D SegmentEnd)
{
	const FVector2D Segment = SegmentEnd - SegmentStart;
	const float SegmentLengthSquared = Segment.SizeSquared();
	if (SegmentLengthSquared <= UE_KINDA_SMALL_NUMBER)
	{
		return FVector2D::Distance(Point, SegmentStart);
	}

	const float T = FMath::Clamp(FVector2D::DotProduct(Point - SegmentStart, Segment) / SegmentLengthSquared, 0.0f, 1.0f);
	return FVector2D::Distance(Point, SegmentStart + Segment * T);
}

float SmoothStep01(float Value)
{
	const float T = FMath::Clamp(Value, 0.0f, 1.0f);
	return T * T * (3.0f - 2.0f * T);
}
}

AFogOfWarManager::AFogOfWarManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FogPostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("FogPostProcess"));
	FogPostProcessComponent->SetupAttachment(SceneRoot);
	FogPostProcessComponent->bUnbound = true;
	FogPostProcessComponent->BlendWeight = 0.0f;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FogPostProcessMaterialFinder(TEXT("/Game/Materials/M_FogOfWar_PostProcess.M_FogOfWar_PostProcess"));
	if (FogPostProcessMaterialFinder.Succeeded())
	{
		FogPostProcessMaterial = FogPostProcessMaterialFinder.Object;
	}
}

void AFogOfWarManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeFog();
	UpdatePostProcessMaterial();
}

void AFogOfWarManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeUntilNextUpdate -= DeltaSeconds;
	if (TimeUntilNextUpdate > 0.0f)
	{
		return;
	}

	TimeUntilNextUpdate = FMath::Max(UpdateInterval, MinRuntimeUpdateInterval);
	UpdateFog();
}

AFogOfWarManager* AFogOfWarManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	return Cast<AFogOfWarManager>(UGameplayStatics::GetActorOfClass(WorldContextObject, StaticClass()));
}

void AFogOfWarManager::InitializeFog()
{
	ApplyPerformanceLimits();

	const int32 PixelCount = FogTextureSize.X * FogTextureSize.Y;
	FogAlpha.Init(HiddenAlpha, PixelCount);
	TexturePixels.Init(FColor(0, 0, 0, HiddenAlpha), PixelCount);
	TextureUpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, FogTextureSize.X, FogTextureSize.Y);

	FogTexture = UTexture2D::CreateTransient(FogTextureSize.X, FogTextureSize.Y, PF_B8G8R8A8);
	if (FogTexture)
	{
		FogTexture->NeverStream = true;
		FogTexture->CompressionSettings = TC_VectorDisplacementmap;
		FogTexture->SRGB = false;
		FogTexture->Filter = bUseBilinearFogTextureFiltering ? TF_Bilinear : TF_Nearest;
		FogTexture->AddressX = TA_Clamp;
		FogTexture->AddressY = TA_Clamp;
		FogTexture->UpdateResource();
		UploadTexture();
		UpdatePostProcessMaterial();
	}
}

void AFogOfWarManager::ResizeFogTexture(FIntPoint NewTextureSize)
{
	FogTextureSize = NewTextureSize;
	InitializeFog();
}

void AFogOfWarManager::ApplyPerformanceLimits()
{
	MaxRuntimeTextureDimension = FMath::Clamp(MaxRuntimeTextureDimension, 64, 4096);
	MinRuntimeUpdateInterval = FMath::Max(0.001f, MinRuntimeUpdateInterval);
	MaxRuntimeRevealRayCount = FMath::Clamp(MaxRuntimeRevealRayCount, 3, 512);
	RevealFeatherWorldDistance = FMath::Max(0.0f, RevealFeatherWorldDistance);

	const FIntPoint RequestedTextureSize = FogTextureSize;
	FogTextureSize.X = FMath::Clamp(FogTextureSize.X, 64, MaxRuntimeTextureDimension);
	FogTextureSize.Y = FMath::Clamp(FogTextureSize.Y, 64, MaxRuntimeTextureDimension);

	if (FogTextureSize != RequestedTextureSize && !bWarnedTextureSizeClamped)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("FogOfWarManager clamped FogTextureSize from %dx%d to %dx%d. Increase MaxRuntimeTextureDimension only after profiling."),
			RequestedTextureSize.X,
			RequestedTextureSize.Y,
			FogTextureSize.X,
			FogTextureSize.Y);
		bWarnedTextureSizeClamped = true;
	}

	if (UpdateInterval < MinRuntimeUpdateInterval)
	{
		if (!bWarnedUpdateIntervalClamped)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("FogOfWarManager clamped UpdateInterval from %.4f to %.4f at runtime."),
				UpdateInterval,
				MinRuntimeUpdateInterval);
			bWarnedUpdateIntervalClamped = true;
		}
		UpdateInterval = MinRuntimeUpdateInterval;
	}

	if (FogTextureSize.X * FogTextureSize.Y >= HighUploadCostPixelCount && UpdateInterval < 0.033f && !bWarnedHighUploadCost)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("FogOfWarManager is uploading a %dx%d texture every %.4f seconds. This is a high-cost setting for the current CPU texture upload path."),
			FogTextureSize.X,
			FogTextureSize.Y,
			UpdateInterval);
		bWarnedHighUploadCost = true;
	}
}

void AFogOfWarManager::RegisterRevealSource(UFogOfWarRevealComponent* RevealSource)
{
	if (RevealSource)
	{
		RevealSources.AddUnique(RevealSource);
	}
}

void AFogOfWarManager::UnregisterRevealSource(UFogOfWarRevealComponent* RevealSource)
{
	RevealSources.Remove(RevealSource);
}

void AFogOfWarManager::UpdatePostProcessMaterial()
{
	if (!FogPostProcessComponent)
	{
		return;
	}

	FogPostProcessComponent->BlendWeight = bEnableFogPostProcess ? 1.0f : 0.0f;
	if (!bEnableFogPostProcess)
	{
		return;
	}

	if (!FogPostProcessMaterial)
	{
		if (!bWarnedMissingPostProcessMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("FogOfWarManager post process is enabled, but FogPostProcessMaterial is not assigned."));
			bWarnedMissingPostProcessMaterial = true;
		}
		return;
	}

	bWarnedMissingPostProcessMaterial = false;

	if (!FogPostProcessMaterialInstance || FogPostProcessMaterialInstance->Parent != FogPostProcessMaterial)
	{
		FogPostProcessMaterialInstance = UMaterialInstanceDynamic::Create(FogPostProcessMaterial, this);
		FogPostProcessComponent->Settings.WeightedBlendables.Array.Reset();
		FogPostProcessComponent->Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, FogPostProcessMaterialInstance));
	}

	if (!FogPostProcessMaterialInstance)
	{
		return;
	}

	if (FogTexture)
	{
		FogPostProcessMaterialInstance->SetTextureParameterValue(FogTextureParameterName, FogTexture);
	}

	FogPostProcessMaterialInstance->SetVectorParameterValue(
		FogWorldOriginParameterName,
		FLinearColor(FogWorldOrigin.X, FogWorldOrigin.Y, 0.0f, 0.0f));
	FogPostProcessMaterialInstance->SetVectorParameterValue(
		FogWorldSizeParameterName,
		FLinearColor(FogWorldSize.X, FogWorldSize.Y, 0.0f, 0.0f));
	FogPostProcessMaterialInstance->SetVectorParameterValue(
		FogTextureSizeParameterName,
		FLinearColor(FogTextureSize.X, FogTextureSize.Y, 0.0f, 0.0f));
}

bool AFogOfWarManager::IsWorldLocationCurrentlyVisible(FVector WorldLocation) const
{
	FIntPoint Pixel;
	return WorldToFogPixel(WorldLocation, Pixel) && FogAlpha.IsValidIndex(PixelIndex(Pixel.X, Pixel.Y)) && FogAlpha[PixelIndex(Pixel.X, Pixel.Y)] == VisibleAlpha;
}

bool AFogOfWarManager::WasWorldLocationExplored(FVector WorldLocation) const
{
	FIntPoint Pixel;
	return WorldToFogPixel(WorldLocation, Pixel) && FogAlpha.IsValidIndex(PixelIndex(Pixel.X, Pixel.Y)) && FogAlpha[PixelIndex(Pixel.X, Pixel.Y)] < HiddenAlpha;
}

bool AFogOfWarManager::WorldToFogPixel(FVector WorldLocation, FIntPoint& OutPixel) const
{
	if (FogWorldSize.X <= 0.0f || FogWorldSize.Y <= 0.0f)
	{
		return false;
	}

	const FVector2D Normalized(
		(WorldLocation.X - FogWorldOrigin.X) / FogWorldSize.X,
		(WorldLocation.Y - FogWorldOrigin.Y) / FogWorldSize.Y);

	if (Normalized.X < 0.0f || Normalized.X > 1.0f || Normalized.Y < 0.0f || Normalized.Y > 1.0f)
	{
		return false;
	}

	OutPixel.X = FMath::Clamp(FMath::FloorToInt(Normalized.X * FogTextureSize.X), 0, FogTextureSize.X - 1);
	OutPixel.Y = FMath::Clamp(FMath::FloorToInt(Normalized.Y * FogTextureSize.Y), 0, FogTextureSize.Y - 1);
	return true;
}

void AFogOfWarManager::UpdateFog()
{
	if (!FogTexture || FogAlpha.Num() != FogTextureSize.X * FogTextureSize.Y)
	{
		InitializeFog();
	}

	DecayVisibleToExplored();

	for (int32 Index = RevealSources.Num() - 1; Index >= 0; --Index)
	{
		UFogOfWarRevealComponent* RevealSource = RevealSources[Index];
		if (!IsValid(RevealSource))
		{
			RevealSources.RemoveAtSwap(Index);
			continue;
		}

		if (RevealSource->IsRevealEnabled())
		{
			RevealFromSource(*RevealSource);
		}
	}

	UploadTexture();
	UpdatePostProcessMaterial();
}

void AFogOfWarManager::DecayVisibleToExplored()
{
	for (uint8& Alpha : FogAlpha)
	{
		if (Alpha < ExploredAlpha)
		{
			Alpha = ExploredAlpha;
		}
	}
}

void AFogOfWarManager::RevealFromSource(const UFogOfWarRevealComponent& RevealSource)
{
	const FVector Origin = RevealSource.GetRevealOrigin();
	RevealCircle(Origin, RevealSource.NearSightRadius);

	const int32 ClampedRayCount = FMath::Clamp(RevealSource.RayCount, 3, MaxRuntimeRevealRayCount);
	const float HalfAngle = FMath::DegreesToRadians(RevealSource.SightAngleDegrees * 0.5f);
	const FVector Direction = RevealSource.GetRevealDirection().GetSafeNormal2D();
	const float BaseAngle = FMath::Atan2(Direction.Y, Direction.X);
	UWorld* World = GetWorld();

	TArray<FVector> Endpoints;
	Endpoints.Reserve(ClampedRayCount + 1);

	for (int32 RayIndex = 0; RayIndex <= ClampedRayCount; ++RayIndex)
	{
		const float Alpha = static_cast<float>(RayIndex) / static_cast<float>(ClampedRayCount);
		const float RayAngle = BaseAngle - HalfAngle + HalfAngle * 2.0f * Alpha;
		const FVector RayDirection(FMath::Cos(RayAngle), FMath::Sin(RayAngle), 0.0f);
		FVector End = Origin + RayDirection * RevealSource.SightDistance;

		if (RevealSource.bUseLineOfSight && World)
		{
			const FVector TraceStart = Origin + FVector(0.0f, 0.0f, RevealSource.TraceHeight);
			const FVector TraceEnd = End + FVector(0.0f, 0.0f, RevealSource.TraceHeight);
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(FogOfWarLineOfSight), false, RevealSource.GetOwner());
			Params.bTraceComplex = RevealSource.bLineOfSightTraceComplex;

			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, RevealSource.LineOfSightChannel, Params))
			{
				End = FVector(Hit.Location.X, Hit.Location.Y, Origin.Z);

				if (RevealSource.bDrawLineOfSightDebug)
				{
					DrawDebugLine(
						World,
						TraceStart,
						Hit.Location,
						RevealSource.LineOfSightClearDebugColor,
						false,
						RevealSource.LineOfSightDebugDuration,
						0,
						RevealSource.LineOfSightDebugThickness);
					DrawDebugLine(
						World,
						Hit.Location,
						TraceEnd,
						RevealSource.LineOfSightBlockedDebugColor,
						false,
						RevealSource.LineOfSightDebugDuration,
						0,
						RevealSource.LineOfSightDebugThickness);
					DrawDebugSphere(
						World,
						Hit.Location,
						12.0f,
						8,
						RevealSource.LineOfSightBlockedDebugColor,
						false,
						RevealSource.LineOfSightDebugDuration,
						0,
						RevealSource.LineOfSightDebugThickness);
				}

				if (RevealSource.bLogLineOfSightHits)
				{
					UE_LOG(
						LogTemp,
						Verbose,
						TEXT("FogOfWar LOS hit %s component %s at %s."),
						*GetNameSafe(Hit.GetActor()),
						*GetNameSafe(Hit.GetComponent()),
						*Hit.Location.ToString());
				}
			}
			else if (RevealSource.bDrawLineOfSightDebug)
			{
				DrawDebugLine(
					World,
					TraceStart,
					TraceEnd,
					RevealSource.LineOfSightClearDebugColor,
					false,
					RevealSource.LineOfSightDebugDuration,
					0,
					RevealSource.LineOfSightDebugThickness);
			}
		}

		Endpoints.Add(End);
	}

	for (int32 Index = 0; Index + 1 < Endpoints.Num(); ++Index)
	{
		RevealTriangle(
			Origin,
			Endpoints[Index],
			Endpoints[Index + 1],
			Index == 0,
			Index + 2 == Endpoints.Num());
	}
}

void AFogOfWarManager::RevealCircle(FVector WorldOrigin, float Radius)
{
	if (Radius <= 0.0f)
	{
		return;
	}

	FIntPoint CenterPixel;
	if (!WorldToFogPixel(WorldOrigin, CenterPixel))
	{
		return;
	}

	const float PixelRadiusX = Radius / FogWorldSize.X * FogTextureSize.X;
	const float PixelRadiusY = Radius / FogWorldSize.Y * FogTextureSize.Y;
	const int32 RadiusPixelsX = FMath::CeilToInt(PixelRadiusX);
	const int32 RadiusPixelsY = FMath::CeilToInt(PixelRadiusY);

	for (int32 Y = CenterPixel.Y - RadiusPixelsY; Y <= CenterPixel.Y + RadiusPixelsY; ++Y)
	{
		for (int32 X = CenterPixel.X - RadiusPixelsX; X <= CenterPixel.X + RadiusPixelsX; ++X)
		{
			if (!IsValidPixel(X, Y))
			{
				continue;
			}

			const FVector2D WorldPoint = FogPixelToWorld(X, Y);
			const float DistanceFromOrigin = FVector2D::Distance(WorldPoint, FVector2D(WorldOrigin.X, WorldOrigin.Y));
			if (DistanceFromOrigin <= Radius)
			{
				RevealPixel(X, Y, GetFeatheredRevealAlpha(Radius - DistanceFromOrigin));
			}
		}
	}
}

void AFogOfWarManager::RevealTriangle(FVector WorldA, FVector WorldB, FVector WorldC, bool bFeatherEdgeAB, bool bFeatherEdgeCA)
{
	FIntPoint A;
	FIntPoint B;
	FIntPoint C;
	if (!WorldToFogPixel(WorldA, A) || !WorldToFogPixel(WorldB, B) || !WorldToFogPixel(WorldC, C))
	{
		return;
	}

	const int32 MinX = FMath::Clamp(FMath::Min3(A.X, B.X, C.X), 0, FogTextureSize.X - 1);
	const int32 MaxX = FMath::Clamp(FMath::Max3(A.X, B.X, C.X), 0, FogTextureSize.X - 1);
	const int32 MinY = FMath::Clamp(FMath::Min3(A.Y, B.Y, C.Y), 0, FogTextureSize.Y - 1);
	const int32 MaxY = FMath::Clamp(FMath::Max3(A.Y, B.Y, C.Y), 0, FogTextureSize.Y - 1);

	const FVector2D AVec(A.X, A.Y);
	const FVector2D BVec(B.X, B.Y);
	const FVector2D CVec(C.X, C.Y);
	const FVector2D AWorld(WorldA.X, WorldA.Y);
	const FVector2D BWorld(WorldB.X, WorldB.Y);
	const FVector2D CWorld(WorldC.X, WorldC.Y);
	const float Area = FVector2D::CrossProduct(BVec - AVec, CVec - AVec);
	if (FMath::IsNearlyZero(Area))
	{
		return;
	}

	for (int32 Y = MinY; Y <= MaxY; ++Y)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const FVector2D P(X + 0.5f, Y + 0.5f);
			const float W0 = FVector2D::CrossProduct(BVec - AVec, P - AVec);
			const float W1 = FVector2D::CrossProduct(CVec - BVec, P - BVec);
			const float W2 = FVector2D::CrossProduct(AVec - CVec, P - CVec);

			if ((W0 >= 0.0f && W1 >= 0.0f && W2 >= 0.0f) || (W0 <= 0.0f && W1 <= 0.0f && W2 <= 0.0f))
			{
				uint8 RevealAlpha = VisibleAlpha;
				if (RevealFeatherWorldDistance > UE_KINDA_SMALL_NUMBER)
				{
					const FVector2D WorldPoint = FogPixelToWorld(X, Y);
					float DistanceToFeatherEdge = DistanceToSegment(WorldPoint, BWorld, CWorld);
					if (bFeatherEdgeAB)
					{
						DistanceToFeatherEdge = FMath::Min(DistanceToFeatherEdge, DistanceToSegment(WorldPoint, AWorld, BWorld));
					}
					if (bFeatherEdgeCA)
					{
						DistanceToFeatherEdge = FMath::Min(DistanceToFeatherEdge, DistanceToSegment(WorldPoint, CWorld, AWorld));
					}
					RevealAlpha = GetFeatheredRevealAlpha(DistanceToFeatherEdge);
				}

				RevealPixel(X, Y, RevealAlpha);
			}
		}
	}
}

void AFogOfWarManager::RevealPixel(int32 X, int32 Y, uint8 NewAlpha)
{
	if (IsValidPixel(X, Y))
	{
		uint8& CurrentAlpha = FogAlpha[PixelIndex(X, Y)];
		CurrentAlpha = FMath::Min(CurrentAlpha, NewAlpha);
	}
}

uint8 AFogOfWarManager::GetFeatheredRevealAlpha(float DistanceToEdge) const
{
	if (RevealFeatherWorldDistance <= UE_KINDA_SMALL_NUMBER || DistanceToEdge >= RevealFeatherWorldDistance)
	{
		return VisibleAlpha;
	}

	const float EdgeBlend = SmoothStep01(1.0f - DistanceToEdge / RevealFeatherWorldDistance);
	const float Alpha = FMath::Lerp(static_cast<float>(VisibleAlpha), static_cast<float>(ExploredAlpha), EdgeBlend);
	return static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Alpha), 0, 255));
}

void AFogOfWarManager::UploadTexture()
{
	if (!FogTexture || TexturePixels.Num() != FogAlpha.Num())
	{
		return;
	}

	for (int32 Index = 0; Index < FogAlpha.Num(); ++Index)
	{
		if (bUseDebugTextureColors)
		{
			const uint8 DebugValue = 255 - FogAlpha[Index];
			TexturePixels[Index] = FColor(DebugValue, DebugValue, DebugValue, 255);
		}
		else
		{
			TexturePixels[Index] = FColor(0, 0, 0, FogAlpha[Index]);
		}
	}

	FogTexture->UpdateTextureRegions(0, 1, &TextureUpdateRegion, FogTextureSize.X * sizeof(FColor), sizeof(FColor), reinterpret_cast<uint8*>(TexturePixels.GetData()));
}

bool AFogOfWarManager::IsValidPixel(int32 X, int32 Y) const
{
	return X >= 0 && X < FogTextureSize.X && Y >= 0 && Y < FogTextureSize.Y && FogAlpha.IsValidIndex(PixelIndex(X, Y));
}

int32 AFogOfWarManager::PixelIndex(int32 X, int32 Y) const
{
	return Y * FogTextureSize.X + X;
}

FVector2D AFogOfWarManager::FogPixelToWorld(int32 X, int32 Y) const
{
	return FVector2D(
		FogWorldOrigin.X + (static_cast<float>(X) + 0.5f) / static_cast<float>(FogTextureSize.X) * FogWorldSize.X,
		FogWorldOrigin.Y + (static_cast<float>(Y) + 0.5f) / static_cast<float>(FogTextureSize.Y) * FogWorldSize.Y);
}
