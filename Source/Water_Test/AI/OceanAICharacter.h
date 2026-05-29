#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OceanAICharacter.generated.h"

/**
 * Simple AI chaser character for the ocean/boat world.
 * Assign SKM_Manny + ABP_Manny in a Blueprint subclass to get default mannequin + animations.
 * Mesh offset is pre-configured for the UE5 mannequin skeleton.
 */
UCLASS()
class WATER_TEST_API AOceanAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOceanAICharacter();

protected:
	virtual void BeginPlay() override;

public:
	// Walk speed in cm/s — synced to CharacterMovementComponent on BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Movement")
	float WalkSpeed = 400.f;
};
