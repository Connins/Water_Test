#include "AI/OceanAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

AOceanAIController::AOceanAIController()
{
	// State machine runs on a timer, not per-frame tick
	PrimaryActorTick.bCanEverTick = false;
}

void AOceanAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GetWorldTimerManager().SetTimer(
		AIUpdateTimerHandle,
		this,
		&AOceanAIController::UpdateAI,
		UpdateInterval,
		true,          // looping
		UpdateInterval // first fire after one interval (not immediately)
	);
}

void AOceanAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(AIUpdateTimerHandle);
	Super::OnUnPossess();
}

void AOceanAIController::UpdateAI()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn || !GetPawn())
	{
		return;
	}

	const float DistanceToPlayer = GetPawn()->GetDistanceTo(PlayerPawn);

	// --- State selection ---
	const EOceanAIState NextState = (DistanceToPlayer <= DetectionRange)
		? EOceanAIState::Chase
		: EOceanAIState::Idle;

	SetState(NextState);

	// --- State execution ---
	switch (CurrentState)
	{
	case EOceanAIState::Chase:
		// MoveToActor handles pathfinding via the NavMesh — re-issued every interval
		// so the path stays fresh as the player moves
		MoveToActor(PlayerPawn, AcceptanceRadius);
		break;

	case EOceanAIState::Idle:
		StopMovement();
		break;
	}
}

void AOceanAIController::SetState(EOceanAIState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
}
