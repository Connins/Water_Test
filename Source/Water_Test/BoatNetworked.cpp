// Fill out your copyright notice in the Description page of Project Settings.


#include "BoatNetworked.h"

// Sets default values
ABoatNetworked::ABoatNetworked()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Create Static Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	bReplicates = true;
	SetReplicateMovement(true);
	// Make it the Root
	RootComponent = Mesh;
	
	// Disable physics
	Mesh->SetSimulatePhysics(false);

	// Set collision profile
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	
	Mesh->SetIsReplicated(true);
	Mesh->bReplicatePhysicsToAutonomousProxy = false;
}

// Called when the game starts or when spawned
void ABoatNetworked::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
	}
	else
	{
		Mesh->SetSimulatePhysics(false);
	}
}

// Called every frame
void ABoatNetworked::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

