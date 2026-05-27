#include "BoatNetworkedInterpAll.h"
#include "WaterSplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"
#include "Components/BoxComponent.h"
#include "Water_TestCharacter.h"

ABoatNetworkedInterpAll::ABoatNetworkedInterpAll()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(false); // IMPORTANT

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetEnableGravity(true);

	Mesh->SetIsReplicated(false); // we manually replicate transform

	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	Mesh->SetLinearDamping(0.3f);
	Mesh->SetAngularDamping(2.5f);

	Mesh->SetGenerateOverlapEvents(false);

	// Create driving trigger zone
	DrivingTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DrivingTrigger"));
	DrivingTrigger->SetupAttachment(Mesh);
	DrivingTrigger->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	DrivingTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DrivingTrigger->SetGenerateOverlapEvents(true);

	// Create motor location marker (position this at the rear of the boat in the editor)
	MotorLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MotorLocation"));
	MotorLocation->SetupAttachment(Mesh);
	// Default to 150cm behind center
	MotorLocation->SetRelativeLocation(FVector(-150.f, 0.f, 0.f));
}

void ABoatNetworkedInterpAll::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Mesh->SetSimulatePhysics(true); // enabled once 2 players are on board

		// Bind overlap events for driving trigger
		if (DrivingTrigger)
		{
			DrivingTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABoatNetworkedInterpAll::OnDrivingTriggerBeginOverlap);
			DrivingTrigger->OnComponentEndOverlap.AddDynamic(this, &ABoatNetworkedInterpAll::OnDrivingTriggerEndOverlap);
		}
	}
	else
	{
		Mesh->SetSimulatePhysics(false);
	}
}

void ABoatNetworkedInterpAll::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// CheckPlayersOnBoat();

		if (OceanBody)
		{
			SnapToWaterSurface();
			//ApplyWaterSurfaceForce();
		}

		// Check if driver has left the trigger zone
		CheckDriverInZone();

		// Server updates transform
		ServerTransform = GetActorTransform();
	}
	else
	{
		// Client interpolates toward server transform
		InterpBoat(DeltaTime);
	}
}

void ABoatNetworkedInterpAll::OnRep_ServerTransform()
{
	TargetTransform = ServerTransform;
}

void ABoatNetworkedInterpAll::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABoatNetworkedInterpAll, ServerTransform);
	DOREPLIFETIME(ABoatNetworkedInterpAll, CurrentDriver);
}

void ABoatNetworkedInterpAll::ApplyWaterSurfaceForce()
{
	UWaterBodyComponent* WaterComp = OceanBody->GetWaterBodyComponent();
	if (!WaterComp)
	{
		return;
	}

	const EWaterBodyQueryFlags QueryFlags =
		EWaterBodyQueryFlags::ComputeLocation |
		EWaterBodyQueryFlags::ComputeImmersionDepth |
		EWaterBodyQueryFlags::IncludeWaves;

	auto QueryResult = WaterComp->TryQueryWaterInfoClosestToWorldLocation(Mesh->GetComponentLocation(), QueryFlags);
	if (!QueryResult.HasValue())
	{
		return;
	}

	const FWaterBodyQueryResult& Result = QueryResult.GetValue();

	// No IsInWater() guard here — when above the surface Displacement is negative,
	// so the spring pulls the boat downward and it follows the wave as it drops
	// instead of freefalling into the new surface height.

	const float TargetZ = Result.GetWaterSurfaceLocation().Z + WaterSurfaceOffset;
	const float CurrentZ = Mesh->GetComponentLocation().Z;
	const float Displacement = TargetZ - CurrentZ;

	// Spring force toward the surface — bAccelChange=true makes it mass-independent so it's
	// easy to tune regardless of the mesh's physics mass setting.
	const float VerticalVel = Mesh->GetPhysicsLinearVelocity().Z;
	const float ForceZ = (SurfaceSpringStrength * Displacement) - (SurfaceVerticalDamping * VerticalVel);

	Mesh->AddForce(FVector(0.f, 0.f, ForceZ), NAME_None, /*bAccelChange=*/true);
}

