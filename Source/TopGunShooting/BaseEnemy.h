#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "BaseEnemy.generated.h"

UCLASS(Abstract, Blueprintable)
class TOPGUNSHOOTING_API ABaseEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	virtual void PrimaryAction_Implementation() override;
	virtual void SecondaryAction_Implementation() override;
};
