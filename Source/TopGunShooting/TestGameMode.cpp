#include "TestGameMode.h"

#include "TopGunPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATestGameMode::ATestGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(
		TEXT("/Game/Blueprints/Character/BP_PointManCharacter"));

	if (PlayerPawnClassFinder.Succeeded())
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}

	PlayerControllerClass = ATopGunPlayerController::StaticClass();
}
