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

	// Interaction region for entering driving mode
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UBoxComponent* DrivingTrigger;

	// Visual marker for where motor thrust is applied (position this at the rear of the boat in the editor)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USceneComponent* MotorLocation;

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

	// --- Boat Driving ---

	// Player currently driving this boat (nullptr if no one is driving)
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Driving")
	ACharacter* CurrentDriver;

	// Maximum forward/backward thrust force
	UPROPERTY(EditAnywhere, Category="Driving", meta=(ClampMin=0, ClampMax=500000))
	float ThrustForce = 100000.f;

	// Torque applied around Z axis for steering
	UPROPERTY(EditAnywhere, Category="Driving", meta=(ClampMin=0, ClampMax=100000))
	float SteeringTorque = 50000.f;

	// Max speed in cm/s before thrust cuts off
	UPROPERTY(EditAnywhere, Category="Driving", meta=(ClampMin=0, ClampMax=5000))
	float MaxSpeed = 2000.f;

	// Apply thrust and steering (called by driver character)
	UFUNCTION(BlueprintCallable, Category="Driving")
	void ApplyDrivingInput(float Throttle, float Steering);

	// Assign a driver to this boat (server only)
	UFUNCTION(BlueprintCallable, Category="Driving")
	void SetDriver(ACharacter* NewDriver);

	// Can the given character start driving? (checks proximity, etc.)
	UFUNCTION(BlueprintCallable, Category="Driving")
	bool CanBeginDriving(ACharacter* Character) const;

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
