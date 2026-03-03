#include "BoatNetworked.h"
#include "Net/UnrealNetwork.h"

ABoatNetworked::ABoatNetworked()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);

    NetUpdateFrequency = 200.f;
    MinNetUpdateFrequency = 60.f;
    bAlwaysRelevant = true;

    // === PHYSICS MESH (ROOT) ===
    PhysicsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsMesh"));
    RootComponent = PhysicsMesh;

    PhysicsMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    PhysicsMesh->SetIsReplicated(true);
    PhysicsMesh->bReplicatePhysicsToAutonomousProxy = true;

    // // Optional but often helpful for boats
    // PhysicsMesh->SetAngularDamping(2.5f);
    // PhysicsMesh->SetLinearDamping(0.3f);

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

// void ABoatNetworked::Tick(float DeltaTime)
// {
//     Super::Tick(DeltaTime);
//
//     if (HasAuthority())
//     {
//         // Server updates transform
//         ServerTransform = GetActorTransform();
//     }
//     else
//     {
//         // Client interpolates toward server transform
//         FVector NewLocation = FMath::VInterpTo(
//             GetActorLocation(),
//             TargetTransform.GetLocation(),
//             DeltaTime,
//             InterpSpeed
//         );
//
//         FRotator NewRotation = FMath::RInterpTo(
//             GetActorRotation(),
//             TargetTransform.GetRotation().Rotator(),
//             DeltaTime,
//             InterpSpeed
//         );
//
//         SetActorLocationAndRotation(NewLocation, NewRotation);
//     }
// }
//
// void ABoatNetworked::OnRep_ServerTransform()
// {
//     TargetTransform = ServerTransform;
// }
//
// void ABoatNetworked::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
//     Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//     DOREPLIFETIME(ABoatNetworked, ServerTransform);
// }

void ABoatNetworked::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Super::Tick(DeltaTime);

    if (!HasAuthority())
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
}