void ABoatNetworkedInterpAll::SnapToWaterSurface()
{
	UWaterBodyComponent* WaterComp = OceanBody->GetWaterBodyComponent();
	if (!WaterComp)
	{
		return;
	}

	const EWaterBodyQueryFlags QueryFlags =
		EWaterBodyQueryFlags::ComputeLocation |
		EWaterBodyQueryFlags::ComputeImmersionDepth |
		EWaterBodyQueryFlags::IncludeWaves;

	auto QueryResult = WaterComp->TryQueryWaterInfoClosestToWorldLocation(Mesh->GetComponentLocation(), QueryFlags);
	if (!QueryResult.HasValue())
	{
		return;
	}

	const FWaterBodyQueryResult& Result = QueryResult.GetValue();

	FVector NewLocation = Mesh->GetComponentLocation();
	NewLocation.Z = Result.GetWaterSurfaceLocation().Z + WaterSurfaceOffset;
	if (GetActorTransform().GetLocation().Z < NewLocation.Z ){
		
		Mesh->SetWorldLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	
}

void ABoatNetworkedInterpAll::InterpBoat(float DeltaTime)
{
	FVector NewLocation = FMath::VInterpTo(
		GetActorLocation(),
		TargetTransform.GetLocation(),
		DeltaTime,
		InterpSpeed
	);

	FRotator NewRotation = FMath::RInterpTo(
		GetActorRotation(),
		TargetTransform.GetRotation().Rotator(),
		DeltaTime,
		InterpSpeed
	);

	SetActorLocationAndRotation(NewLocation, NewRotation);
}

// // void ABoatNetworkedInterpAll::CheckPlayersOnBoat()
// // {
// // 	if (Mesh->IsSimulatingPhysics())
// // 	{
// // 		return;
// // 	}
// //
// // 	int32 NewPlayersOnBoat = 0;
// // 	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
// // 	{
// // 		if (It->GetMovementBase() == Mesh)
// // 		{
// // 			NewPlayersOnBoat++;
// // 		}
// // 	}
// //
// // 	if (NewPlayersOnBoat != PlayersOnBoat)
// // 	{
// // 		PlayersOnBoat = NewPlayersOnBoat;
// // 		if (GEngine)
// // 		{
// // 			GEngine->AddOnScreenDebugMessage(
// // 				-1,
// // 				5.0f,
// // 				FColor::Green,
// // 				FString::Printf(TEXT("Players on boat: %d/2"), PlayersOnBoat)
// // 			);
// // 		}
// // 		Mesh->SetSimulatePhysics(PlayersOnBoat >= playerNumber);
// // 	}
// // }
//
 void ABoatNetworkedInterpAll::PushBoatToSpline()
 {
	if (!CurrentRiver)
	{
		// if (GEngine)
		// {
		// 	GEngine->AddOnScreenDebugMessage(
		// 		-1,
		// 		5.0f,
		// 		FColor::Green,
		// 		TEXT("No River")
		// 	);
		// }
		return;
	}

	UWaterSplineComponent* RiverSpline = CurrentRiver->GetWaterSpline();

	if (!RiverSpline)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.0f,
				FColor::Green,
				TEXT("No Spline")
			);
		}
		return;
	}


	FVector RaftLocation = Mesh->GetComponentLocation();

	FVector ClosestPoint =
		RiverSpline->FindLocationClosestToWorldLocation(
			RaftLocation,
			ESplineCoordinateSpace::World
		);

	FVector Direction = ClosestPoint - RaftLocation;

	// ignore vertical correction
	Direction.Z = 0;

	float Distance = Direction.Size();

	if (Distance < 10.f)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.0f,
				FColor::Green,
				TEXT("Not Large Enough")
			);
		}
		return;
	}
	Direction.Normalize();

	FVector Force = Direction * CenteringForce;

	Mesh->AddForce(Force);

	FVector DisplaceArrow;
	DisplaceArrow.Z = 30.f;
	FVector ArrowLocation = RaftLocation + DisplaceArrow;
	// visualize the force
	DrawDebugDirectionalArrow(
		GetWorld(),
		ArrowLocation,
		ArrowLocation + Direction * 300,
		50.f,
		FColor::Red,
		false,
		0.0f,
		0,
		3.0f
	);
 }

