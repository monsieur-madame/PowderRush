# PowderRush - Arcade Skiing/Snowboarding Game
## Full Development Plan (UE5 Latest, Mobile, Solo Dev, F2P)

---

## Context

Build an addictive, arcade-style skiing/snowboarding mobile game designed for 45-90 second play sessions. Based on "Concept 1: Flow State Drifter" with the best elements borrowed from the other two concepts. The core fantasy: **the satisfying feeling of carving through fresh powder at speed**.

Target: iOS/Android, Free-to-Play with IAP, solo developer using latest UE5.

---

## 1. Game Design - Refined Concept

### Camera & Perspective
- **Third-person, behind-the-back camera** (not top-down) - gives visceral speed feeling
- Camera dynamically pulls back at high speed, pushes in during tight carves
- Slight camera tilt into turns for juiciness

### Core Controls (One-Thumb)
- **Touch left side of screen**: Carve left (hold duration = carve depth)
- **Touch right side of screen**: Carve right
- **No touch**: Ride straight, accelerate with gravity
- **Swipe up on ramps**: Jump / trick (simple swipe direction = trick type)
- Deep carves spray snow, build **Boost Meter**, but bleed speed
- Releasing a carve with a full meter = **Boost burst** forward

### The Core Loop (60-90 seconds)
```
Start Run -> Carve through terrain -> Build combos -> Hit jumps ->
Score multipliers -> Wipeout or finish -> Score screen ->
Earn coins -> Unlock stuff -> Start Run
```

### Terrain "Zone" System
Instead of pure endless runner, use **pre-designed "zones" (~10-15 seconds each)** stitched together in random order:
- **Powder Bowl**: Wide open, forgiving, lots of collectibles - warm-up
- **Tree Slalom**: Tight trees, near-miss opportunities, high risk/reward
- **Ice Sheet**: Low friction, harder to control, different carve feel
- **Mogul Field**: Bumpy terrain, automatic small jumps
- **Cliff Run**: Narrow path with drop-offs, gaps to clear
- **Jump Park**: Ramps for tricks, big air moments
- Difficulty escalates by biasing toward harder zones as the run progresses

### Scoring System
| Action | Points | Multiplier Effect |
|--------|--------|-------------------|
| Deep carve | 10 | - |
| Near-miss (tree/rock) | 50 | +0.5x |
| Gate pass | 25 | +0.2x |
| Trick landed | 100-500 | +1.0x |
| Boost burst | 30 | - |
| Chain (no break >2s) | - | Multiplier holds |
| Wipeout | - | Multiplier resets |

### Ski vs Snowboard
- **Per-character**: Each character is inherently a skier OR snowboarder
- Different feel for each type (skiers = tighter carves, boarders = wider arcs + better tricks)
- Gives players a reason to collect both types and find their preference
- Starter character: snowboarder (broader appeal with younger audience)

### Progression & Meta-game
- **Characters**: 15-20 unlockable (Penguin, Yeti, Retro 80s Skier, Robot, Astronaut, etc.) - each is a skier or boarder
- **Boards/Skis**: Different stats (Speed, Carve, Trick Spin, Stability)
- **Trails**: Cosmetic snow trail effects
- **Daily Challenges**: 3 per day ("Score 5000 in one run", "Near-miss 10 trees", "Land a 720")
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
- **Template**: Blank C++ project (not Third Person template - too much baggage)
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
- Surface detection: powder vs ice vs moguls (different friction/response)
- Spline-based terrain following for smooth movement

#### B. Terrain Generation System (`ATerrainManager`)
- **Zone tiles**: Pre-built level chunks (~200m each) as Level Instances
- **Tile pool**: Load 3 zones ahead, unload 2 behind (memory management)
- **Obstacle spawner**: Randomized tree/rock/gate placement within zones
- **Difficulty curve**: Weight table shifts toward harder zones over time
- **Collectible placer**: Coin lines, power-up spawns based on zone type

#### C. Scoring & Combo System (`UScoreSubsystem`)
- Game Instance Subsystem for persistence across levels
- Near-miss detection via overlap volumes on obstacles (thin outer shell)
- Combo timer with visual feedback
- Multiplier stacking with cap (10x max)
- End-of-run breakdown screen

#### D. Camera System (`APowderCameraManager`)
- Dynamic FOV: widens with speed
- Offset: pulls back at high speed, pushes in during carves
- Roll: slight tilt into turns
- Shake: on near-misses, landings, wipeouts
- Smooth interpolation on all axes

#### E. Save/Progression System
- `USaveGame` subclass for local save
- Cloud save via platform (Game Center / Google Play Games)
- Tracks: coins, gems, unlocked items, daily challenge progress, stats

### Input System
- **Enhanced Input** with touch-specific Input Mapping Context
- Two virtual zones (left half / right half of screen)
- Hold duration drives carve intensity
- Swipe detection for tricks on ramps

---

## 3. Asset Strategy (Solo Dev)

### Art Style Definition
- **Low-poly, geometric** with flat shading and bold colors
- Inspired by: Alto's Adventure (clean), Monument Valley (geometric), Crossy Road (charming)
- Limited color palette per zone (e.g., powder = white/blue/teal, sunset = orange/pink/purple)
- Strong silhouettes for readability at speed

### 3D Models - Creation Pipeline
| Asset | Approach | Tool |
|-------|----------|------|
| **Terrain tiles** | Hand-model base shapes, then instance | Blender (free) |
| **Trees (10 variants)** | Low-poly conifers, procedural placement | Blender + UE5 Foliage |
| **Rocks (8 variants)** | Geo-nodes in Blender for procedural rocks | Blender Geometry Nodes |
| **Characters (15-20)** | Simple mesh + unique silhouette per character | Blender, AI concept art for reference |
| **Boards/Skis** | Simple flat meshes with texture variation | Blender |
| **Gates/Flags** | Basic geometric shapes | UE5 BSP or Blender |
| **Buildings/Lodges** | Simple structures for background scenery | Blender |
| **Coins/Pickups** | Basic shapes with emissive material | UE5 directly |

