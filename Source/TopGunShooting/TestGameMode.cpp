#include "TestGameMode.h"

#include "TopGunPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATestGameMode::ATestGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(
		TEXT("/Game/Blueprints/Character/BP_TopGunPlayerCharacter"));

	if (PlayerPawnClassFinder.Succeeded())
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}

	PlayerControllerClass = ATopGunPlayerController::StaticClass();
}
