// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterBodyRiverComponent.h"
#include "BoatNetworkedInterpAll.generated.h"

class AWaterBody;

UCLASS()
class WATER_TEST_API ABoatNetworkedInterpAll : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABoatNetworkedInterpAll();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* Mesh;

	// Replicated transform from server
	UPROPERTY(ReplicatedUsing = OnRep_ServerTransform)
	FTransform ServerTransform;

	UFUNCTION()
	void OnRep_ServerTransform();

	// Client interpolation target
	FTransform TargetTransform;

	// Interp speed
	UPROPERTY(EditAnywhere)
	float InterpSpeed = 8.0f;

	void InterpBoat(float DeltaTime);

	UPROPERTY(EditAnywhere)
	float CenteringForce = 2000.0f;

	void PushBoatToSpline();

	UPROPERTY()
	UWaterBodyRiverComponent* CurrentRiver;

	// Assign the ocean water body in the editor so we can query wave height each tick
	UPROPERTY(EditAnywhere, Category="Water Surface")
	TObjectPtr<AWaterBody> OceanBody;

	// Vertical offset above the wave surface (positive = higher)
	UPROPERTY(EditAnywhere, Category="Water Surface", meta=(ClampMin=-200, ClampMax=200, Units="cm"))
	float WaterSurfaceOffset = 0.f;

	// Strength of the spring force pulling the boat toward the wave surface (cm/s² per cm of displacement)
	UPROPERTY(EditAnywhere, Category="Water Surface", meta=(ClampMin=0, ClampMax=10000))
	float SurfaceSpringStrength = 800.f;

	// Damping applied to vertical velocity to prevent bouncing (higher = settles faster)
	UPROPERTY(EditAnywhere, Category="Water Surface", meta=(ClampMin=0, ClampMax=50))
	float SurfaceVerticalDamping = 4.f;

private:
	// Spring force pulling the boat toward the wave surface each tick
	void ApplyWaterSurfaceForce();

	// Hard-snaps the boat's Z to the wave surface while preserving all other physics state
	void SnapToWaterSurface();

	// // Overlap events
	// UFUNCTION()
	// void OnOverlapBegin(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex,
	// 	bool bFromSweep,
	// 	const FHitResult& SweepResult
	// );
	//
	// UFUNCTION()
	// void OnOverlapEnd(
	// 	UPrimitiveComponent* OverlappedComponent,
	// 	AActor* OtherActor,
	// 	UPrimitiveComponent* OtherComp,
	// 	int32 OtherBodyIndex
	// );
	//
	// void CheckPlayersOnBoat();

	int32 PlayersOnBoat = 0;

	UPROPERTY(EditAnywhere)
	int playerNumber = 1;
};
