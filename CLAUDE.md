# PowderRush - Project Conventions

## Overview
Arcade skiing/snowboarding mobile game built in UE5 (C++). Target: iOS/Android, F2P, 60fps on mid-range phones.

## Project Structure
```
PowderRush/
  Source/PowderRush/          - C++ source code
    PowderRush.h/.cpp         - Primary game module
    Core/                     - Game mode, game state, game instance
    Player/                   - Character, movement component, controller
    Terrain/                  - Zone generation, tile system, obstacles
    Scoring/                  - Score manager, combo system, near-miss detection
    Pickup/                   - Coins, power-ups, collectibles
    UI/                       - HUD, menus, shop, daily challenges
    Meta/                     - Progression, save/load, currency, unlocks
    Effects/                  - Snow trails, particles, camera effects
    Audio/                    - Sound manager, adaptive music
  Config/                     - Engine/Game/Input configuration
  Content/                    - UE5 assets (Blueprints, meshes, materials, etc.)
  docs/plan/                  - Game design documents
```

## Coding Conventions

### Naming
- **Classes**: UE5 prefix convention (A for Actors, U for UObjects, F for structs, E for enums)
- **Project prefix**: `Powder` for all game-specific classes (e.g., `APowderCharacter`, `UPowderMovementComponent`)
- **Files**: PascalCase matching class name without prefix (e.g., `PowderCharacter.h`)
- **Variables**: PascalCase for UPROPERTY, camelCase for locals
- **Functions**: PascalCase

### C++ Style
- Use `#pragma once` instead of include guards
- UPROPERTY/UFUNCTION macros for all Blueprint-exposed members
- Category = "PowderRush|SubCategory" for property organization
- Prefer `TObjectPtr<>` over raw pointers for UObject properties
- Use `GENERATED_BODY()` (not `GENERATED_UCLASS_BODY()`)

### Architecture Patterns
- **C++ base classes** with Blueprint extensions for gameplay tuning
- **Game Instance Subsystems** for persistent systems (scoring, progression)
- **Enhanced Input System** for all input handling
- **Niagara** for all particle effects
- **Common UI plugin** for mobile-friendly widgets

### Mobile Constraints
- Forward Shading renderer (no Lumen/Nanite)
- Baked lighting only (Lightmass)
- Target < 500MB memory usage
- Target < 16ms frame time (60fps)
- Texture streaming enabled
- Mesh LODs required for all visible assets

### Key Classes
- `APowderGameMode` - Main game mode, run lifecycle (state machine: InMenu→Starting→Running→Paused/WipedOut→ScoreScreen)
- `APowderCharacter` - Player character with Skier mesh, diorama camera, snow spray
- `UPowderMovementComponent` - Custom physics-based movement (CORE SYSTEM), freeze support, ollie, tuning profile blending
- `APowderPlayerController` - Touch + keyboard input, binary left/right carve with ease-in ramp, gesture tricks, double-tap ollie
- `UPowderGameInstance` - Persistent game state, save/load via UPowderSaveGame, lifetime stats
- `UScoreSubsystem` - Scoring, combo tracking, run stats (Game Instance Subsystem)
- `UPowderTrickComponent` - Trick system (flips, spins, spread eagle) with mesh rotation
- `UPowderSaveGame` - Save game with FLifetimeStats (best-of + cumulative)
- `APowderHUD` - Canvas-based HUD with button hit-testing, menus (main/pause/stats/score), gameplay overlay
- `APowderEnvironmentSetup` - Spawns slope, lighting, sky, fog, trees, rocks
- `APowderFinishLine` - Finish line trigger (tall box to catch airborne players)
- `APowderPowerup` - Speed boost + score multiplier pickups

### Build Targets
- **Editor**: Development builds for iteration
- **iOS**: Shipping builds for iPhone 12+ (Metal)
- **Android**: Shipping builds for Galaxy S21+ (Vulkan) - Phase 6+

### UE5 Documentation
- Engine API docs: `/Users/toto/Documents/Epic Games/UE_5.7/Engine/Documentation/Source`

## Current Phase
Phase 2 complete. Core loop works: menus, skiing, tricks, ollie, powerups, crash/respawn, save system, score screen.

## Game State Machine
`EPowderRunState`: InMenu → Starting → Running → (Paused | WipedOut → respawn → Running | finish → ScoreScreen) → InMenu
- Player is frozen (movement disabled) in all non-Running states
- Crash/wipeout: freeze → 1.5s delay → respawn at slope start → resume Running (run continues, score preserved)
- Pause/Resume freezes/unfreezes movement component

## Input Design
- **Touch**: Binary left/right (screen halves), ease-in ramp over CarveRampTime. Double-tap for ollie. Swipe gestures for airborne tricks. Two-finger hold for SpreadEagle.
- **Keyboard**: A/D or arrows for carve, W/S for tricks, Space for ollie, Escape for pause, R for restart.
- User preference: binary touch over analog — do not change to analog distance-from-center.

## Tuning System
- `FMovementTuning` struct in `PowderTypes.h` — all movement params are data-driven
- `UPowderTuningProfile` data asset with `FMovementTuning` + `FCameraTuning` + BlendTime
- Default profile loaded via ConstructorHelpers in PowderCharacter constructor
- `ApplyTuningProfile()` smoothly blends between tuning sets over time

## Important Notes
- The game lives or dies on how the skiing FEELS. Movement component is the #1 priority.
- Test on device early and often - desktop feel != mobile feel.
- Keep draw calls low, keep materials simple, keep meshes low-poly.
- FRotator Lerp normalizes to [-180,180] — use component-wise float Lerp for 360° rotations (see PowderTrickComponent).