### Particle Effects (Niagara)
- **Snow spray on carve**: GPU particles, velocity-driven emission
- **Powder cloud on wipeout**: Burst emitter
- **Trail in snow**: Ribbon renderer following skis
- **Collectible sparkle**: Simple sprite particles
- **Speed lines**: Screen-space effect at high speed
- **Snowfall**: Ambient weather particles (light, not distracting)

### Materials (Procedural in UE5)
- **Snow surface**: Procedural noise-based, with sparkle (fake subsurface)
- **Ice**: High specularity, slight blue tint, different snow spray
- **Terrain base**: Vertex-painted blend between snow/rock/ice
- **Character materials**: Flat color with slight rim lighting for readability

---

## 4. Audio Design

- **Adaptive music**: Layers that intensify with speed/combo multiplier
  - Base layer: Chill ambient (low speed)
  - Layer 2: Rhythmic percussion (medium speed)
  - Layer 3: Full energy track (boost/high combo)
- **SFX priority list**:
  1. Carve sound (changes with surface type - powder swoosh vs ice scrape)
  2. Speed wind (pitch increases with velocity)
  3. Coin collect (satisfying chime)
  4. Near-miss whoosh
  5. Boost activation
  6. Wipeout crunch + ragdoll tumble
  7. UI sounds (menu taps, purchase confirmations)
- **Haptic feedback** (crucial for mobile):
  - Light vibration on carve
  - Pulse on near-miss
  - Heavy thud on landing
  - Rumble pattern on wipeout

---

## 5. Development Phases

### Phase 1: Prototype Core Feel (Weeks 1-3)
- [ ] Set up UE5 project (blank C++, mobile settings, forward renderer)
- [ ] Build a single straight slope (static mesh, no generation)
- [ ] Implement custom movement component with gravity-based acceleration
- [ ] Implement carve mechanic (touch left/right, speed bleed, snow spray)
- [ ] Implement boost meter (fills on carve, burst on release)
- [ ] Basic third-person camera with speed-based FOV
- [ ] Test on device (iOS or Android) - feel must be right before proceeding

### Phase 2: Core Gameplay Loop (Weeks 4-6)
- [ ] Build 3-4 zone tiles (Powder Bowl, Tree Slalom, Jump Park, Ice Sheet)
- [ ] Zone stitching system (load ahead, unload behind)
- [ ] Obstacle collision and wipeout (ragdoll physics)
- [ ] Scoring system (base points, near-miss, combos)
- [ ] Basic coin collectibles along the path
- [ ] HUD: speed, score, combo multiplier, boost meter
- [ ] Score screen with breakdown
- [ ] Instant restart (zero load time)

### Phase 3: Game Feel Polish (Weeks 7-8)
- [ ] Camera system polish (tilt, pull-back, shake)
- [ ] Niagara snow spray on carve (different per surface)
- [ ] Snow trail behind skis/board
- [ ] Ragdoll wipeout physics (exaggerated, funny)
- [ ] Near-miss visual/audio feedback (slow-mo flash, whoosh sound)
- [ ] Screen effects (speed lines, vignette at high speed)
- [ ] Haptic feedback integration
- [ ] Sound design pass (carve, wind, coins, wipeout)
- [ ] Adaptive music system (basic 2-layer)

### Phase 4: Art & Visual Identity (Weeks 9-11)
- [ ] Finalize art style (color palette, reference sheet)
- [ ] Model/acquire terrain assets (trees, rocks, terrain tiles)
- [ ] Create 3-5 characters with unique silhouettes
- [ ] Design and build 3-5 boards/skis
- [ ] Procedural snow material
- [ ] Skybox / environment (mountains in background, clouds)
- [ ] Lighting pass (baked, warm/cool palette per zone type)
- [ ] UI art pass (menus, HUD, buttons - clean mobile style)

### Phase 5: Meta-game & Monetization (Weeks 12-14)
- [ ] Currency system (coins + gems)
- [ ] Shop UI (characters, boards, trails)
- [ ] Unlock/progression system
- [ ] Daily challenges (3 per day, rotating pool)
- [ ] Stats tracking (best score, total distance, total tricks)
- [ ] Rewarded ads integration
- [ ] IAP integration (platform-specific)
- [ ] Remove-ads IAP
- [ ] Save/load system with cloud sync

### Phase 6: Content & Polish (Weeks 15-17)
- [ ] Build remaining zone tiles (8-12 total for variety)
- [ ] Remaining characters (15-20 total)
- [ ] Power-ups (Magnet, Shield, 2x Coins, Rocket Boost)
- [ ] Tutorial / first-time user experience
- [ ] Difficulty balancing pass
- [ ] Performance optimization
- [ ] Memory optimization
- [ ] Bug fixing pass
- [ ] Analytics integration

### Phase 7: Launch Prep (Weeks 18-19)
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
| Particles | Niagara (GPU) | Beautiful snow effects, good mobile GPU support |
| Input | Enhanced Input System | Modern UE5 standard, clean touch handling |
| Audio | MetaSounds | Procedural audio for adaptive music/SFX |
| Save | USaveGame + Cloud | Simple, reliable, platform cloud backup |
| Terrain | Level Instance streaming | Pre-built zones, streamed in/out for memory |
| UI | UMG (Common UI plugin) | Mobile-friendly widget system |
