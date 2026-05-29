# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Water_Test** is an Unreal Engine 5.7 game project (single `Water_Test` runtime module). The primary goal is a **multiplayer cooperative boat physics game** — players share a physics-driven boat on an ocean. It also contains three independent gameplay prototype variants (Platforming, Combat, SideScrolling) that are unrelated to the boat game.

Uses: Water + Buoyancy plugins (ocean/wave surface), StateTree + GameplayStateTree (AI), UMG (UI), EnhancedInput (controls), AIModule (EQS/behavior).

## Building

No standalone build scripts — use the standard UE5 workflow:

- **Editor build**: Open `Water_Test.uproject` in Unreal Engine 5.7
- **Command-line build** (from UE install directory):
  ```
  UnrealBuildTool.exe Water_Test Win64 Development "path/to/Water_Test.uproject"
  ```
- **Targets**: `Source/Water_TestEditor.Target.cs` (editor), `Source/Water_Test.Target.cs` (game)
- No automated tests or lint commands.

## Source Layout

```
Source/Water_Test/
├── Water_TestCharacter.h/.cpp       # Base player character (abstract) — boat driving RPCs live here
├── Water_TestPlayerController.h/.cpp
├── Water_TestGameMode.h/.cpp
├── WaterTestGameState.h/.cpp
├── NetworkedActors/
│   ├── CLAUDE.md                    # Detailed boat implementation notes — read this
│   ├── ReplicatedPhysicsObject.h/.cpp  # Base class: server physics, client interpolation
│   └── Boats/
│       ├── BoatNetworked.h/.cpp         # Split physics/visual mesh approach (older)
│       ├── BoatNetworkedInterpAll.h/.cpp # Manual transform replication (active/current)
│       └── BoatAwareMovementComponent.h/.cpp  # Prevents character from pushing boat on collision
├── Boat/
│   └── GerstnerWaveGeneratorLayered.h/.cpp  # Multi-band Gerstner wave generator asset
├── Variant_Platforming/
├── Variant_Combat/
└── Variant_SideScrolling/
```

**Note**: `Source/Water_Test/NetworkedActors/CLAUDE.md` contains detailed implementation notes on the boat networking approach, water surface spring physics, commented-out features, and tuning properties — read it before touching boat code.

## Active Boat System (`ABoatNetworkedInterpAll`)

The current boat being developed. Physics runs **server-only**; clients receive replicated `ServerTransform` and interpolate.

**Boat driving flow:**
1. Player walks into `DrivingTrigger` (BoxComponent on boat) → overlap event fires
2. Player presses Enter/Exit key → `AWater_TestCharacter::DoEnterExitBoat()` → `ServerEnterBoat()` RPC
3. Server calls `ABoatNetworkedInterpAll::SetDriver()`, sets `ControlledBoat` on character
4. Character move input → `DoDriveBoat(Throttle, Steering)` → `ServerDriveBoat()` RPC → `ApplyDrivingInput()` on boat
5. Player leaves trigger zone → `CheckDriverInZone()` calls `ServerExitBoat()` on character

**Wave surface**: `OceanBody` must be assigned in editor. Spring-damper force pulls boat toward wave height each tick (`SurfaceSpringStrength`, `SurfaceVerticalDamping`). Never enable `SetSimulatePhysics(true)` on a client — causes desync.

**`UBoatAwareMovementComponent`**: Overrides `ApplyImpactPhysicsForces` to prevent the character's movement component from shoving the boat mesh on collision. Used by `AWater_TestCharacter`.

**`UGerstnerWaveGeneratorLayered`**: Custom wave generator asset with independent spectral bands (swell, secondary swell, chop, ripple). Assign as the Generator on a `UGerstnerWaterWaves` asset. Each `FWaveLayerConfig` has its own seed offset so bands don't interfere.

**`AReplicatedPhysicsObject`**: Base class for any networked physics prop (boxes, crates, barrels). Replicates `ServerTransform` via `OnRep_ServerTransform`; clients interpolate toward it.

## Key Design Patterns

- **`Do*()` input abstraction**: Every character exposes `BlueprintCallable` virtual `Do*()` methods mirroring raw input handlers, so Blueprint UI widgets and hardware input share the same code path.
- **Server-only physics + client interpolation**: All physics simulation runs on the server. Clients set physics off, receive replicated transform, and lerp visuals. Pattern used by both boat classes and `AReplicatedPhysicsObject`.
- **AnimNotify-driven gameplay**: Combat timing (attack traces, combo windows, charged attack) is driven entirely by AnimNotify classes, not timers.
- **StateTree AI**: Combat and SideScrolling variants use UE5 StateTree with custom `*StateTreeUtility` task/evaluator classes.
- **Interface-based damage**: `ICombatDamageable` / `ICombatAttacker` decouple the player, enemies, and props.

## Module Dependencies (`Water_Test.Build.cs`)

Public: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`, `Water`, `PhysicsCore`

`PublicIncludePaths` covers all variant subdirectories and `Water_Test/Boat`, so headers can be included without path qualification. `NetworkedActors/` is NOT in `PublicIncludePaths` — include with relative path (e.g. `#include "NetworkedActors/Boats/BoatAwareMovementComponent.h"`).

## Prototype Variants (not part of the boat game)

Three self-contained gameplay prototypes, each with its own GameMode/Character/PlayerController:

- **`Variant_Platforming/`**: Third-person platformer — press-and-hold jump, double jump, wall jump (sphere trace), coyote time, dash with `AnimNotify_EndDash`
- **`Variant_Combat/`**: Melee combat — combo string via AnimMontage sections, charged attack, HP/ragdoll/respawn, UMG life bar as `UWidgetComponent`, StateTree AI enemies, EQS targeting
- **`Variant_SideScrolling/`**: Side-scroller — constrained axis, drop-through soft platforms (custom `SoftCollisionObjectType` channel), interact system via `ISideScrollingInteractable`, StateTree AI NPCs
