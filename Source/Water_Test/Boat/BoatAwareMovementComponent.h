#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BoatAwareMovementComponent.generated.h"

UCLASS()
class WATER_TEST_API UBoatAwareMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	virtual void ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity) override;
};
