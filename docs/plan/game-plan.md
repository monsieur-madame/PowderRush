# PowderRush - Arcade Skiing/Snowboarding Game
## Lonely Mountains' Diorama Aesthetic x SkiFree's Arcade Gameplay

---

## Context

An addictive, arcade-style skiing/snowboarding mobile game fusing two reference games:
- **Lonely Mountains: Snow Riders** — low-poly diorama art style, serene mountain atmosphere, pulled-back cinematic camera
- **SkiFree** — endless downhill descent, obstacle avoidance, trick scoring, an escalating chase threat, "one more run" compulsion

The core fantasy: **the satisfying feeling of carving through fresh powder at speed — serene and meditative until the mountain turns on you.**

Target: iOS/Android, Free-to-Play with IAP, solo developer using latest UE5.

---

## 1. Game Design

### Camera & Perspective — Three-Quarter Diorama View

A pulled-back three-quarter angle inspired by Lonely Mountains. The character feels small against the mountain — a tiny figure carving lines in an enormous snow-covered diorama.

- **Angle**: ~45 deg pitch, ~30 deg yaw offset from directly behind
- **Spring arm length**: 800-1200 (character feels small against the mountain)
- **Speed response**: Lower pitch slightly (45 deg -> 35 deg) to reveal more terrain ahead; subtle FOV widen
- **Carve response**: Yaw tracks carve direction to preview the player's path; no roll tilt
- **Zone transitions**: Brief pull-out to show new terrain arriving, then ease back in
- **Portrait orientation**: Tall slice of mountain with character in upper-middle third, upcoming terrain visible below
- **Mobile readability**: Character is larger than in Lonely Mountains (readability on small screen) but smaller than a typical runner. The angle gives the diorama feel while keeping terrain readable

### Core Controls (One-Thumb)
- **Touch left side of screen**: Carve left (hold duration = carve depth)
- **Touch right side of screen**: Carve right
- **No touch**: Ride straight, accelerate with gravity
- **Swipe while airborne**: Tricks (see Trick System section)
- Deep carves spray snow, build **Boost Meter**, but bleed speed
- Releasing a carve with a full meter = **Boost burst** forward

### The Core Loop
```
Start Run -> Carve through terrain (Meditation) ->
Obstacles tighten (Escalation) -> Hit jumps, chain tricks ->
Mountain rumbles (Warning) -> AVALANCHE CHASE ->
Survive as long as possible -> Engulfed ->
Score screen -> Earn coins -> Unlock stuff -> Start Run
```

Every run ends the same way — the avalanche catches you. The question is how far you get, not whether you crash.

### Terrain "Zone" System — Act-Based Pacing

Instead of purely random zone escalation, runs follow a structured three-act arc that naturally flows through all gameplay experiences.

**The run structure:**

| Distance | Act | Zone Bias | Experience | Mood |
|----------|-----|-----------|------------|------|
| 0-800m | Meditation | PowderBowl, gentle TreeSlalom | Open terrain, scattered coins, learn to carve | Serene |
| 800-1500m | Escalation | GateSlalom, dense TreeSlalom, JumpPark | Gates appear, trees tighten, ramps for tricks | Focused |
| 1500-1800m | Warning | MogulField, CliffRun, MixedRun | Terrain gets aggressive, rumbling begins | Tense |
| 1800m+ | The Avalanche | All types, high density | Avalanche chases, pure survival | Frantic |

**Zone types:**
- **PowderBowl** — Wide open, forgiving, collectibles (warm-up)
- **TreeSlalom** — Dense evergreens, near-miss opportunities (SkiFree tree slalom)
- **GateSlalom** — Open slope with gate pairs to thread (SkiFree slalom)
- **IceSheet** — Low friction, harder to control, different carve feel
- **MogulField** — Bumpy terrain, automatic small jumps
- **JumpPark** — Ramps for tricks, big air (SkiFree freestyle)
- **CliffRun** — Narrow path with drop-offs, gaps to clear
- **MixedRun** — Everything combined (late-game chaos)

Pre-designed zones (~200m each) stitched together. The `ATerrainManager` zone selection shifts from pure difficulty weighting to act-based weighting driven by distance thresholds.

### The Avalanche — Chase Mechanic

