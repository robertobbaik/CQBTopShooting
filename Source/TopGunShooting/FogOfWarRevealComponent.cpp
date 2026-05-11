#include "FogOfWarRevealComponent.h"

#include "FogOfWarManager.h"
#include "GameFramework/Actor.h"

UFogOfWarRevealComponent::UFogOfWarRevealComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFogOfWarRevealComponent::BeginPlay()
{
	Super::BeginPlay();

	RegisteredManager = AFogOfWarManager::Get(GetWorld());
	if (RegisteredManager)
	{
		RegisteredManager->RegisterRevealSource(this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FogOfWarRevealComponent on %s could not find a FogOfWarManager."), *GetNameSafe(GetOwner()));
	}
}

void UFogOfWarRevealComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RegisteredManager)
	{
		RegisteredManager->UnregisterRevealSource(this);
		RegisteredManager = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UFogOfWarRevealComponent::SetRevealEnabled(bool bNewEnabled)
{
	bRevealEnabled = bNewEnabled;
}

FVector UFogOfWarRevealComponent::GetRevealOrigin() const
{
	const AActor* Owner = GetOwner();
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

FVector UFogOfWarRevealComponent::GetRevealDirection() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ForwardVector;
	}

	const FVector Forward = Owner->GetActorForwardVector();
	return Forward.IsNearlyZero() ? FVector::ForwardVector : Forward.GetSafeNormal();
}
