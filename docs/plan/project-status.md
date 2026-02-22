# PowderRush - Project Status & Roadmap

Last updated: 2026-02-22

---

## What's Done (Fully Functional)

### Core Gameplay Loop
- **Game state machine** (InMenu / Starting / Running / Paused / WipedOut / ScoreScreen) — all transitions working
- **Run lifecycle**: start, pause/resume, wipeout (1.5s delay then full restart), finish line, quit to menu
- **Player freeze/unfreeze** gated by run state

### Player Systems
- **PowderMovementComponent** (~1000 lines) — production-quality ski physics
  - Gravity-driven downhill acceleration with slope angle
  - Carving: angle ramping, edge depth/pressure, turn commit, speed bleed
  - Edge transitions (low-grip phase when switching edges)
  - Speed-dependent turn rate limiting
  - Boost: meter fills from carving, burst activation, duration timer
  - Ollie: upward impulse with cooldown
  - Airborne physics: drag, terminal velocity, landing blend
  - Landing penalties: speed loss + temporary steering reduction + quality scoring
  - Terrain following: line trace, ground normal filtering, snap threshold
  - Surface property blending (smooth transitions between surface types)
  - Tuning profile blending (smooth transitions between movement feel presets)
  - Heading system: downhill alignment, yaw lag, heading-based friction
- **PowderTrickComponent** — 5 trick types (backflip, frontflip, heli-spin L/R, spread eagle)
  - Gesture recognition (swipe up/down/left/right, two-finger hold)
  - Mesh rotation with eased interpolation, component-wise lerp for 360 spins
  - Chain tracking with multiplier
  - Landing validation (must complete animation before touchdown)
  - Events: OnTrickCompleted, OnTrickFailed
- **PowderPlayerController** — full touch + keyboard input
  - Binary left/right carve (screen halves) with ease-in ramp
  - Double-tap ollie, swipe gestures for tricks, two-finger spread eagle
  - Keyboard fallback (A/D, W/S, Space, Esc, R)
  - Side-switch cooldown & grace time
  - State-gated (only processes input in Running state)
- **PowderCharacter** — static mesh skier, spring arm camera, snow spray Niagara component
  - Camera: arm length, pitch, FOV, yaw lag — all speed-responsive
  - Feel preset ladder: 5 tuning profiles loaded, switchable via dev menu

### Terrain & Course
- **TerrainManager** — runtime course query service
  - Reads placed APowderCoursePath spline, builds runtime samples
  - Player distance tracking every tick
  - Course projection (world position to spline distance)
  - Respawn positioning (backtrack from current progress)
  - Surface property querying via IPowderSurfaceQueryProvider
- **PowderCoursePath** — authored spline with editor tools
  - Snap points to terrain, resample uniformly, reverse direction
  - Auto-place finish line at end
  - Generate/clear boundary trees (HISM for efficient rendering)
  - Weather breakpoint editing
- **PowderFinishLine** — tall box trigger catches airborne players
- **PowderAvalancheComponent** — accelerating chase threat
  - Position tracks along course spline independently of player
  - Speed = BaseSpeed + AccelerationRate * ElapsedTime, capped at MaxSpeed
  - Catch detection: gap < CatchDistance triggers wipeout
  - Responds to all run state transitions (pause/resume/reset)
  - HUD proximity bar (orange to red) with pulsing "AVALANCHE!" warning

### Scoring & Progression
- **ScoreSubsystem** (GameInstance subsystem) — full combo scoring
  - Score actions: deep carve, near-miss, gate pass, trick, boost, speed bonus, air time
  - Multiplier: builds from actions, decays on timeout, resets on wipeout
  - Powerup multiplier stacking
  - Run stats: distance, tricks, near-misses, coins, best combo, highest multiplier
- **PowderGameInstance** — persistent state across runs
  - Currency (coins, gems), high score, lifetime stats
  - Save/load via PowderSaveGame (async, versioned)
- **PowderSaveGame** — FLifetimeStats with best-of and cumulative tracking
  - Best: score, distance, combo chain, tricks, near-misses, multiplier, air time
  - Cumulative: coins earned, tricks landed, near-misses, gates passed, total distance

### UI
- **PowderHUD** — canvas-based, mobile-friendly
  - Main menu (play, stats, dev), pause menu (resume, restart, quit), score screen, stats screen
  - Gameplay overlay: score, multiplier, combo timer bar, speed/boost bars, airborne indicator
  - Powerup indicator with timer bar and flash text
  - Avalanche proximity bar with warning
  - Trick notification popups
  - Button hit-testing with touch padding
  - iOS retina coordinate fix (viewport-to-canvas offset)
  - DPI-aware UI scaling (design ref 1080x1920)
  - Dev tuning menu: single-column layout, 57+ params, scrollable, feel lab with preset switching