The signature run-ending mechanic. Replaces instant death on wipeout. The mountain itself rejects you.

**1. Warning phase (~1500m):**
- Ground rumbles (haptic pulse). Distant cracking sounds
- Snow falls from trees. Camera shakes subtly
- Music gains an ominous low drone

**2. Avalanche triggers (~1800m):**
- A massive wall of snow appears at the top of the screen (visible from three-quarter camera)
- Particle system: dense rolling cloud of white with debris chunks
- Thunderous roar sound

**3. The chase:**
- The avalanche has a base speed slightly below the player's comfortable cruising speed
- **Carving bleeds speed but the avalanche never slows down** — core tension: you must carve to dodge obstacles, but carving lets the avalanche gain on you
- Boost becomes critical — it's your escape valve
- A simple distance meter on HUD shows how close the avalanche is
- Rubberband: avalanche speeds up if player goes very fast (only upward, never slows artificially). Player can gain distance through skill but never fully escape
- Every second survived = bonus score

**4. Pre-avalanche wipeouts:**
- Brief stun + auto-recovery. You lose combo multiplier and a moment of momentum, but the run continues
- Encourages aggressive play — crashing isn't fatal until the avalanche is on you

**5. Run ends:**
- The avalanche catches you. Screen whites out as the snow engulfs the character
- Transition to score screen. Every run ends this way

**Why avalanche over Yeti:** Fits the natural mountain aesthetic. It IS the mountain — "the mountain rejecting you." Visually spectacular as a rolling snow wall visible from the diorama camera. Creates the same SkiFree panic while honoring Lonely Mountains' tone.

### Trick System — Touch-Friendly Aerials

Swipe-based controls while airborne:

