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
- `APowderGameMode` - Main game mode, run lifecycle
- `APowderCharacter` - Player character (ski/snowboard)
- `UPowderMovementComponent` - Custom physics-based movement (CORE SYSTEM)
- `APowderPlayerController` - Touch input handling
- `UPowderGameInstance` - Persistent game state
- `UScoreSubsystem` - Scoring and combo tracking (Game Instance Subsystem)
- `ATerrainManager` - Zone tile loading and stitching
- `APowderCameraManager` - Dynamic camera with speed-based effects

### Build Targets
- **Editor**: Development builds for iteration
- **iOS**: Shipping builds for iPhone 12+ (Metal)
- **Android**: Shipping builds for Galaxy S21+ (Vulkan) - Phase 6+

## Current Phase
Phase 1: Prototype Core Feel - Make skiing feel amazing on a simple slope.

## Important Notes
- The game lives or dies on how the skiing FEELS. Movement component is the #1 priority.
- Test on device early and often - desktop feel != mobile feel.
- Keep draw calls low, keep materials simple, keep meshes low-poly.
