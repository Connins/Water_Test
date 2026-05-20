// Fill out your copyright notice in the Description page of Project Settings.

#include "ReplicatedPhysicsObject.h"
#include "Net/UnrealNetwork.h"

AReplicatedPhysicsObject::AReplicatedPhysicsObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;
	SetReplicateMovement(false); // We manually replicate transform via ServerTransform

	// Create physics mesh component
	PhysicsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsMesh"));
	RootComponent = PhysicsMesh;

	// Set up physics defaults
	PhysicsMesh->SetEnableGravity(true);
	PhysicsMesh->SetIsReplicated(false); // We manually replicate transform
	PhysicsMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	PhysicsMesh->SetGenerateOverlapEvents(true);
}

void AReplicatedPhysicsObject::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Server: enable physics simulation
		PhysicsMesh->SetSimulatePhysics(true);
	}
	else
	{
		// Client: disable physics, we'll interpolate to server's transform
		PhysicsMesh->SetSimulatePhysics(false);
		TargetTransform = GetActorTransform();
	}
}

void AReplicatedPhysicsObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// Server: update replicated transform
		ServerTransform = GetActorTransform();
	}
	else
	{
		// Client: interpolate toward server transform
		InterpolateToTarget(DeltaTime);
	}
}

void AReplicatedPhysicsObject::OnRep_ServerTransform()
{
	// Update interpolation target when server transform replicates
	TargetTransform = ServerTransform;
}

void AReplicatedPhysicsObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedPhysicsObject, ServerTransform);
}

void AReplicatedPhysicsObject::InterpolateToTarget(float DeltaTime)
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
