#include "BoatAwareMovementComponent.h"
#include "BoatNetworkedInterpAll.h"

void UBoatAwareMovementComponent::ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity)
{
	if (Impact.GetActor() && Impact.GetActor()->IsA<ABoatNetworkedInterpAll>())
	{
		return;
	}

	Super::ApplyImpactPhysicsForces(Impact, ImpactAcceleration, ImpactVelocity);
}
