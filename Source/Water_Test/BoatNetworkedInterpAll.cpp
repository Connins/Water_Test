#include "BoatNetworkedInterpAll.h"
#include "WaterSplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"

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

	Mesh->SetGenerateOverlapEvents(true);
}

void ABoatNetworkedInterpAll::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Mesh->SetSimulatePhysics(true); // enabled once 2 players are on board
		// Mesh->OnComponentBeginOverlap.AddDynamic(this, &ABoatNetworkedInterpAll::OnOverlapBegin);
		// Mesh->OnComponentEndOverlap.AddDynamic(this, &ABoatNetworkedInterpAll::OnOverlapEnd);
		//ServerTransform = GetActorTransform();
	}
	else
	{
		Mesh->SetSimulatePhysics(false);
		//SetActorLocationAndRotation(TargetTransform.GetLocation(), TargetTransform.GetRotation());
	}
}

void ABoatNetworkedInterpAll::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// CheckPlayersOnBoat();

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
// void ABoatNetworkedInterpAll::PushBoatToSpline()
// {
// 	if (!CurrentRiver)
// 	{
// 		// if (GEngine)
// 		// {
// 		// 	GEngine->AddOnScreenDebugMessage(
// 		// 		-1,
// 		// 		5.0f,
// 		// 		FColor::Green,
// 		// 		TEXT("No River")
// 		// 	);
// 		// }
// 		return;
// 	}
//
// 	UWaterSplineComponent* RiverSpline = CurrentRiver->GetWaterSpline();
//
// 	if (!RiverSpline)
// 	{
// 		if (GEngine)
// 		{
// 			GEngine->AddOnScreenDebugMessage(
// 				-1,
// 				5.0f,
// 				FColor::Green,
// 				TEXT("No Spline")
// 			);
// 		}
// 		return;
// 	}
//
//
// 	FVector RaftLocation = Mesh->GetComponentLocation();
//
// 	FVector ClosestPoint =
// 		RiverSpline->FindLocationClosestToWorldLocation(
// 			RaftLocation,
// 			ESplineCoordinateSpace::World
// 		);
//
// 	FVector Direction = ClosestPoint - RaftLocation;
//
// 	// ignore vertical correction
// 	Direction.Z = 0;
//
// 	float Distance = Direction.Size();
//
// 	if (Distance < 10.f)
// 	{
// 		if (GEngine)
// 		{
// 			GEngine->AddOnScreenDebugMessage(
// 				-1,
// 				5.0f,
// 				FColor::Green,
// 				TEXT("Not Large Enough")
// 			);
// 		}
// 		return;
// 	}
// 	Direction.Normalize();
//
// 	FVector Force = Direction * CenteringForce;
//
// 	Mesh->AddForce(Force);
//
// 	FVector DisplaceArrow;
// 	DisplaceArrow.Z = 30.f;
// 	FVector ArrowLocation = RaftLocation + DisplaceArrow;
// 	// visualize the force
// 	DrawDebugDirectionalArrow(
// 		GetWorld(),
// 		ArrowLocation,
// 		ArrowLocation + Direction * 300,
// 		50.f,
// 		FColor::Red,
// 		false,
// 		0.0f,
// 		0,
// 		3.0f
// 	);
 //}

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
// 			// 	);
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
// 	// 		);
// 	// 	}
// 	// 
// 	// }
//
// }
