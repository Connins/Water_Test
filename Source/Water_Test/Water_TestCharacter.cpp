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

AWater_TestCharacter::AWater_TestCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBoatAwareMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// QueryOnly removes the capsule from Chaos physics simulation entirely.
	// As QueryAndPhysics the capsule is a kinematic Chaos body that generates contact
	// impulses on simulated bodies (like the boat) even though the character doesn't
	// simulate physics itself. QueryOnly keeps it in the sweep/trace system so CMC
	// movement and floor detection work normally, but the boat's Chaos particle never
	// sees the capsule and receives no contact forces from it.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Skeletal mesh must also stay out of Chaos simulation — if it were QueryAndPhysics
	// it would be a second kinematic body contacting the boat mesh independently.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
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
	GetCharacterMovement()->bEnablePhysicsInteraction = true;

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
	
	SetReplicates(true);
	SetReplicateMovement(true);

	
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	
	MoveComp->bServerAcceptClientAuthoritativePosition = true;
	MoveComp->bIgnoreClientMovementErrorChecksAndCorrection = true;
	MoveComp->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
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

void AWater_TestCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !IsLocallyControlled())
	{
		ServerSmoothedLocation = GetActorLocation();
		bServerSmoothingInitialized = true;
	}
}

void AWater_TestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && !IsLocallyControlled())
	{
		FVector TargetLocation = GetActorLocation();

		if (!bServerSmoothingInitialized)
		{
			ServerSmoothedLocation = TargetLocation;
			bServerSmoothingInitialized = true;
		}

		float Dist = FVector::Dist(ServerSmoothedLocation, TargetLocation);

		// snap if extremely far (lag spike / teleport)
		if (Dist > 200.f)
		{
			ServerSmoothedLocation = TargetLocation;
		}
		else
		{
			ServerSmoothedLocation = FMath::VInterpTo(
				ServerSmoothedLocation,
				TargetLocation,
				DeltaTime,
				25.f
			);
		}

		// move capsule smoothly for physics
		GetCapsuleComponent()->SetWorldLocation(ServerSmoothedLocation, true);
	}
}