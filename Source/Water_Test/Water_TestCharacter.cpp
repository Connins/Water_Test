// Copyright Epic Games, Inc. All Rights Reserved.

#include "Water_TestCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Water_Test.h"
#include "BoatNetworked.h"

AWater_TestCharacter::AWater_TestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
	// bUseControllerRotationPitch = false;
	// bUseControllerRotationRoll = false;
	// bUseControllerRotationYaw = true;
	//
	// GetCharacterMovement()->bIgnoreBaseRotation = true;
	// GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Linear;
	// GetCharacterMovement()->bNetworkAlwaysReplicateTransformUpdateTimestamp = true;
	// GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
	SetReplicates(true);
	SetReplicateMovement(true);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	
	MoveComp->bServerAcceptClientAuthoritativePosition = true;
	MoveComp->bIgnoreClientMovementErrorChecksAndCorrection = true;
	MoveComp->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
	
	// MoveComp->PushForceFactor = 200.0f;
	// MoveComp->MaxTouchForce = 150.0f;
	// MoveComp->MinTouchForce = 0.0f;
	// MoveComp->InitialPushForceFactor = 0.f;   // important for landing
	// MoveComp->StandingDownwardForceScale = 0.f;
}

void AWater_TestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWater_TestCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AWater_TestCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWater_TestCharacter::Look);
	}
	else
	{
		UE_LOG(LogWater_Test, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AWater_TestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AWater_TestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AWater_TestCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AWater_TestCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AWater_TestCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AWater_TestCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

// void AWater_TestCharacter::Tick(float DeltaTime)
// {
// 	Super::Tick(DeltaTime);
//
// 	AActor* BaseActor = APawn::GetMovementBaseActor(this);
//
// 	if (BaseActor && BaseActor->IsA(ABoatNetworked::StaticClass()))
// 	{
// 		if (GetAttachParentActor() != BaseActor)
// 		{
// 			AttachToActor(BaseActor, FAttachmentTransformRules::KeepWorldTransform);
// 		}
// 	}
// 	else
// 	{
// 		if (GetAttachParentActor())
// 		{
// 			DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
// 		}
// 	}
// }

// void AWater_TestCharacter::Tick(float DeltaTime)
// {
// 	AActor* BaseActor = GetMovementBaseActor(this);
//
// 	if (BaseActor && BaseActor->IsA(ABoatNetworked::StaticClass()))
// 	{
// 		FTransform CurrentBoatTransform = BaseActor->GetActorTransform();
//
// 		if (!LastBoatTransform.Equals(FTransform::Identity))
// 		{
// 			FTransform Delta = CurrentBoatTransform.GetRelativeTransform(LastBoatTransform);
//
// 			// Only move player horizontally with boat
// 			FVector Offset = Delta.GetLocation();
// 			Offset.Z = 0.f;
//
// 			AddActorWorldOffset(Offset, true);
//
// 			// Apply only yaw rotation
// 			FRotator RotDelta = Delta.Rotator();
// 			AddActorWorldRotation(FRotator(0.f, RotDelta.Yaw, 0.f));
// 		}
//
// 		LastBoatTransform = CurrentBoatTransform;
// 	}
// 	else
// 	{
// 		LastBoatTransform = FTransform::Identity;
// 	}
// }

// void AWater_TestCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
// {
// 	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
//
// 	AActor* BaseActor = APawn::GetMovementBaseActor(this);
//
// 	if (BaseActor && BaseActor->IsA(ABoatNetworked::StaticClass()))
// 	{
// 		//AttachToActor(BaseActor, FAttachmentTransformRules::KeepWorldTransform);
// 		if (HasAuthority())
// 		{
// 			AttachToActor(BaseActor, FAttachmentTransformRules::KeepWorldTransform);
// 		}
// 	}
// 	else
// 	{
// 		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
// 	}
// }