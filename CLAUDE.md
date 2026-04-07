# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Water_Test** is an Unreal Engine 5.7 game project (single `Water_Test` runtime module) that serves as a multi-variant gameplay prototype. It uses the Water, Buoyancy, StateTree, and GameplayStateTree plugins alongside UMG for UI and EnhancedInput for controls.
In general making a boat physics game. It is going to be mutliplayer coopritive. Boat behaves with phsyics. 

## Building

Build from UE5 via the Editor or UnrealBuildTool. There are no standalone build scripts — use the standard UE5 workflow:

- **Editor build**: Open `Water_Test.uproject` in Unreal Engine 5.7
- **Command-line build** (from UE install directory):
  ```
  UnrealBuildTool.exe Water_Test Win64 Development "path/to/Water_Test.uproject"
  ```
- **Editor target**: `Source/Water_TestEditor.Target.cs` (for editor builds)
- **Game target**: `Source/Water_Test.Target.cs`

There are no automated tests or lint commands in this project.

## Architecture: Three Game Variants

The `Source/Water_Test/` directory contains three independent gameplay variants, each with its own GameMode, Character, PlayerController, and supporting classes:

### `Variant_Platforming/`
Third-person platformer character (`APlatformingCharacter`) with:
- Press-and-hold jump, double jump, wall jump (sphere trace), coyote time
- Dash with AnimMontage and `AnimNotify_EndDash`
- All input actions exposed as `Do*()` BlueprintCallable virtuals for UI or interface control

### `Variant_Combat/`
Third-person melee combat character (`ACombatCharacter`) implementing `ICombatAttacker` and `ICombatDamageable`:
- Combo attack string driven by AnimMontage sections (`ComboSectionNames`)
- Press-and-hold charged attack with loop section
- Attack tracing via `AnimNotify_DoAttackTrace`, combo gating via `AnimNotify_CheckCombo`, charged hold check via `AnimNotify_CheckChargedAttack`
- HP, ragdoll-on-death, respawn timer
- `CombatLifeBar` UMG widget rendered as a `UWidgetComponent` on the character
- AI: `CombatAIController` + `CombatEnemy` (enemy character), `CombatEnemySpawner`, StateTree-based behavior via `CombatStateTreeUtility`
- EQS: `EnvQueryContext_Player` and `EnvQueryContext_Danger` for enemy targeting
- Gameplay volumes: `CombatActivationVolume`, `CombatCheckpointVolume`, `CombatLavaFloor`
- Interface hierarchy: `ICombatActivatable`, `ICombatAttacker`, `ICombatDamageable`

### `Variant_SideScrolling/`
Side-scrolling character (`ASideScrollingCharacter`) with:
- Constrained axis movement, wall jump, double jump, coyote time
- Soft platform drop-through (custom collision channel `SoftCollisionObjectType`)
- Interact system via `ISideScrollingInteractable` interface; sphere-overlap interaction check
- `SideScrollingCameraManager` for camera management
- AI: `SideScrollingAIController` + `SideScrollingNPC`, StateTree via `SideScrollingStateTreeUtility`
- Gameplay actors: `SideScrollingJumpPad`, `SideScrollingMovingPlatform`, `SideScrollingPickup`, `SideScrollingSoftPlatform`
- `SideScrollingUI` UMG widget

### `Boat/`
- `ABoatNetworked` / `ABoatNetworkedInterpAll`: Physics-driven boat actors using a split physics/visual mesh with interpolation and physics clamping (Water + Buoyancy plugin integration)
- `AWater_TestGameMode` / `AWater_TestCharacter` / `AWater_TestPlayerController`: Base/default classes

## Key Design Patterns

- **Input abstraction**: Every variant character exposes `Do*()` BlueprintCallable virtual methods that mirror the raw input handlers. This allows Blueprint UI widgets to drive the same code paths as hardware input.
- **AnimNotify-driven gameplay**: Combat timing (attack traces, combo windows, charged attack checks) is entirely driven by AnimNotify classes rather than timers, keeping animation and logic in sync.
- **StateTree AI**: Both Combat and SideScrolling variants use UE5 StateTree for enemy AI behavior, with custom `*StateTreeUtility` task/evaluator classes.
- **Interface-based damage**: Combat damageable actors implement `ICombatDamageable`; attackers implement `ICombatAttacker`. This decouples enemies, the player, and props (e.g., `CombatDamageable Box`, `CombatDummy`).

## Module Dependencies

Declared in `Source/Water_Test/Water_Test.Build.cs`:
- Public: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, `Slate`, `Water`, `PhysicsCore`
- All variant subdirectories are added to `PublicIncludePaths`, so headers can be included without path qualification.
