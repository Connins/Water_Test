#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterBodyRiverComponent.h"
#include "BoatNetworkedInterpAll.generated.h"

class AWaterBody;

/**
 * Which system controls the boat's steering.
 * Switch at runtime via SetSteeringMode() to hot-swap between the two approaches.
 *
 *   DriverInput  — one assigned driver sends throttle+steering axis each tick (original system)
 *   ZoneBased    — any player standing in ClockwiseTurnZone or AntiClockwiseTurnZone and
 *                  holding the ZoneTurn button contributes steering; both sides cancel out
 */
UENUM(BlueprintType)
enum class EBoatSteeringMode : uint8
{
	DriverInput UMETA(DisplayName="Driver Input"),
	ZoneBased   UMETA(DisplayName="Zone Based")
};


UCLASS()
class WATER_TEST_API ABoatNetworkedInterpAll : public AActor
{
	GENERATED_BODY()

public:
	ABoatNetworkedInterpAll();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// -------------------------------------------------------------------------
	// Components
	// -------------------------------------------------------------------------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* Mesh;

	/** Interaction region for entering driver-input driving mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UBoxComponent* DrivingTrigger;

	/** Visual marker for where motor thrust is applied — position at the rear in the editor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USceneComponent* MotorLocation;

	/** Zone-based steering: standing here turns the boat clockwise */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|ZoneSteering")
	class UBoxComponent* ClockwiseTurnZone;

	/** Zone-based steering: standing here turns the boat anti-clockwise */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|ZoneSteering")
	class UBoxComponent* AntiClockwiseTurnZone;

	/** Zone-based thrust: standing here applies forward thrust via the motor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|ZoneSteering")
	class UBoxComponent* ThrustZone;

	// -------------------------------------------------------------------------
	// Replication
	// -------------------------------------------------------------------------

	UPROPERTY(ReplicatedUsing = OnRep_ServerTransform)
	FTransform ServerTransform;

	UFUNCTION()
	void OnRep_ServerTransform();

	FTransform TargetTransform;

	UPROPERTY(EditAnywhere)
	float InterpSpeed = 8.0f;

	void InterpBoat(float DeltaTime);

	// -------------------------------------------------------------------------
	// Water surface
	// -------------------------------------------------------------------------

	UPROPERTY(EditAnywhere)
	float CenteringForce = 2000.0f;

	void PushBoatToSpline();

	UPROPERTY()
	UWaterBodyRiverComponent* CurrentRiver;

	UPROPERTY(EditAnywhere, Category="Water Surface")
	TObjectPtr<AWaterBody> OceanBody;

	UPROPERTY(EditAnywhere, Category="Water Surface", meta=(ClampMin=-200, ClampMax=200, Units="cm"))
	float WaterSurfaceOffset = 0.f;

	UPROPERTY(EditAnywhere, Category="Water Surface", meta=(ClampMin=0, ClampMax=10000))
	float SurfaceSpringStrength = 800.f;

	UPROPERTY(EditAnywhere, Category="Water Surface", meta=(ClampMin=0, ClampMax=50))
	float SurfaceVerticalDamping = 4.f;

	// -------------------------------------------------------------------------
	// Driving — shared properties used by both steering modes
	// -------------------------------------------------------------------------

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Driving")
	ACharacter* CurrentDriver;

	UPROPERTY(EditAnywhere, Category="Driving", meta=(ClampMin=0, ClampMax=500000))
	float ThrustForce = 100000.f;

	/** Steering torque magnitude — used by both DriverInput and ZoneBased modes */
	UPROPERTY(EditAnywhere, Category="Driving", meta=(ClampMin=0, ClampMax=100000))
	float SteeringTorque = 50000.f;

	UPROPERTY(EditAnywhere, Category="Driving", meta=(ClampMin=0, ClampMax=5000))
	float MaxSpeed = 2000.f;

	/**
	 * Throttle/steering axis input — called by the driver character each tick.
	 * In ZoneBased mode the Steering parameter is ignored; steering is handled
	 * entirely by ApplyZoneSteering() in Tick.
	 */
	UFUNCTION(BlueprintCallable, Category="Driving")
	void ApplyDrivingInput(float Throttle, float Steering);

	UFUNCTION(BlueprintCallable, Category="Driving")
	void SetDriver(ACharacter* NewDriver);

	UFUNCTION(BlueprintCallable, Category="Driving")
	bool CanBeginDriving(ACharacter* Character) const;

	void CheckDriverInZone();

	// -------------------------------------------------------------------------
	// Steering mode swap
	// -------------------------------------------------------------------------

	/**
	 * Active steering system.
	 * Change this to swap between the driver-input and zone-based approaches.
	 * Can be set in the editor or called at runtime via SetSteeringMode().
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Steering")
	EBoatSteeringMode SteeringMode = EBoatSteeringMode::DriverInput;

	/** Switches steering mode and clears any active zone-press state. */
	UFUNCTION(BlueprintCallable, Category="Steering")
	void SetSteeringMode(EBoatSteeringMode NewMode);

	// -------------------------------------------------------------------------
	// Zone-based steering — called from AWater_TestCharacter server RPCs
	// -------------------------------------------------------------------------

	/** Server: record that Character started holding the ZoneTurn button */
	void NotifyZoneTurnPressed(ACharacter* Character);

	/** Server: record that Character released the ZoneTurn button */
	void NotifyZoneTurnReleased(ACharacter* Character);

private:
	// Spring / snap helpers
	void ApplyWaterSurfaceForce();
	void SnapToWaterSurface();

	// Zone-based steering helpers
	void ApplyZoneSteering();

	// Zone-based thrust helper
	void ApplyZoneThrust();

	// Who is currently standing in each turn zone (populated by overlap events)
	TSet<ACharacter*> PlayersInClockwiseZone;
	TSet<ACharacter*> PlayersInAntiClockwiseZone;

	// Who is currently standing in the thrust zone
	TSet<ACharacter*> PlayersInThrustZone;

	// Subset of zone occupants currently holding the ZoneTurn button
	TSet<ACharacter*> ActiveClockwisePressers;
	TSet<ACharacter*> ActiveAntiClockwisePressers;

	// Overlap callbacks — driving trigger
	UFUNCTION()
	void OnDrivingTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnDrivingTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Overlap callbacks — clockwise turn zone
	UFUNCTION()
	void OnClockwiseZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnClockwiseZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Overlap callbacks — anti-clockwise turn zone
	UFUNCTION()
	void OnAntiClockwiseZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnAntiClockwiseZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// Overlap callbacks — thrust zone
	UFUNCTION()
	void OnThrustZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnThrustZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// // void CheckPlayersOnBoat();
	int32 PlayersOnBoat = 0;

	UPROPERTY(EditAnywhere)
	int playerNumber = 1;
};
