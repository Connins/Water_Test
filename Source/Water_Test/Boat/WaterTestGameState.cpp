#include "WaterTestGameState.h"
#include "WaterSubsystem.h"

AWaterTestGameState::AWaterTestGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWaterTestGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// WaterSubsystem accumulates its own clock from DeltaTime starting at 0 on client join.
	// Without correction, late-joining clients evaluate Gerstner waves at a different phase
	// than the server — affecting both physics queries and the visual ocean shader (via MPC).
	// SetSmoothedWorldTimeSeconds() switches the subsystem onto a caller-controlled float
	// instead of its internal accumulator, syncing both wave queries and visual rendering.
	if (!HasAuthority())
	{
		if (UWaterSubsystem* WaterSub = UWaterSubsystem::GetWaterSubsystem(GetWorld()))
		{
			// GetServerWorldTimeSeconds() = LocalTime + smoothed(ServerTime - LocalTime),
			// so it advances every frame and converges to the server's wave clock.
			WaterSub->SetSmoothedWorldTimeSeconds(static_cast<float>(GetServerWorldTimeSeconds()));
		}
	}
}
