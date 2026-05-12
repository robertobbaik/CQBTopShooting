#include "BaseEnemy.h"

#include "Components/SkeletalMeshComponent.h"
#include "FogOfWarManager.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = FogVisibilityUpdateInterval;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SetRootComponent(SkeletalMeshComponent);
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.TickInterval = FogVisibilityUpdateInterval;
	SetActorTickEnabled(bRenderOnlyWhenCurrentlyVisibleByFog);
	UpdateFogRenderVisibility();
}

void ABaseEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateFogRenderVisibility();
}

void ABaseEnemy::UpdateFogRenderVisibility()
{
	bool bShouldRender = true;

	if (bRenderOnlyWhenCurrentlyVisibleByFog)
	{
		const AFogOfWarManager* FogOfWarManager = AFogOfWarManager::Get(this);
		bShouldRender = FogOfWarManager
			? FogOfWarManager->IsWorldLocationCurrentlyVisible(GetFogVisibilitySampleLocation())
			: bRenderWhenFogManagerMissing;
	}

	if (bLastFogRenderVisible != bShouldRender || IsHidden() == bShouldRender)
	{
		bLastFogRenderVisible = bShouldRender;
		SetActorHiddenInGame(!bShouldRender);
	}
}

FVector ABaseEnemy::GetFogVisibilitySampleLocation() const
{
	return GetActorLocation() + FogVisibilitySampleOffset;
}
