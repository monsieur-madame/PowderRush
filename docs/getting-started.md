# PowderRush: Getting Started & Level Workflow

## Table of Contents

- [Project Overview](#project-overview)
- [Code Architecture](#code-architecture)
- [Creating a New Level](#creating-a-new-level)
- [The Tag System](#the-tag-system)
- [Course Path Setup](#course-path-setup)
- [Environment & Weather](#environment--weather)
- [Obstacles](#obstacles)
- [Movement Tuning](#movement-tuning)
- [Runtime Flow](#runtime-flow)
- [Troubleshooting](#troubleshooting)

---

## Project Overview

PowderRush is an arcade ski/snowboard game built in UE5 C++. Levels are **fully authored in the editor** — there is no procedural generation at runtime. You build terrain, lay a spline path down the mountain, place obstacles and weather volumes, and the runtime systems handle player movement, scoring, and weather transitions.

### Key Source Directories

```
Source/PowderRush/
  Core/        Game mode, environment setup, shared types
  Player/      Character, movement component, controller, tricks
  Terrain/     Course path (spline), terrain manager (query service), obstacles
  Effects/     Weather volumes, weather manager, materials, particles
  Scoring/     Score subsystem, combo tracking
  Pickup/      Powerups, collectibles
  UI/          Canvas HUD, menus, dev tuning overlay
  Meta/        Save/load, progression
```

---

## Code Architecture

### Core Classes

| Class | Role |
|---|---|
| `APowderGameMode` | Run lifecycle state machine (InMenu → Running → ScoreScreen). Finds required actors, positions player, drives weather updates. |
| `APowderCharacter` | Player pawn with skier mesh, diorama camera, snow spray effect. |
| `UPowderMovementComponent` | Physics-based skiing movement. Traces for terrain, queries course heading, applies carving/speed/boost. **This is the most important system.** |
| `APowderPlayerController` | Touch and keyboard input — binary left/right carve, ollie, trick gestures. |
| `ATerrainManager` | Runtime query service. Reads the authored spline, provides spawn/respawn positions, course projection, surface queries. |
| `APowderCoursePath` | Editor actor with a `USplineComponent` defining the canonical run path. Has editor tools for snapping, resampling, boundary tree generation, and finish line placement. |
| `APowderEnvironmentSetup` | Spawns (or reuses) directional light, sky dome, sky light, and fog. Creates the `UPowderWeatherManager` component. |
| `APowderWeatherVolume` | Placed box volumes that define weather regions with priority-based blending. |
| `APowderHUD` | Canvas-based HUD with gameplay overlay, menus, and dev tuning panel. |

### Data Flow

```
APowderCoursePath (editor-placed spline)
        │
        ▼
ATerrainManager::InitializeCourse()  ──  samples spline into runtime data
        │
        ├──► GetSlopeStartPosition()    → GameMode uses for spawn
        ├──► SampleCourseFrameAtWorldPosition() → MovementComponent uses for heading
        ├──► GetRespawnPosition()        → GameMode uses after wipeout
        └──► GetPlayerDistance()         → HUD/scoring for progress
```

---

## Creating a New Level

### Step 1: Create the Level

Create a new level in the UE5 editor. Set the game mode override to `BP_PowderGameMode` (or `APowderGameMode` directly) in World Settings.

### Step 2: Build Terrain

Create your ski slope using Landscapes, static meshes, or any geometry.

**Critical:** Tag your terrain actor with `PowderTerrain`. Select the actor → Details → Actor → Tags → add `PowderTerrain`. Without this tag, the player cannot ski on the surface (the movement system only traces for tagged geometry).

### Step 3: Place Required Actors

Every playable level needs exactly three actors:

#### APowderEnvironmentSetup
Handles lighting, sky, and fog. Place one in the level.

| Property | Default | Description |
|---|---|---|
| `bReuseExistingSceneLights` | `true` | Looks for existing lights before spawning new ones |
| `DynamicShadowDistance` | `8000` | Cascaded shadow map distance |

#### ATerrainManager
The runtime query service. Place one in the level.

| Property | Default | Description |
|---|---|---|
| `bAutoInitializeCourseOnBeginPlay` | `true` | Automatically finds and samples the course path |
| `CoursePathSampleStep` | `500` | Spline sampling resolution (smaller = more precise, more memory) |
| `SpawnHeightOffset` | `100` | Height above spline for spawn/respawn |
| `SpawnFacingYawOffset` | `0` | Extra yaw rotation at spawn |

#### APowderCoursePath
The authored spline defining the run. Place one in the level and add spline points down the mountain. Point 0 is the start (top), the last point is the end (bottom near the finish line).

| Property | Default | Description |
|---|---|---|
| `CourseName` | `"DefaultCourse"` | Human-readable name |
| `StartDistanceOffset` | `0` | Distance along spline where the player spawns |
| `RespawnBacktrackDistance` | `1500` | How far uphill to place the player after a wipeout |
| `WeatherBreakpointsNormalized` | `[0.25, 0.55, 0.80]` | Course-progress thresholds for weather transitions |

### Step 4: Design the Course Spline

With the `APowderCoursePath` selected, use the spline editing tools in the viewport to lay out your course. Then use the built-in editor tools (buttons in the Details panel):

1. **SnapPointsToPowderTerrain** — Traces each spline point downward and snaps it to the `PowderTerrain`-tagged surface.
2. **ResampleSplineUniformly** — Rebuilds the spline with evenly spaced points (controlled by `ResampleStepDistance`).
3. **ReverseSplineDirection** — Flips start/end if you built it backwards.

### Step 5: Place a Finish Line

Either:
- Click **PlaceFinishLineAtEnd** on the CoursePath actor (auto-spawns an `APowderFinishLine` near the spline end, snapped to terrain), or
- Manually place an `APowderFinishLine` and position it.

Finish line properties on the CoursePath:

| Property | Default | Description |
|---|---|---|
| `FinishLineWidth` | `5000` | Width of the trigger box |
| `FinishLineDistanceFromEnd` | `200` | Offset back from spline end |
| `FinishLineHeightOffset` | `20` | Vertical offset above terrain |

### Step 6: Place Obstacles

Drag `APowderTree` and `APowderRock` actors into the level along the course. Both automatically tag themselves with `PowderObstacle` so the movement system knows to trigger a wipeout on collision.

For boundary trees along the course edges, use **GenerateBoundaryTrees** on the CoursePath actor (requires assigning meshes to `BoundaryTreeMeshes`).

### Step 7: Add Weather Volumes (Optional)

Place `APowderWeatherVolume` actors to create weather regions. See [Environment & Weather](#environment--weather) below.

### Step 8: Play-Test

Hit Play. The game starts in `InMenu` state with the player frozen at the spline start. Tap "Start" (or press any key on desktop) to begin skiing.

---

## The Tag System

PowderRush uses actor/component tags for runtime identification:

| Tag | Purpose | Required On |
|---|---|---|
| `PowderTerrain` | Identifies skiable surfaces. The movement component only snaps to geometry with this tag. | Terrain actors or their mesh components |
| `PowderObstacle` | Identifies collision obstacles that trigger wipeout. | Obstacle actors (auto-applied by `APowderTree`, `APowderRock`) |
| `PowderGenerated` | Marks auto-generated actors (boundary trees). | Applied automatically by CoursePath tools |
| `PowderBoundaryTree` | Identifies boundary tree HISM components. | Applied automatically by CoursePath tools |

### Making Custom Obstacles

Any actor can be an obstacle if it:
1. Has the `PowderObstacle` tag on the actor or a collision component.
2. Has collision enabled (`QueryAndPhysics`) and blocks the Pawn channel.

---

## Course Path Setup

### Editor Tool Buttons

These appear as clickable buttons in the Details panel when an `APowderCoursePath` is selected:

| Tool | What It Does |
|---|---|
| **SnapPointsToPowderTerrain** | Traces each spline point downward onto `PowderTerrain`-tagged geometry |
| **ResampleSplineUniformly** | Rebuilds the spline with uniform point spacing at `ResampleStepDistance` intervals |
| **ReverseSplineDirection** | Flips the spline start and end |
| **PlaceFinishLineAtEnd** | Spawns a finish line actor near the spline end, snapped to terrain |
| **GenerateBoundaryTrees** | Creates HISM tree lines along both edges of the spline |
| **ClearBoundaryTrees** | Removes all generated boundary tree components |

### Boundary Tree Properties

| Property | Default | Description |
|---|---|---|
| `BoundaryTreeMeshes` | (empty) | Array of static meshes to use — must be assigned for generation to work |
| `BoundaryEdgeOffset` | `2600` | Distance from spline centerline to boundary |
| `BoundaryTreeSpacing` | `700` | Spacing between trees along the spline |
| `BoundaryAlongJitter` | `160` | Random forward/back offset |
| `BoundaryLateralJitter` | `120` | Random side-to-side offset |
| `BoundaryYawJitter` | `35` | Random rotation variation (degrees) |
| `BoundarySeed` | `1337` | Deterministic RNG seed for reproducibility |

---

## Environment & Weather

### Weather Volumes

`APowderWeatherVolume` actors define weather regions in the level. Each has a box extent and a full `FWeatherConfig`.

| Property | Default | Description |
|---|---|---|
| `bEnabled` | `true` | Toggle this volume |
| `Priority` | `0` | Higher priority wins when volumes overlap |
| `BlendDistance` | `2000` | Falloff distance outside the box (linear blend) |
| `Config` | ClearDay | Full weather configuration struct |

**Blending rules:**
- Player inside a box = weight 1.0
- Player outside but within `BlendDistance` = linear falloff
- Only volumes at the **highest active priority** participate
- Multiple same-priority volumes blend by normalized weight
- No active volumes = fallback to ClearDay

### FWeatherConfig Properties

| Property | Default | Description |
|---|---|---|
| `Preset` | `ClearDay` | One of: ClearDay, Overcast, Snowfall, Blizzard, Sunset, Twilight |
| `SunColor` | warm white | Directional light color |
| `SunIntensity` | `4.0` | Directional light intensity |
| `SunRotation` | `(-40, -60, 0)` | Sun angle |
| `SkyColor` | sky blue | Sky dome tint |
| `SkyLightIntensity` | `1.0` | Ambient light intensity |
| `FogDensity` | `0.002` | Exponential height fog density |
| `FogColor` | cool blue-white | Fog tint |
| `FogStartDistance` | `2000` | Where fog begins |
| `FogHeightFalloff` | `0.2` | Vertical fog density curve |
| `SnowfallRate` | `0.0` | Snowfall particle rate (0 = none) |
| `WindStrength` | `0.0` | Wind intensity |
| `WindDirection` | `0.0` | Wind heading (degrees) |
| `TransitionTime` | `5.0` | Blend duration when transitioning |

---

## Obstacles

### Built-in Types

**APowderTree** — Procedurally assembled from basic shapes (cylinder trunk + cone foliage layers + optional snow cap). Use `Randomize()` with `FProceduralTreeParams` for variation.

**APowderRock** — Built from cube meshes (primary rock + cluster pieces + optional snow cover). Use `Randomize()` with `FProceduralRockParams` for variation.

Both auto-tag themselves with `PowderObstacle` and have proper collision set up.

**APowderJump** — Ski jump ramp. Place on the slope and the player rides up it naturally, going airborne off the lip. The ramp mesh auto-tags itself with `PowderTerrain` so terrain following works. A thin trigger at the lip transitions the player to airborne state with their natural velocity (no forced impulse — the arc comes from the ramp geometry). Players can ollie on the ramp for extra pop.

| Property | Default | Description |
|---|---|---|
| `RampLength` | `400` | Front-to-back ramp size (cm) |
| `RampWidth` | `300` | Left-to-right ramp size (cm) |
| `RampHeight` | `150` | Vertical rise — pitch angle auto-calculated from height/length |
| `LaunchSpeedMultiplier` | `0.6` | (Legacy, not used with natural launch) |

### Placement Tips

- Place obstacles along the course where you want challenge areas.
- Keep obstacles near the spline path — the player follows the course heading so obstacles too far off the sides won't be encountered.
- Use boundary trees (via CoursePath's `GenerateBoundaryTrees`) to line the course edges and prevent the player from skiing off into the void.

---

## Movement Tuning

All movement parameters live on `UPowderMovementComponent` under the `"PowderRush|Movement|Tuning"` category. Key parameters:

### Speed
| Parameter | Default | What It Controls |
|---|---|---|
| `GravityAcceleration` | `980` | Base gravity (cm/s^2) |
| `MaxSpeed` | `3000` | Hard speed cap |
| `BaseFriction` | `0.02` | Baseline friction coefficient |
| `GravityAlongSlopeScale` | `0.45` | Scales gravity projection along slope (feel > realism) |

### Carving
| Parameter | Default | What It Controls |
|---|---|---|
| `MaxCarveAngle` | `50` | Maximum carve turn angle |
| `YawRate` | `90` | Turn speed at full carve (deg/s) |
| `CarveLateralSpeed` | `700` | Sideways drift during carves |
| `CarveSpeedBleed` | `0.70` | Speed penalty from carving |
| `TurnRateLimitDegPerSec` | `130` | Max heading change rate |
| `DownhillAlignRate` | `28` | How fast heading returns to fall line after carve release |
| `EdgeDisengageRate` | `1.8` | Edge release speed (lower = more lateral carry) |
| `HeadingTraverseFactor` | `0.15` | Gravity at 90° traverse (0=no accel, 1=full) |
| `CarveLeanMaxAngle` | `25` | Max visual body lean into turns (degrees) |
| `SlopePitchInterpSpeed` | `8` | How fast slope pitch adjusts |
| `CarveRampTime` | `0.6` | Ease-in duration for carve input |
| `CourseHeadingBlend` | `0.85` | 0 = pure gravity downhill, 1 = pure course spline heading |

### Boost & Ollie
| Parameter | Default | What It Controls |
|---|---|---|
| `BoostBurstSpeed` | `1500` | Speed added during boost |
| `BoostDuration` | `0.5` | Boost length (seconds) |
| `OllieForce` | `450` | Vertical jump impulse |
| `OllieCooldown` | `1.0` | Seconds between ollies |

### Terrain Following
| Parameter | Default | What It Controls |
|---|---|---|
| `TerrainTraceDistance` | `1000` | How far down to trace for ground |
| `TerrainContactOffset` | `-6` | Z adjustment after snap |
| `GroundNormalFilterSpeed` | `12` | How fast the terrain normal smooths |
| `LedgeLaunchThreshold` | `50` | Height separation (cm) that triggers airborne transition |

**Gravity-based following**: Terrain only snaps the player UP (going uphill / riding ramps). When terrain drops away (bumps, ledges, ramp lips), gravity pulls the player down naturally, preserving any upward slope velocity. If the player separates from terrain by more than `LedgeLaunchThreshold`, they transition to full airborne physics. This makes all terrain transitions feel natural without special-case code.

### Data-Driven Tuning

Use `UPowderTuningProfile` data assets to package `FMovementTuning` + `FCameraTuning` + `BlendTime`. Apply at runtime via `ApplyTuningProfile()` for smooth transitions between tuning sets (e.g., different feels for powder vs. ice sections).

---

## Runtime Flow

### Startup Sequence

1. `APowderEnvironmentSetup::BeginPlay()` — spawns/finds lights, sky, fog; creates `UPowderWeatherManager`.
2. `ATerrainManager::BeginPlay()` — calls `InitializeCourse()`:
   - Finds the `APowderCoursePath` via `TActorIterator`.
   - Samples the spline at `CoursePathSampleStep` intervals into `RuntimeSplineSamples`.
   - Caches start offset, respawn distance, weather breakpoints, total course length.
3. `APowderGameMode::BeginPlay()` — finds EnvironmentSetup and TerrainManager, positions player at spline start, freezes player, enters `InMenu` state.

### Game Loop

```
InMenu ──[Start]──► Starting ──► Running ──[Finish]──► ScoreScreen ──► InMenu
                                    │
                                    ├──[Pause]──► Paused ──[Resume]──► Running
                                    │
                                    └──[Crash]──► WipedOut ──[1.5s]──► Running
```

### Per-Frame (Running State)

- **TerrainManager** projects the player position onto the spline to track progress.
- **MovementComponent** traces for `PowderTerrain`, queries course heading from TerrainManager, applies physics.
- **GameMode** polls `APowderWeatherVolume::GetWeatherAtLocation()` and drives weather transitions.
- **HUD** reads player distance, speed, score from the relevant systems.

### Wipeout & Respawn

1. Player hits a `PowderObstacle`-tagged object → `OnWipeout` broadcasts.
2. GameMode freezes player, enters `WipedOut` state.
3. After 1.5s delay, `RespawnPlayer()`:
   - `TerrainManager->GetRespawnPosition()` — backtracks `RespawnBacktrackDistance` from current progress.
   - Teleports player, unfreezes, returns to `Running`.

---

## Troubleshooting

| Problem | Likely Cause |
|---|---|
| Player falls through terrain | Terrain not tagged with `PowderTerrain` |
| Player spawns at world origin | No `APowderCoursePath` in the level, or spline has fewer than 2 points |
| No wipeout on obstacle collision | Obstacle missing `PowderObstacle` tag or collision not set to block Pawn |
| Spline snap tool does nothing | Terrain not tagged with `PowderTerrain` |
| Weather doesn't change | No `APowderWeatherVolume` actors placed, or volumes are disabled |
| Boundary trees not generating | `BoundaryTreeMeshes` array is empty on the CoursePath actor |
| Game stuck in InMenu | No "Start" button tap / TerrainManager or EnvironmentSetup missing from level |
| Movement feels wrong | Check `CourseHeadingBlend` — values near 1.0 follow the spline closely, near 0.0 follow raw gravity |
| Player goes through jump ramp | Ramp mesh missing `PowderTerrain` component tag (auto-applied in code, but check if overridden) |
| Jump launches player into ground | Ramp not tagged with `PowderTerrain` — terrain trace ignores ramp and snaps player to ground below |
| Camera clips into terrain | Increase `CameraHeightOffset` on the character, or check that `bDoCollisionTest` is enabled on the spring arm |
| Character lean looks wrong | Rotation must use quaternion composition (Heading * Tilt * MeshOffset) when VisualYawOffset != 0 |
