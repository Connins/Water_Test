// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedPhysicsObject.generated.h"

/**
 * Base class for networked physics objects with server-authoritative simulation.
 * Server simulates physics; clients interpolate to replicated transform.
 * Inherit from this for boxes, crates, barrels, or any physics object that needs networking.
 */
UCLASS()
class WATER_TEST_API AReplicatedPhysicsObject : public AActor
{
	GENERATED_BODY()

public:
	AReplicatedPhysicsObject();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// The physics mesh - can be any mesh component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* PhysicsMesh;

	// Replicated transform from server (authority)
	UPROPERTY(ReplicatedUsing = OnRep_ServerTransform)
	FTransform ServerTransform;

	UFUNCTION()
	void OnRep_ServerTransform();

	// How fast clients interpolate toward server transform
	UPROPERTY(EditAnywhere, Category="Networking", meta=(ClampMin=0.1, ClampMax=50))
	float InterpSpeed = 10.0f;

protected:
	// Client interpolation target
	FTransform TargetTransform;

	// Interpolates client transform toward target
	void InterpolateToTarget(float DeltaTime);
};