- **Enter air**: Movement component detects player left a ramp surface
- **Swipe up while airborne**: Backflip (100 pts)
- **Swipe down**: Front flip (100 pts)
- **Swipe left/right**: Helicopter spin (150 pts)
- **Hold both screen sides**: Spread eagle (200 pts, can't steer landing — risky)
- **No input**: Safe air (0 pts, guaranteed clean landing)
- **Chain tricks**: Multiple swipes in one jump, each additional trick x1.5 multiplier
- **Landing**: Complete trick animation before landing = points. Incomplete = wipeout (stun + recovery)
- **Visual**: Character model rotates/spins with ghost trail ribbon during tricks

### Scoring System

| Action | Points | Multiplier Effect |
|--------|--------|-------------------|
| Deep carve | 10 | - |
| Near-miss (tree/rock) | 50 | +0.5x |
| Gate pass | 25 | +0.2x |
| Trick landed | 100-500 | +1.0x |
| Boost burst | 30 | - |
| Tree thread (near-miss both sides) | 100 | +1.0x |
| Clean section (zone with no crashes) | 200 | - |
| Speed bonus (max speed 5+ sec) | 50 | - |
| **Avalanche survival** | **10/sec** | **+0.1x per 5sec** |
| Chain (no break >2s) | - | Multiplier holds |
| Wipeout | - | Multiplier resets |

**During avalanche chase**: All actions get an additional 2x multiplier (stacks with combo). The chase is extremely lucrative, rewarding skilled survival.

**Distance scoring**: 1 point per meter. Longer runs always score higher.

**Leaderboard metric**: Total score (combines distance + actions + survival).

### Ski vs Snowboard
- **Per-character**: Each character is inherently a skier OR snowboarder
- Different feel for each type (skiers = tighter carves, boarders = wider arcs + better tricks)
- Gives players a reason to collect both types and find their preference
- Starter character: snowboarder (broader appeal with younger audience)

### Progression & Meta-game
- **Characters**: 15-20 unlockable (Penguin, Yeti, Retro 80s Skier, Robot, Astronaut, etc.) - each is a skier or boarder
- **Boards/Skis**: Different stats (Speed, Carve, Trick Spin, Stability)
- **Trails**: Cosmetic snow trail effects
- **Daily Challenges**: 3 per day ("Score 5000 in one run", "Near-miss 10 trees", "Land a 720", "Survive 30s in avalanche")
- **Season Pass**: Free + premium track with cosmetic rewards

### Monetization (F2P + IAP)
- **Coins**: Earned in-game, buy common cosmetics
- **Gems**: Premium currency, buy rare cosmetics / season pass
- **No pay-to-win**: All stat-affecting gear earnable through gameplay
- **Optional rewarded ads**: Watch ad to double run coins, or continue after wipeout once
- **IAP tiers**: Remove ads ($2.99), Gem packs ($0.99-$19.99), Season Pass ($4.99)

---

## 2. Technical Architecture (UE5)

### Project Setup
- **Template**: Blank C++ project
- **Rendering**: Forward Shading (Mobile renderer)
- **No Nanite/Lumen** (not supported on mobile)
- **Target**: 60fps on mid-range phones (iPhone 12+, Galaxy S21+)
- **Vulkan** (Android) / **Metal** (iOS)

### Module Structure
```
Source/
  PowderRush/
    Core/           - Game mode, game state, game instance
    Player/         - Character, movement component, controller
    Terrain/        - Zone generation, tile system, obstacles
    Scoring/        - Score manager, combo system, near-miss detection
    Pickup/         - Coins, power-ups, collectibles
    Threat/         - Avalanche actor and chase system
    UI/             - HUD, menus, shop, daily challenges
    Meta/           - Progression, save/load, currency, unlocks
    Effects/        - Snow trails, particles, camera effects
    Audio/          - Sound manager, adaptive music
```

### Key Systems (C++ Base, Blueprint Extension)

#### A. Custom Movement Component (`UPowderMovementComponent`)
- Custom physics for skiing feel (NOT standard UCharacterMovement)
- Gravity-based acceleration on slopes
- Carve mechanics: lateral drag, speed bleed, boost accumulation
- Surface detection: powder vs ice vs moguls vs rock (different friction/response)
- Spline-based terrain following for smooth movement
- Airborne state: ramp launch detection, trick state machine

#### B. Terrain Generation System (`ATerrainManager`)
- **Zone tiles**: Pre-built level chunks (~200m each) as Level Instances
- **Tile pool**: Load 3 zones ahead, unload 2 behind (memory management)
- **Obstacle spawner**: Randomized tree/rock/gate/ramp placement within zones
- **Act-based zone weighting**: `SelectNextZoneType()` driven by distance thresholds — Meditation zones early, MixedRun during avalanche
- **Collectible placer**: Coin lines, power-up spawns based on zone type

#### C. Scoring & Combo System (`UScoreSubsystem`)
- Game Instance Subsystem for persistence across levels
- Near-miss detection via overlap volumes on obstacles (thin outer shell)
- Combo timer with visual feedback
- Multiplier stacking with cap (10x max, 20x during avalanche)
- Score actions: DeepCarve, NearMiss, GatePass, TrickLanded, BoostBurst, TreeThread, CleanSection, SpeedBonus, AvalancheSurvival
- Distance scoring: 1 point per meter
- End-of-run breakdown screen

#### D. Camera System (`APowderCameraManager`)
- Three-quarter diorama angle (45 deg pitch, 30 deg yaw offset, 800-1200 arm length)
- Speed response: pitch lowers to reveal terrain, subtle FOV widen
- Carve response: yaw tracks carve direction to preview player's path
- Zone transition pull-out
- Near-miss, landing, and wipeout shake
- Smooth interpolation on all axes

#### E. Avalanche System (`APowderAvalanche` + `UAvalancheComponent`)
- `APowderAvalanche`: Avalanche actor — particle wall (Niagara), collision volume, thunderous roar sound
- `UAvalancheComponent`: Chase speed logic, rubberband algorithm, distance tracking, warning phase triggers
- Spawned by `APowderGameMode` at distance threshold (~1800m)
- Warning phase begins at ~1500m (haptics, camera shake, audio cues)

#### F. Save/Progression System
- `USaveGame` subclass for local save
- Cloud save via platform (Game Center / Google Play Games)
- Tracks: coins, gems, unlocked items, daily challenge progress, stats

### Input System
- **Enhanced Input** with touch-specific Input Mapping Context
- Two virtual zones (left half / right half of screen)
- Hold duration drives carve intensity
- Swipe detection for tricks while airborne (up/down/left/right)

### New Classes

| Class | Module | Purpose |
|-------|--------|---------|
| `APowderAvalanche` | `Source/PowderRush/Threat/` | Avalanche actor — particle wall, collision, sound |
| `UAvalancheComponent` | `Source/PowderRush/Threat/` | Chase speed logic, rubberband, distance tracking |
| `USkiTrailComponent` | `Source/PowderRush/Effects/` | Ribbon mesh rendering player's path in snow |
| `ARampObstacle` | `Source/PowderRush/Terrain/` | Ramp subclass of `AObstacleBase` with launch velocity |
| `AGateObstacle` | `Source/PowderRush/Terrain/` | Gate pair subclass of `AObstacleBase` for slalom |

### Existing Classes Modified

| Class | Change |
|-------|--------|
| `APowderCharacter` | Reconfigure spring arm for three-quarter camera (pitch, yaw, arm length, lag) |
| `APowderPlayerController` | Add swipe gesture detection for tricks while airborne |
| `APowderGameMode` | Add run phases (Meditation/Escalation/Warning/Chase), distance-based act system, avalanche spawn trigger |
| `UPowderMovementComponent` | Add airborne/trick state, ramp launch detection |
| `PowderTypes.h` | Add `GateSlalom`/`MixedRun` to `EZoneType`, `Rock` to `ESurfaceType`, new trick/phase enums |
| `UScoreSubsystem` | New score actions (AvalancheSurvival, TreeThread, CleanSection, SpeedBonus), avalanche multiplier, distance scoring |
| `ATerrainManager` | Act-based zone weighting in `SelectNextZoneType()` driven by distance thresholds |

---

## 3. Art Direction — Low-Poly Diorama Aesthetic

Primary visual reference: **Lonely Mountains: Snow Riders**. The mountain should look carved from wood then dusted with snow.

### Core Principles
- **Terrain**: Faceted low-poly meshes with flat shading. Sharp geometric edges
- **Snow**: Near-pure white, subtle blue in shadows. Flat white + vertex-painted blue in concavities (cheaper than procedural noise on mobile)
- **Color palette**: Extremely limited per zone. Base: white/gray/dark green. Accents come from sky (warm sunset oranges late-game, cold blues on ice). Player character is the most saturated element on screen
- **Trees**: Geometric cones and cylinders. Dark green (near black-green) for silhouette readability against white snow. No foliage transparency (saves draw calls)
- **Rocks**: Angular, faceted boulders. Gray-brown. 3-4 scale variants
- **Lighting**: Single directional light, baked. Long shadows from trees/rocks provide gameplay-readable terrain info. Fixed shadow direction so players learn to "read" terrain
- **Ski tracks**: Ribbon mesh drawing the player's path behind them — thin paired lines fading to transparent over distance. Most evocative visual from Lonely Mountains
- **Atmosphere**: Gentle ambient snow particles. Light distance fog to hide zone streaming seams. Serene and timeless until the avalanche hits

### 3D Models - Creation Pipeline
| Asset | Approach | Tool |
|-------|----------|------|
| **Terrain tiles** | Hand-model base shapes, then instance | Blender (free) |
| **Trees (10 variants)** | Low-poly conifers, geometric cones/cylinders | Blender |
| **Rocks (3-4 variants)** | Angular faceted boulders, multiple scale variants | Blender Geometry Nodes |
| **Characters (15-20)** | Simple mesh + unique silhouette per character | Blender, AI concept art for reference |
| **Boards/Skis** | Simple flat meshes with texture variation | Blender |
| **Gates/Flags** | Basic geometric shapes | UE5 BSP or Blender |
| **Ramps** | Simple geometric wedges with snow cover | Blender |
| **Coins/Pickups** | Basic shapes with emissive material | UE5 directly |

### Particle Effects (Niagara)
- **Snow spray on carve**: GPU particles, velocity-driven emission
- **Powder cloud on wipeout**: Burst emitter
- **Ski trail in snow**: Ribbon renderer following skis (paired thin lines, `USkiTrailComponent`)
- **Collectible sparkle**: Simple sprite particles
- **Speed lines**: Screen-space effect at high speed
- **Snowfall**: Ambient weather particles (light, not distracting)
- **Avalanche cloud**: Dense rolling white cloud with debris chunks (large-scale Niagara system)

### Materials
- **Snow surface**: Flat white + vertex-painted blue in concavities
- **Ice**: High specularity, slight blue tint, different snow spray
- **Terrain base**: Vertex-painted blend between snow/rock/ice
- **Character materials**: Flat color with slight rim lighting for readability (most saturated element on screen)
- **Material budget**: 10-15 unique materials total. Use material instances with parameter variation

---

## 4. Audio Design — "Serene Until It Isn't"

A three-act emotional arc matching the run structure. The contrast IS the game — peaceful early game makes the avalanche terrifying. The avalanche makes you savor the next run's peaceful opening.

### Act 1 — Meditation (0-1200m)
- Ambient pads, wind sounds, distant bird calls. Minimal music
- Warm golden light, long peaceful shadows, pristine snow
- Flowing rhythmic gameplay. "Just you and the mountain"
- **SFX focus**: Carve swoosh, gentle wind, coin chimes

### Act 2 — Escalation (1200-1800m)
- Subtle percussion fades in. Wind picks up. Occasional distant rumble/crack
- Light shifts cooler (blue-white). Shadows lengthen. Steeper terrain
- Tighter lines, more obstacles, gates appear. Mountain is testing you
- **SFX focus**: Ice scrapes, near-miss whooshes, trick wind

### Act 3 — The Chase (1800m+)
- Thunderous avalanche roar, cracking ice, rush of displaced air
- Camera subtly closer, snow particles intensify, screen edges vignette
- Pure survival. Every carve is a calculated risk
- The serene layers are still underneath — buried but present
- **SFX focus**: Avalanche rumble, frantic carve sounds, desperate boost blasts

### SFX Priority List
1. Carve sound (changes with surface type - powder swoosh vs ice scrape)
2. Speed wind (pitch increases with velocity)
3. Coin collect (satisfying chime)
4. Near-miss whoosh
5. Boost activation
6. Wipeout crunch + stun recovery
7. Avalanche roar and rumble
8. Gate pass ding
9. Trick spin whoosh
10. UI sounds (menu taps, purchase confirmations)

### Haptic Feedback (crucial for mobile)
- Light vibration on carve
- Pulse on near-miss
- Heavy thud on landing
- Rumble pattern on wipeout
- **Warning phase rumble** (~1500m): Increasing intensity ground tremor
- **Avalanche proximity**: Constant low rumble, intensifying as distance closes

---

## 5. Development Phases

### Phase 1: Core Feel (Weeks 1-3)
- [ ] Set up UE5 project (blank C++, mobile settings, forward renderer)
- [ ] Build a single straight slope (static mesh, no generation)
- [ ] Implement custom movement component with gravity-based acceleration
- [ ] Implement carve mechanic (touch left/right, speed bleed, snow spray)
- [ ] Implement boost meter (fills on carve, burst on release)
- [ ] Implement three-quarter diorama camera (45 deg pitch, 30 deg yaw, 800-1200 arm)
- [ ] Camera speed response (pitch shift, FOV widen) and carve response (yaw tracking)
- [ ] Test on device (iOS) - feel must be right before proceeding

### Phase 2: The Mountain (Weeks 4-6)
- [ ] Build 4 zone tiles in diorama style (PowderBowl, TreeSlalom, JumpPark, IceSheet)
- [ ] Zone stitching system (load ahead, unload behind)
- [ ] Act-based zone weighting in `ATerrainManager` driven by distance thresholds
- [ ] Obstacle collision and wipeout-as-stun (brief stun + auto-recovery)
- [ ] `AGateObstacle` — gate pair obstacle for slalom zones
- [ ] Gate pass detection (overlap volumes)
- [ ] `USkiTrailComponent` — ribbon mesh ski trail rendering (paired lines, fade over distance)
- [ ] Basic coin collectibles along the path

### Phase 3: Tricks & Scoring (Weeks 7-9)
- [ ] Airborne state in `UPowderMovementComponent` (ramp launch detection)
- [ ] `ARampObstacle` — ramp subclass with configurable launch velocity
- [ ] Swipe gesture detection in `APowderPlayerController` (up/down/left/right while airborne)
- [ ] Trick state machine (start trick, animate, land/wipeout)
- [ ] Full scoring integration in `UScoreSubsystem` (all actions from scoring table)
- [ ] Near-miss detection, tree thread detection
- [ ] Combo timer and multiplier system
- [ ] HUD: speed, score, combo multiplier, boost meter, distance
- [ ] Score screen with full breakdown
- [ ] Instant restart (zero load time)

### Phase 4: The Avalanche (Weeks 10-12)
- [ ] `APowderAvalanche` actor with Niagara particle wall (rolling snow cloud)
- [ ] `UAvalancheComponent` with chase speed logic and rubberband algorithm
- [ ] Warning phase (~1500m): haptic rumble, camera shake, audio cues, snow falling from trees
- [ ] Avalanche spawn trigger in `APowderGameMode` at ~1800m
- [ ] Avalanche proximity HUD meter
- [ ] Run-end: screen white-out engulf sequence
- [ ] Survival scoring (10/sec + 0.1x per 5sec + 2x multiplier on all actions)
- [ ] Run phase system in `APowderGameMode` (Meditation/Escalation/Warning/Chase)

### Phase 5: Art & Atmosphere (Weeks 13-15)
- [ ] Finalize diorama art style (Lonely Mountains reference sheet, color palette)
- [ ] Model terrain assets: faceted low-poly trees (geometric cones/cylinders), angular rocks
- [ ] Baked lighting pass: single directional light, long shadows, fixed direction
- [ ] Snow material: flat white + vertex-painted blue in concavities
- [ ] Three-act sound/music system (Meditation ambient -> Escalation percussion -> Chase roar)
- [ ] Niagara snow effects: carve spray, wipeout cloud, ambient snowfall, avalanche cloud
- [ ] Ski trail polish (paired thin lines, fade to transparent)
- [ ] Distance fog for zone streaming seams
- [ ] 3-5 characters with unique silhouettes
- [ ] 3-5 boards/skis
- [ ] Haptic feedback pass (carve, near-miss, landing, wipeout, warning rumble, avalanche proximity)
- [ ] UI art pass (menus, HUD, buttons)

### Phase 6: Meta & Polish (Weeks 16-18)
- [ ] Currency system (coins + gems)
- [ ] Shop UI (characters, boards, trails)
- [ ] Unlock/progression system
- [ ] Daily challenges (3 per day, rotating pool, includes avalanche survival challenges)
- [ ] Stats tracking (best score, total distance, total tricks, longest avalanche survival)
- [ ] Rewarded ads integration
- [ ] IAP integration (platform-specific)
- [ ] Remove-ads IAP
- [ ] Save/load system with cloud sync
- [ ] Build remaining zone tiles (GateSlalom, MogulField, CliffRun, MixedRun)
- [ ] Remaining characters (15-20 total)
- [ ] Power-ups (Magnet, Shield, 2x Coins, Rocket Boost)
- [ ] Tutorial / first-time user experience
- [ ] Difficulty balancing pass
- [ ] Performance optimization (draw calls, materials, memory)
- [ ] Bug fixing pass
- [ ] Analytics integration

### Phase 7: Launch (Weeks 19-20)
- [ ] App Store / Google Play store listing
- [ ] App icons and splash screens
- [ ] Privacy policy, terms of service
- [ ] Soft launch in limited markets
- [ ] Iterate on retention/monetization based on soft launch data
- [ ] Full launch

---

## 6. Key Technical Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Language | C++ core + Blueprints gameplay | Performance for physics/movement, fast iteration for gameplay |
| Renderer | Forward Shading (Mobile) | Required for mobile, good performance |
| Lighting | Baked (Lightmass) | Static zones allow baked lighting, best mobile perf |
| Particles | Niagara (GPU) | Snow effects + avalanche cloud, good mobile GPU support |
| Input | Enhanced Input System | Modern UE5 standard, clean touch handling |
| Audio | MetaSounds | Procedural audio for adaptive music/SFX |
| Save | USaveGame + Cloud | Simple, reliable, platform cloud backup |
| Terrain | Level Instance streaming | Pre-built zones, streamed in/out for memory |
| UI | UMG (Common UI plugin) | Mobile-friendly widget system |
| Camera | Three-quarter diorama | Lonely Mountains feel, terrain readability, mobile-friendly |
| Run end | Avalanche chase | SkiFree tension, natural to setting, visually spectacular |
| Shading | Flat/faceted low-poly | Diorama aesthetic, low draw call count, readable on mobile |
