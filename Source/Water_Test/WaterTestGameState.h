#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WaterTestGameState.generated.h"

UCLASS()
class WATER_TEST_API AWaterTestGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AWaterTestGameState();
	virtual void Tick(float DeltaSeconds) override;
};
