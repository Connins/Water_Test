#include "AI/OceanAICharacter.h"
#include "AI/OceanAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AOceanAICharacter::AOceanAICharacter()
{
	// Wire up our controller — spawned automatically when placed in world
	AIControllerClass = AOceanAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Standard mannequin mesh offset — assign SKM_Manny + ABP_Manny in a Blueprint subclass
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// Face the direction of movement rather than the controller rotation
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void AOceanAICharacter::BeginPlay()
{
	Super::BeginPlay();

	// Sync the editable WalkSpeed property to the movement component
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
