#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OceanAIController.generated.h"

UENUM(BlueprintType)
enum class EOceanAIState : uint8
{
	Idle  UMETA(DisplayName="Idle"),
	Chase UMETA(DisplayName="Chase")
};

/**
 * AI controller for AOceanAICharacter.
 * State machine runs on a timer (not Tick) — evaluates conditions then executes the active state.
 * Navigation is handled entirely by AAIController::MoveToActor (UE NavMesh).
 */
UCLASS()
class WATER_TEST_API AOceanAIController : public AAIController
{
	GENERATED_BODY()

public:
	AOceanAIController();

	// Exposed so you can read current state from Blueprint or debugger
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|State")
	EOceanAIState CurrentState = EOceanAIState::Idle;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	// Called on a repeating timer — evaluates state then executes it
	void UpdateAI();

	// Transitions state only when it actually changes
	void SetState(EOceanAIState NewState);

	FTimerHandle AIUpdateTimerHandle;

	// Distance at which this AI notices and starts chasing the player (cm)
	UPROPERTY(EditDefaultsOnly, Category="AI|Detection", meta=(ClampMin=0, Units="cm"))
	float DetectionRange = 1500.f;

	// How close the AI gets before it stops moving (cm)
	UPROPERTY(EditDefaultsOnly, Category="AI|Chase", meta=(ClampMin=0, Units="cm"))
	float AcceptanceRadius = 100.f;

	// How often state is re-evaluated and move commands are re-issued (seconds)
	UPROPERTY(EditDefaultsOnly, Category="AI|Detection", meta=(ClampMin=0.05f))
	float UpdateInterval = 0.25f;
};