### Effects & Weather
- **PowderWeatherManager** — smooth weather transitions
  - 6 presets: ClearDay, Overcast, Snowfall, Blizzard, Sunset, Twilight
  - Lerps sun, sky, fog, snowfall, wind between configs
  - Ambient snowfall Niagara component (spawned on demand)
- **PowderWeatherVolume** — map-placed weather regions
  - Box triggers with priority and blend distance
  - GameMode evaluates player position each tick, applies blended weather
- **PowderEnvironmentSetup** — spawns lighting, sky, fog components
- **PowderSnowSpray** — Niagara carve spray (direction, amount, color from surface)

### Pickups
- **CoinPickup** — bob/rotate animation, collection trigger, sparkle VFX (missing: sound)
- **PowderPowerup** — SpeedBoost and ScoreMultiplier types, tunable durations

### Obstacles
- **PowderTree** — trunk + foliage + snow cap mesh components
- **PowderRock** — cube clusters with snow cover
- **PowderJump** — launch trigger with speed multiplier

### Content Assets
- 1 playable level (TestSlope)
- 5 feel-tuning profiles (Base, QuickerTurnIn, MoreGrip, LessSpeedLoss, CameraLead)
- Skier character mesh + material
- Snow material (PBR textures), skydome material, snow spray particle material
- Stylized PBR Nature pack: 15 tree meshes, 16 rock meshes, foliage types, materials, textures
- 2 test mountain meshes

### Config
- Forward shading, no Lumen/Nanite
- iOS: Metal, 60fps lock, portrait + landscape, iOS 15+
- Android: Vulkan, SDK 28-34, full sensor rotation
- Enhanced Input System enabled
- Niagara, CommonUI, PCG plugins enabled

---

## What's Built But Not Connected

These systems are implemented and functional but not wired to anything that triggers them:

| System | What Exists | What's Missing |
|--------|-------------|----------------|
| **Near-miss detection** | `EScoreAction::NearMiss` in ScoreSubsystem (50 pts + 0.5x multiplier), `FRunStats.NearMissCount`, displayed in stats screens | No proximity overlap volumes on obstacle actors, no code calls `AddScore(NearMiss)` |
| **UPowderSurfaceProfile** | Data asset wrapping `FSurfaceProperties`, primary asset pattern | Movement component uses hardcoded `FSurfaceProperties::GetPreset()` instead; no profiles created in Content/ |
| **UPowderTrickRegistry** | Data asset with `FPowderTrickDefinition` array, helper to find by gesture | Trick component uses hardcoded trick array; no registry assets created |
| **UPowderWeatherProfile** | Data asset wrapping `FWeatherConfig` | Weather manager uses `FWeatherConfig` directly with preset functions; no profile assets created |
| **IPowderAnimInterface** | Interface with `PlayTrickAnimation()`, `PlayLandAnimation()`, `SetCarveLean()`, `SetSpeedFactor()` | Character uses static mesh; interface ready for future skeletal mesh characters |
| **Gate scoring** | `EScoreAction::GatePass` in ScoreSubsystem (25 pts + 0.2x) | No gate obstacle actor exists |
| **Tree thread scoring** | Designed in game-plan.md (near-miss both sides, 100 pts + 1.0x) | No implementation; depends on near-miss detection |
| **Clean section scoring** | Designed in game-plan.md (200 pts for crashless zone) | No zone tracking or clean-section detection |
| **Distance scoring** | `ScoreSubsystem::AddDistance()` accumulates distance | Nothing calls it during gameplay |
| **Air time bonus** | `ScoreSubsystem::TrackAirTime()` with 0.2x multiplier bonus | Scoring infrastructure ready; not triggered from movement component |

---

## What's Planned But Not Started

### Phase 4 — Avalanche Polish & Run Phases (game-plan.md)

**Avalanche visuals & audio:**
- Avalanche actor with Niagara cloud wall (rolling white particle system with debris)
- Thunderous roar sound, cracking ice SFX
- Screen white-out engulf sequence when caught
- Rubberband speed (avalanche speeds up if player is very fast; never slows)

**Warning phase (~1500m):**
- Haptic rumble pulse
- Distant cracking sounds, snow falling from trees
- Subtle camera shake
- Ominous music drone

**Avalanche survival scoring:**
- 10 points/second during chase + 0.1x multiplier per 5 seconds survived
- 2x multiplier on all actions during chase phase

**Run phase system:**
- Track current phase: Meditation (0-800m) / Escalation (800-1500m) / Warning (1500-1800m) / Chase (1800m+)
- Phase drives zone weighting, obstacle density, music intensity, weather progression

### Phase 5 — Art & Audio

**Art finalization:**
- Character models (15-20 unlockable: Penguin, Yeti, Retro Skier, Robot, Astronaut, etc.)
- Skeletal mesh characters with animations (wire up IPowderAnimInterface)
- Ski trail ribbon renderer (paired lines fading to transparent)
- Polish terrain meshes and material work

