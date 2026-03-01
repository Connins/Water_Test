// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoatNetworked.generated.h"

UCLASS()
class WATER_TEST_API ABoatNetworked : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoatNetworked();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:

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
	float InterpSpeed = 8.0f;
};
