# Boat Module

Physics-driven, multiplayer cooperative boat system using UE5's Water + Buoyancy plugins.

## Two Classes

### `ABoatNetworked`
Split physics/visual mesh approach:
- **PhysicsMesh** (root): simulates physics on server only, replicated via `SetReplicateMovement(true)`
- **VisualMesh**: child component with absolute world transform — detached from the physics mesh so it can be smoothed independently on clients
- Client: physics off, PhysicsMesh hidden, VisualMesh interpolates toward replicated PhysicsMesh transform each tick
- Server: runs `clampRaftPhysics()` — clamps roll/pitch angular velocity to ±40°/s to prevent flipping

### `ABoatNetworkedInterpAll`
Single mesh, manual transform replication approach:
- One mesh (`Mesh`) simulates physics on server only
- `SetReplicateMovement(false)` — transform is manually replicated via `DOREPLIFETIME(ServerTransform)`
- `OnRep_ServerTransform()` sets `TargetTransform`; clients interpolate toward it each tick via `InterpBoat()`
- Has damping set in constructor (LinearDamping=0.3, AngularDamping=2.5); `ABoatNetworked` has these commented out
- This is the **active/current** boat being developed — it has the water surface interaction logic

## Water Surface (InterpAll only)
`ApplyWaterSurfaceForce()` runs server-side each tick when `OceanBody` is assigned:
- Queries wave height via `TryQueryWaterInfoClosestToWorldLocation` with `IncludeWaves` flag
- Applies a spring-damper force: `Force = (SpringStrength * Displacement) - (Damping * VerticalVel)` with `bAccelChange=true` (mass-independent)
- **No `IsInWater()` guard** — intentional, so the boat tracks the wave surface even when above it instead of freefalling
- `SnapToWaterSurface()` exists as an alternative (hard Z snap via `ETeleportType::TeleportPhysics`) but is currently commented out in Tick — was tried and caused jitter

## Commented-Out / Shelved Code
- **Player boarding detection** (`CheckPlayersOnBoat`): was going to enable physics only when N players are on the boat (`GetMovementBase() == Mesh`). Commented out, not deleted — likely to return.
- **River spline centering** (`PushBoatToSpline`, `CurrentRiver`): applied a lateral force toward the nearest river spline point. River was removed from the project; river-detection overlap events are commented out. `PushBoatToSpline()` still compiles but is never called.
- **Overlap events** for river detection: fully commented out in both `.h` and `.cpp`

## Key Tuning Properties (InterpAll)
| Property | Default | Purpose |
|---|---|---|
| `InterpSpeed` | 8.0 | Client interp speed toward server transform |
| `SurfaceSpringStrength` | 800 | Spring stiffness toward wave surface |
| `SurfaceVerticalDamping` | 4.0 | Damping on vertical velocity |
| `WaterSurfaceOffset` | 0 | Z offset above wave surface |
| `CenteringForce` | 2000 | Lateral force toward river spline (unused) |

## Networking Pattern
Both classes: physics runs **server only**, clients receive replicated state and interpolate visuals. Never enable `SetSimulatePhysics(true)` on a client — this will cause the boat to desync.

`OceanBody` must be assigned in the editor (not auto-detected at runtime) for wave following to work.