// void ABoatNetworkedInterpAll::OnOverlapBegin(
// 	UPrimitiveComponent* OverlappedComponent,
// 	AActor* OtherActor,
// 	UPrimitiveComponent* OtherComp,
// 	int32 OtherBodyIndex,
// 	bool bFromSweep,
// 	const FHitResult& SweepResult)
// {
// 	UWaterBodyRiverComponent* RiverComp =
// 		OtherActor->FindComponentByClass<UWaterBodyRiverComponent>();
//
// 	if (RiverComp)
// 	{
// 		CurrentRiver = RiverComp;
//
// 			// if (GEngine)
// 			// {
// 			// 	GEngine->AddOnScreenDebugMessage(
// 			// 		-1,
// 			// 		5.0f,
// 			// 		FColor::Green,
// 			// 		TEXT("Raft entered river")
// 			// 		);
// 			// }
// 	}
//
// }

// void ABoatNetworkedInterpAll::OnOverlapEnd(
// 	UPrimitiveComponent* OverlappedComponent,
// 	AActor* OtherActor,
// 	UPrimitiveComponent* OtherComp,
// 	int32 OtherBodyIndex)
// {
// 	// UWaterBodyRiverComponent* RiverComp =
// 	// 	OtherActor->FindComponentByClass<UWaterBodyRiverComponent>();
// 	//
// 	// if (RiverComp && RiverComp == CurrentRiver)
// 	// {
// 	// 	CurrentRiver = nullptr;
// 	// 	if (GEngine)
// 	// 	{
// 	// 		GEngine->AddOnScreenDebugMessage(
// 	// 			-1,
// 	// 			5.0f,
// 	// 			FColor::Green,
// 	// 			TEXT("Raft left river")
// 	// 			);
// 	// 	}
// 	//
// 	// }
//
// }

void ABoatNetworkedInterpAll::ApplyDrivingInput(float Throttle, float Steering)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!Mesh->IsSimulatingPhysics())
	{
		return;
	}

	// Clamp inputs
	Throttle = FMath::Clamp(Throttle, -1.f, 1.f);
	Steering = FMath::Clamp(Steering, -1.f, 1.f);

	// Get current forward velocity to check against max speed
	FVector Velocity = Mesh->GetPhysicsLinearVelocity();
	FVector ForwardDir = GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(Velocity, ForwardDir);

	// Apply thrust force at rear of boat (like a motor)
	if (FMath::Abs(ForwardSpeed) < MaxSpeed || (Throttle * ForwardSpeed < 0.f))
	{
		FVector ThrustDirection = ForwardDir * Throttle;
		FVector ForceToApply = ThrustDirection * ThrustForce;

		// Apply force at motor location (position this component in the editor)
		FVector WorldMotorLocation = MotorLocation->GetComponentLocation();

		Mesh->AddForceAtLocation(ForceToApply, WorldMotorLocation);
	}

	// Apply steering torque around Z axis
	if (FMath::Abs(Steering) > 0.01f)
	{
		FVector TorqueToApply = FVector(0.f, 0.f, Steering * SteeringTorque);
		Mesh->AddTorqueInRadians(TorqueToApply, NAME_None, true);
	}
}

void ABoatNetworkedInterpAll::SetDriver(ACharacter* NewDriver)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentDriver = NewDriver;
}

bool ABoatNetworkedInterpAll::CanBeginDriving(ACharacter* Character) const
{
	if (!Character || !DrivingTrigger)
	{
		return false;
	}

	// Check if already has a driver
	if (CurrentDriver != nullptr)
	{
		return false;
	}

	// Check if character is within the trigger zone
	return DrivingTrigger->IsOverlappingActor(Character);
}

void ABoatNetworkedInterpAll::CheckDriverInZone()
{
	if (!HasAuthority() || !CurrentDriver || !DrivingTrigger)
	{
		return;
	}

	// If driver is no longer overlapping the trigger, auto-exit them
	if (!DrivingTrigger->IsOverlappingActor(CurrentDriver))
	{
		// Notify the character to clear their ControlledBoat reference
		AWater_TestCharacter* WaterCharacter = Cast<AWater_TestCharacter>(CurrentDriver);
		if (WaterCharacter)
		{
			WaterCharacter->ServerExitBoat();
		}

		CurrentDriver = nullptr;
	}
}

void ABoatNetworkedInterpAll::OnDrivingTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Optional: Add visual feedback or UI prompt when player enters the zone
	// For now, we just rely on the player pressing the enter boat key
}

void ABoatNetworkedInterpAll::OnDrivingTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Automatic exit is handled in CheckDriverInZone() in Tick
	// This callback is here for potential future use (UI updates, etc.)
}
