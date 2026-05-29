// Copyright Epic Games, Inc. All Rights Reserved.

#include "Water_TestGameMode.h"
#include "WaterTestGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/LocalPlayer.h"

AWater_TestGameMode::AWater_TestGameMode()
{
	GameStateClass = AWaterTestGameState::StaticClass();
}

void AWater_TestGameMode::PostLogin(APlayerController* NewPlayerController)
{
	Super::PostLogin(NewPlayerController);

	int32 PlayerIndex = GameState->PlayerArray.Num() - 1;
	ULocalPlayer* LocalPlayer = NewPlayerController->GetLocalPlayer();
	if (LocalPlayer)
	{
		LocalPlayer->SetControllerId(PlayerIndex);
	}
}