**Audio system (comprehensive, nothing exists yet):**
- Sound manager component
- Adaptive music: three-act arc (serene pads -> subtle percussion -> frantic chase)
- Carve swoosh (surface-aware: powder vs ice)
- Speed wind (pitch scales with velocity)
- Coin collect chime
- Near-miss whoosh
- Boost activation burst
- Wipeout crunch + recovery
- Gate pass ding
- Trick spin whoosh
- Avalanche roar + proximity rumble
- UI sounds (menu taps, purchases)

**Haptic feedback:**
- Light vibration on carve
- Pulse on near-miss
- Heavy thud on landing
- Rumble on wipeout
- Warning tremor at ~1500m
- Avalanche proximity rumble

### Phase 6 — Meta & Monetization

- **Currency system**: coins (earned) + gems (premium), shop UI
- **Character unlock progression**: 15-20 characters, each skier or snowboarder type
- **Equipment**: boards/skis with stat differences (Speed, Carve, Trick Spin, Stability)
- **Cosmetic trails**: snow trail visual effects
- **Daily challenges**: 3/day ("Score 5000", "Near-miss 10 trees", "Land a 720", "Survive 30s in avalanche")
- **Season pass**: free + premium track with cosmetic rewards
- **IAP**: remove ads ($2.99), gem packs, season pass
- **Ad integration**: rewarded ads for continue/double coins
- **Cloud save**: cross-device progression
- **Leaderboards**: total score ranking

### Phase 7 — Launch

- App Store / Play Store listings
- Soft launch (TestFlight / limited geo)
- Analytics integration
- Balancing pass
- Full launch

---

## Zone System (Designed, Not Implemented)

The game-plan describes 8 zone types weighted by run phase. None have dedicated generation logic yet. Current levels are fully hand-authored.

| Zone | Description | Obstacle Types | Status |
|------|-------------|---------------|--------|
| PowderBowl | Wide open, forgiving, collectibles | Scattered coins | Trees/rocks placeable manually |
| TreeSlalom | Dense evergreens, near-miss opportunities | Trees at varying density | Trees exist, density not phase-driven |
| GateSlalom | Open slope with gate pairs | Gate actors | **No gate actor exists** |
| IceSheet | Low friction, harder control | Fewer obstacles, different surface | Surface system ready, no ice zones authored |
| MogulField | Bumpy terrain, automatic small jumps | Mogul mesh deformations | **Not implemented** |
| JumpPark | Ramps for tricks, big air | Jump ramps | Jump actor exists |
| CliffRun | Narrow path with drop-offs | Cliff edges, gaps | **Not implemented** |
| MixedRun | Everything combined | All types | Depends on all above |

---

## Architecture Ready for Expansion

These patterns are in place and designed to scale:

- **Tuning profiles**: `ApplyTuningProfile()` smoothly blends between any two `FMovementTuning` + `FCameraTuning` sets — can drive per-character feel, equipment stats, or zone-specific physics
- **Surface blending**: movement component smoothly transitions between `FSurfaceProperties` — ready for ice zones, mogul fields, etc.
- **Weather volumes**: layered box triggers with priority and blend distance — any number of weather zones per level
- **Data asset pattern**: TuningProfile, SurfaceProfile, WeatherProfile, TrickRegistry all follow UE5 primary asset conventions — just need content created and wired
- **Score action system**: adding new score sources is one enum value + one `AddScore()` call
- **IPowderAnimInterface**: skeletal mesh characters can implement this to get trick anims, carve lean, and speed factor without changing any gameplay code
- **Course spline workflow**: author spline in editor, snap to terrain, auto-place finish line and boundary trees — extending to gates/coins along path is straightforward

---

## Priority Recommendations

**High impact, low effort (do next):**
1. Near-miss detection — add thin overlap spheres to tree/rock actors, call `AddScore(NearMiss)` on overlap. Scoring infrastructure already handles everything else.
2. Distance scoring — call `AddDistance()` from movement component tick. One line of code.
3. Air time bonus — fire `TrackAirTime()` from movement component on land. Also one line.
4. Coin collection sound — TODO already in code, just needs a sound asset.

**High impact, medium effort:**
5. Gate obstacle actor — simple: two poles + box trigger, `AddScore(GatePass)` on overlap.
6. Run phase tracking — enum + distance thresholds in avalanche component or game mode. Drives everything else (density, music, scoring multipliers).
7. Avalanche survival scoring — 10 pts/sec tick in avalanche component during chase phase.

**High effort, essential for launch:**
8. Audio system — nothing exists; this is the biggest gap in game feel.
9. Skeletal mesh characters + animations — needed for visual variety and progression hooks.
10. Meta/shop/unlock system — needed for F2P monetization.
