#include "BoatNetworked.h"
#include "Net/UnrealNetwork.h"

ABoatNetworked::ABoatNetworked()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);

    SetNetUpdateFrequency(144.f);
    SetMinNetUpdateFrequency(60.f);  

    bAlwaysRelevant = true;

    // === PHYSICS MESH (ROOT) ===
    PhysicsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsMesh"));
    RootComponent = PhysicsMesh;

    PhysicsMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    PhysicsMesh->SetIsReplicated(true);
    PhysicsMesh->bReplicatePhysicsToAutonomousProxy = true;
    
    // // Optional but often helpful for boats

    // PhysicsMesh->SetLinearDamping(0.3f);
    // PhysicsMesh->SetAngularDamping(2.5f);


    // === VISUAL MESH (SMOOTHED ON CLIENT) ===
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(PhysicsMesh);

    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMesh->SetGenerateOverlapEvents(false);

    // THIS is the key
    VisualMesh->SetUsingAbsoluteLocation(true);
    VisualMesh->SetUsingAbsoluteRotation(true);
    VisualMesh->SetUsingAbsoluteScale(true);

}
void ABoatNetworked::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        PhysicsMesh->SetSimulatePhysics(true);
    }
    else
    {
        PhysicsMesh->SetSimulatePhysics(false);
        PhysicsMesh->SetVisibility(false);

        // Initialize visual mesh at correct transform
        VisualMesh->SetWorldTransform(PhysicsMesh->GetComponentTransform());
        
    }
}

void ABoatNetworked::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (HasAuthority())
    {
        clampRaftPhysics();
    }
    if (!HasAuthority())
    {
        interpolateVisualMesh(DeltaTime);
    }
}

void ABoatNetworked::interpolateVisualMesh(float DeltaTime)
{
    const FTransform Target = PhysicsMesh->GetComponentTransform();
    const FTransform Current = VisualMesh->GetComponentTransform();

    FQuat SmoothedRot = FQuat::Slerp(
        Current.GetRotation(),
        Target.GetRotation(),
        0.12f
    );

    FVector SmoothedLoc = FMath::VInterpTo(
        Current.GetLocation(),
        Target.GetLocation(),
        DeltaTime,
        8.f
    );

    VisualMesh->SetWorldLocationAndRotation(SmoothedLoc, SmoothedRot);
}

void ABoatNetworked::clampRaftPhysics()
{
    FVector AngVel = PhysicsMesh->GetPhysicsAngularVelocityInDegrees();
    
    AngVel.X = FMath::Clamp(AngVel.X, -40.f, 40.f);
    AngVel.Y = FMath::Clamp(AngVel.Y, -40.f, 40.f);
    
    PhysicsMesh->SetPhysicsAngularVelocityInDegrees(AngVel);
}
