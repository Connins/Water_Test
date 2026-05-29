// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Water_TestGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AWater_TestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AWater_TestGameMode();

	/** Assigns each local player a unique controller ID so two gamepads route to two clients */
	virtual void PostLogin(APlayerController* NewPlayerController) override;
};



