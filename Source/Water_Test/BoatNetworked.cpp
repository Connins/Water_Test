#include "BoatNetworked.h"
#include "Net/UnrealNetwork.h"

ABoatNetworked::ABoatNetworked()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(false); // IMPORTANT

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    Mesh->SetEnableGravity(true);

    Mesh->SetIsReplicated(false); // we manually replicate transform
}

void ABoatNetworked::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        Mesh->SetSimulatePhysics(true);
    }
    else
    {
        Mesh->SetSimulatePhysics(false);
    }
}

void ABoatNetworked::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        // Server updates transform
        ServerTransform = GetActorTransform();
    }
    else
    {
        // Client interpolates toward server transform
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
}

void ABoatNetworked::OnRep_ServerTransform()
{
    TargetTransform = ServerTransform;
}

void ABoatNetworked::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABoatNetworked, ServerTransform);
}