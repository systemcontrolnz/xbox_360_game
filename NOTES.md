# Xbox 360 Roguelike — Project Notes

Environment/toolchain setup details live in ENVIRONMENT_SETUP.md — this
file is about the game itself: design, workflow, and current status.

## What This Is

A 2D, turn-based, colored ASCII/glyph roguelike (NetHack/Rogue-style
rendering) for the Xbox 360, written in C, using libxenon, launched via
XeLL. Long-term "tinker over time" project, not a quick one-off. Explicit
long-term goal: port to Windows later (see Portability, below) — a real
goal, not just a stretch idea.

## Design (locked in)

1. **Visuals:** Colored ASCII/glyph grid (NetHack/Rogue/Dwarf Fortress
   style), NOT hand-drawn sprite tiles, as the current baseline. Colored
   characters on a monospace grid can be built entirely in code with zero
   art assets. Sprite rendering is now a planned upgrade, not just a
   maybe — see "Sprite Rendering — Planned Upgrade" section below for
   scope assessment. Static sprite images (flip-through frames per
   NPC/character, plus dungeon background images) are the intended art
   style once pursued; not photorealistic or complex animation.
2. **Core loop:** NetHack's rendering/turn structure, with Vampire
   Survivors' power-fantasy escalation feel instead of slow exploration.
   - Turn-based movement (world only advances when player moves)
   - Descending floors that escalate in difficulty/density, more like
     waves than slow maze exploration
   - Kill things -> XP -> level up -> tangibly stronger (constant forward
     momentum / snowballing power)
   - NOTE: the Vampire Survivors influence is scoped ONLY to this power-
     escalation feel. This is NOT a 3D game and NOT a Vampire-Survivors-
     style game in genre, camera, or dimensionality.
3. **Combat:** Bump-to-attack (walking into a monster = melee attack).
4. **Persistence:** Simple permadeath. Dying ends the run; next run is a
   fresh dungeon. No meta-progression in V1.
5. **Post-V1 idea (not started):** Metroidvania-style connected,
   backtrackable dungeon map (SotN/Castlevania inspiration), possibly
   with lock-and-key style ability/item gates. Full SotN-level visual
   presentation is not realistic (art/content problem) — the connected-
   map structure is the realistic part worth borrowing.
6. **Run seeding + randomized objectives (idea, not yet designed in
   detail):** Each run's procedural dungeon (Milestone 3 step 4) is
   generated from a seed — either random per run, or manually entered to
   reproduce a specific playthrough. Each run also picks one or more
   objectives at random from a predefined list (exact objective types TBD
   — candidates: reach the exit, kill N enemies, survive N floors,
   collect a specific item). This is a refinement of the existing
   escalating-difficulty design (see point 2 above), not a new
   direction — still 2D, turn-based, permadeath, same core loop. Natural
   fit after Milestone 3 step 5 (monsters/combat), since most plausible
   objectives depend on enemies existing first.
7. **Menu system (idea, not yet designed in detail):** Main menu, pause
   menu, and potentially a settings menu are anticipated as the game
   grows. Not expected to be a major technical hurdle — same
   `render_draw_tile()` / `input_poll()` primitives as gameplay, just a
   different screen state. The one real open question (console
   reboot/shutdown capability) is now resolved — see "System Power
   Control — SMC Findings" below. A pause menu's "Quit" option will
   trigger a console reboot (see that section for why this is an
   acceptable substitute for "return to Aurora").

## Code Workflow Rules (agreed, important)

- Split files by responsibility from the start: `main.c`, `render.c/h`,
  `input.c/h`, `entities.c/h`, `combat.c/h`, `dungeon.c/h`
- Short header comment per file describing purpose/public functions
- Prefer diffs/patches over full-file rewrites once files exist
- One feature/bug per turn — don't bundle unrelated changes
- **Platform abstraction:** ALL libxenon-specific calls confined to
  `render.c` and `input.c` only. Every other file calls generic functions
  (`draw_tile()`, `get_input()`) and never touches libxenon directly —
  this is what makes a future Windows/SDL2 port a matter of swapping
  render/input, not rewriting game logic. Resist any shortcut where a
  non-render/input file calls libxenon directly "just to get it working
  faster."
- Use git properly: commit after each reasonably working revision, short
  descriptive messages.
- Keep this NOTES.md current as work progresses.
- When researching a new libxenon subsystem (as done for render.c and
  input.c), confirm actual header location, function signatures, and
  struct fields directly from the local `libxenon/` clone via `grep`
  BEFORE writing implementation code — don't trust old wiki examples at
  face value, they can be outdated (see kmem_init gotcha below). Caveat
  learned this session: don't trust filenames alone either —
  `xenon_soc/xenon_power.c` sounds like it should hold shutdown/reboot
  logic but is actually all CPU clock-speed/threading code. The real
  power-control functions turned out to live in `xenon_smc/`. Grep
  contents, not just filenames, when a first guess doesn't pan out.

## Portability Goal (Windows)

Real goal, not a nice-to-have. Approach: platform abstraction layer
(see workflow rules above). Porting later means writing new
`render.c`/`input.c` using something like SDL2, while all game-logic
files carry over unchanged. Caveat: this makes the port itself small,
but the SDL2 replacement layer is still real (if small) work — not a
free recompile. The benefit is avoiding a painful refactor of game logic
later.

## Milestone Plan

- **Milestone 1 — Prove the toolchain pipeline end-to-end: COMPLETE.**
  Built libxenon via Docker, wrote a minimal hello-world, debugged a
  real crash (missing `xenos_init()` call — see ENVIRONMENT_SETUP.md),
  confirmed text rendering live on real hardware via XeLL. Git/GitHub
  set up and working (`systemcontrolnz/xbox_360_game`).

- **Milestone 2 — Input + basic render loop: COMPLETE.**
  Split into `render.c/h` and `input.c/h`, both confined to libxenon
  calls per the platform-abstraction rule; `main.c` now has zero
  libxenon includes. `render_draw_tile(x, y, glyph, fg, bg)` wraps
  libxenon's console API (cursor positioning + BGRA color packing).
  `input_poll()` reads the controller edge-triggered (press, not
  held-repeat) via `get_controller_data()`, matching turn-based movement.
  Confirmed on real hardware: colored glyphs render correctly, and the
  d-pad moves a single `@` around the grid one tile per press with no
  repeat-while-held or diagonal drift.

- **Milestone 3 — The actual roguelike (in progress).** Broken into
  ordered sub-steps rather than tackled all at once, per one-feature-per-
  turn workflow:
  1. **`dungeon.c/h` — static test map: COMPLETE.** Tile-type grid
     (floor, wall) with a fixed hand-authored test layout (not
     procedural yet). Renders via `render_draw_tile()` — walls and floor
     as distinct glyphs/colors. Confirmed multi-tile map draws correctly
     on hardware.
  2. **Collision (next up).** Wire player movement (currently free-
     roaming from Milestone 2) to check the dungeon grid before moving —
     walking into a wall tile should be blocked. This is the first point
     `main.c`'s loop needs to know about both `dungeon.c` and `input.c`
     together.
  3. **`entities.c/h`.** Formalize player state (currently just loose
     `x`/`y` ints in `main.c`) into a proper struct — position, HP, and
     room for stats to be added later (XP, level). Monsters will reuse
     this same struct shape.
  4. **Procedural generation.** Replace the static test map with actual
     room/corridor dungeon generation. Deliberately last in this list —
     easiest to validate generation logic once rendering+collision
     already work against a known-good static map. Will incorporate the
     seed + randomized objectives idea from Design point 6 above.
  5. **Monsters + simple AI, combat resolution, XP/leveling.** Deepen
     indefinitely per the design decisions above, once the floor itself
     is solid.

  Recommended starting point when resuming: step 2, collision.

- **Side-quest — double buffering: COMPLETE, validated on real
  hardware.** `render.c`/`render.h` rewritten for true double buffering
  (see "Double Buffering — Validated on Hardware" section below for the
  full technical findings this was built on). Public interface unchanged
  except one addition: `render_present()` must now be called once per
  frame, after all `render_draw_tile()` calls, to flip the completed
  backbuffer to the display. `main.c` updated accordingly. `dungeon.c/h`
  and `input.c/h` required zero changes — platform-abstraction boundary
  held.

  Confirmed on real hardware: static dungeon map renders correctly via
  the new backbuffer + glyph blitter, flicker is gone, and the character
  grid is now perfectly aligned to the screen's top-left corner
  (previously console.c's internal centering offset shifted it slightly
  — our version draws at raw cursor*8/cursor*16 with no offset, which
  turned out to be an improvement, not a regression).

  Milestone 3 sub-steps (collision, entities, etc.) are now unblocked.

- **Side-quest — sprite rendering: PLANNED, not yet started.** See
  "Sprite Rendering — Planned Upgrade" section below for full scope
  assessment. Comparable in size to the double-buffering work — the
  hardest infrastructure (backbuffer, flip mechanism, pixel-level
  blitting) already exists and sprites reuse it directly. Can be pursued
  as its own side-quest alongside or after Milestone 3, not a blocker to
  either.

- **Side-quest — menu system: PLANNED, not yet started.** Main menu,
  pause menu, possibly settings menu. Not expected to need new rendering
  or input primitives beyond what already exists — just new screen
  states in `main.c`'s loop. Pause menu's "Quit" action will call
  `xenon_smc_power_reboot()` — see "System Power Control — SMC Findings"
  below.

## Known Gotchas (accumulated)

- `xenos_init(VIDEO_MODE_AUTO)` must be called before `console_init()` —
  wrong order causes black-screen reboot or segfault soft-lock. Handled
  internally by `render_init()`.
- libxenon's console color format is BGRA-packed, not RGB:
  `(b<<24) | (g<<16) | (r<<8)`, alpha byte unused. Handled internally by
  `render.c`'s `pack_bgra()` helper — nothing outside render.c needs to
  know this.
- `usb_init()` (declared in `usb/usbmain.h`) already calls `kmem_init()`
  internally. Calling `kmem_init()` manually beforehand is deprecated —
  libxenon's own source prints a warning if you do. Don't call it from
  `input.c`.
- Docker builds create files owned by `root`. Run
  `sudo chown -R $USER:$USER ~/xbox_360_game/game` after every build.
- `struct controller_data_s` (in `libxenon/libxenon/drivers/input/input.h`)
  fields: `s1_x, s1_y, s2_x, s2_y` (stick axes), `s1_z, s2_z, lb, rb,
  start, back, a, b, x, y, up, down, left, right` (all `int`), `lt, rt`
  (analog triggers, `unsigned char`), `logo` (guide button). Only
  `up/down/left/right/a/b` used so far — sticks/triggers unused until a
  feature needs them.
- Docker builds must be launched from `~/xbox_360_game` (project root),
  NOT from inside `libxenon/` — running `docker run -v $PWD:/app` from
  the wrong directory mounts the wrong folder as `/app` and none of the
  project folders (`game/`, `hello/`, etc.) will be visible inside the
  container.

## Double Buffering — Validated on Hardware

Investigated after noticing render flicker (clear + full redraw every
frame with no back buffer). Confirmed true double buffering IS possible
and validated the mechanism end-to-end on real hardware, first via a
standalone `dblbuf_test/` folder (sibling to `game/`, same Makefile
template), then implemented for real in `render.c`/`render.h`. Now
COMPLETE and live in the game itself — see Milestone Plan above.

**Key technical findings:**
- Xenos GPU MMIO base is `0xec800000`. `xenos_write32(reg, val)` (exported
  by libxenon's own xenos.c, not declared in the public xenos.h — needs its
  own `extern` prototype) writes to `0xec800000 + reg`.
- Two different address forms for the same physical RAM:
  - CPU-usable pointer: `physical_addr | 0x80000000`
  - GPU-register-usable address: `cpu_ptr & ~0x80000000` (mask off bit 31)
  - Confirmed empirically: a plain `malloc()`/`memalign()` pointer already
    carries the `0x80` high byte, so heap memory works directly for both
    forms — no special physical-memory allocator needed.
- Register `D1GRPH_PRIMARY_SURFACE_ADDRESS` controls which buffer the GPU is
  actively scanning to the display. Flip pattern (confirmed working):

```c
xenos_write32(D1GRPH_UPDATE, 1);
xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, new_buffer_phys_addr);
xenos_write32(D1GRPH_UPDATE, 0);
```

  This is the same lock/write/commit pattern libxenon's own `xenos_init()`
  uses internally — not a novel technique, just reused.

**Prototype test results (`dblbuf_test/`):**
- Step 1: copied current screen content byte-for-byte into a second buffer,
  flipped to it, flipped back — confirmed non-destructive, no hang/corruption.
- Step 2: filled an entire second buffer with one solid color, flipped —
  screen went cleanly, uniformly solid-color full-screen, then flipped back
  correctly to the original text (including a printf issued while the
  backbuffer was on-screen, proving the two buffers are genuinely
  independent CPU-writable memory regions regardless of which one the GPU
  is scanning). No corruption, no partial fill, no hang across two full
  test runs on real hardware.
  - Note: the "~3 seconds" delay text in dblbuf_test is an uncalibrated
    busy-wait loop, not a real timer — actual observed delay was ~5s/~15s
    (loop counts were in a consistent 1:3 ratio, delays scaled ~1:3 too, so
    the delay behaves predictably, just isn't calibrated to real seconds).
- Step 3: drew a dark blue background + a red rectangle at a known
  position (x=100-300, y=100-200) into a manually-managed backbuffer,
  flipped to it. Rectangle appeared sharp-edged, correctly positioned, no
  scrambling/smearing/mis-tiled blocks — confirmed the tiling formula
  below is correct and buffer-agnostic, dodging the risk that a uniform
  fill (Step 2) would look correct even if tiling were wrong.

**Tiling formula — CONFIRMED, generalizes to arbitrary buffers:**
Xenos framebuffers use a macro-tiled memory layout, not row-major linear.
Reverse-derived from console.c's static `console_pset32`, then generalized
to take an explicit buffer pointer + width (works on ANY buffer, not just
the hardcoded `console_fb`):

```c
static inline void buf_pset32(unsigned int *buf, unsigned int width, int x, int y, unsigned int color) {
    unsigned int base = (((y >> 5) * 32 * width + ((x >> 5) << 10)
        + (x & 3) + ((y & 1) << 2) + (((x & 31) >> 2) << 3) + (((y & 31) >> 1) << 6))
        ^ ((y & 8) << 2));
    buf[base] = color;
}
```

**Glyph rendering internals — researched from libxenon console.c when
implementing render.c for real (not part of the original dblbuf_test
prototype, which only drew a solid rectangle, not text):**
- Font data: `fontdata_8x16` (8x16 px/glyph, 1 byte/row, MSB-first),
  defined via `font_8x16.h` in libxenon's console.c — GPL-licensed
  (sourced from the Linux kernel). We do NOT `#include` that header:
  it would vendor GPL source unnecessarily, and the array already has
  external linkage from being compiled into libxenon.a (re-including
  would cause a duplicate-symbol link error). Instead `render.c` declares
  `extern const unsigned char fontdata_8x16[];` and lets the linker
  resolve it against libxenon.a. Future: swapping in our own font data
  is a drop-in replacement of that one declaration.
- Glyph pixel lookup: `(fontdata_8x16[ch*16+y] >> (7-x)) & 1` selects
  bit x of row y — reverse-derived from console.c's `console_draw_char()`.
- console.c's `offset_x`/`offset_y` (static, not accessible outside
  console.c) intentionally NOT replicated — our version positions glyphs
  at raw `cursor*8`/`cursor*16`. Confirmed on hardware this actually
  improved alignment (grid now sits flush with the screen's top-left
  corner) rather than regressing it.

**Known rough edge (not yet fixed):** `console_init()` is still called in
`render_init()` (kept for printf debug output), but console.c's internal
`console_fb` pointer doesn't track our buffer-swapping — it always targets
the original primary address. This means debug `printf()` output may lag
a frame or get overwritten depending on which buffer is currently
on-screen vs. being drawn to. Not a problem for game rendering (which
bypasses `console_putch()` entirely), only cosmetic for debug text.

## Sprite Rendering — Planned Upgrade

Idea: move from ASCII/glyph rendering to actual 2D sprite images (static
flip-through frames per NPC/character, plus dungeon background images).
Scope assessed before committing — summary below, not yet started.

**Why this is a moderate addition, not a rewrite:**
- The platform-abstraction boundary already isolates this entirely
  inside `render.c`. `dungeon.c` and `main.c` only ever call
  `render_draw_tile()` — they have no awareness that glyphs are
  currently ASCII. Swapping the underlying blit target requires zero
  changes to game logic.
- The hard infrastructure problem is already solved: backbuffer,
  tiled-pixel addressing (`buf_pset32`), and the flip mechanism are all
  validated on hardware (see Double Buffering section above). A sprite
  blitter is structurally the same operation as `buf_draw_glyph()` — loop
  over width x height, write pixel color into the backbuffer — just
  reading from image data instead of `fontdata_8x16`.

**What's genuinely new work:**
1. Getting image data usable by PowerPC code at all. libxenon has no
   image decoder in the toolchain (no libpng/stb_image by default).
   Planned approach: convert art to raw pixel arrays offline (a small
   script on the dev machine), then bake into a `.h` as a `const` array
   (same pattern as `fontdata_8x16`) or load a raw binary blob at
   runtime. One-time tooling problem, not recurring.
2. Sprite sizing vs. the fixed 8x16 glyph grid. Sprites will likely span
   multiple grid cells. Needs a decision: keep tile-grid-based
   positioning with sprites spanning multiple cells, or move to free
   pixel positioning for sprite layers. Not yet decided.
3. Transparency/alpha. Glyphs are opaque (bg + fg color fill). Sprites
   will want transparent backgrounds — requires checking an alpha byte
   per pixel during blit and skipping the write. Not yet implemented.
4. Memory is not expected to be a real constraint at this project's
   scale.

**Verdict:** comparable in size to the double-buffering side-quest,
likely smaller since the hardest part (backbuffer + flip) is already
built. Main new skill is offline art-to-data conversion tooling, not
ongoing rendering complexity. Does not touch the platform-abstraction
boundary — confined to `render.c`.

## System Power Control — SMC Findings

Question investigated: can a running libxenon app (launched via
BadAvatar -> Aurora -> XeLL -> `xenon.elf`) return to Aurora, or at
least hard-reboot the console? Relevant for a pause/main menu "Quit"
option.

**Findings:**
- Returning to Aurora directly: NOT possible. By the time an app is
  running via XeLL, Aurora is not resident/reachable — there's no
  documented path back to it short of a fresh boot cycle re-triggering
  BadAvatar.
- Hard reboot: POSSIBLE, confirmed via source inspection.
  `xenon_smc_power_reboot(void)`, declared and implemented in
  `libxenon/libxenon/drivers/xenon_smc/xenon_smc.c` (declared line 69,
  implemented ~line 274). Sends a simple SMC command message:
  `uint8_t buf[16] = {0x82, 0x04, 0x30, 0x00};` via
  `xenon_smc_send_message(buf)`. `0x82` is the power-command opcode,
  sub-code `0x04` selects reboot.
- Shutdown also available: `xenon_smc_power_shutdown(void)`, same file
  (declared line 68, implemented ~line 268). Sends
  `uint8_t buf[16] = {0x82, 0x01};` — same opcode, sub-code `0x01` for
  shutdown.
- No special preconditions found. Both functions are simple,
  self-contained SMC message sends — no evidence of required setup
  beyond normal boot state. Since a reboot ends CPU execution entirely,
  no backbuffer flush or cleanup is needed beforehand.
- Practical implication for menu design: since a reboot cycles back
  through the console's normal boot chain (BadAvatar -> Aurora), calling
  `xenon_smc_power_reboot()` from a "Quit" menu option lands the player
  back at Aurora's dashboard — functionally equivalent to "quit to
  dashboard" even though there's no direct return path. Decision: pause
  menu's Quit action will call `xenon_smc_power_reboot()`. This should
  be confirmed on real hardware when the menu is actually built (verify
  it does land back at Aurora as expected, not e.g. straight back into
  XeLL or a different state).
- Note on research process: initial grep guess (`xenon_soc/
  xenon_power.c`, based on filename) was a red herring — that file is
  entirely CPU clock-speed/threading logic, nothing about power state.
  The real functions were in `xenon_smc/xenon_smc.c` instead. See Code
  Workflow Rules above for the generalized lesson.

## Prerequisites (not part of this repo)

- Console must have a working XeLL-Reloaded setup for its specific
  motherboard revision (Trinity, in our case). Setup files/instructions
  kept locally in `stuff/LaunchXell/` — excluded from git (third-party
  binaries, revision-specific). See that folder's own README for sources
  and steps if this needs to be reproduced on another console/machine.
- `libxenon/` (Free60Project) is cloned locally but gitignored — treated
  as an external dependency, not vendored into this repo. Re-clone from
  `https://github.com/Free60Project/libxenon.git` if setting up fresh.

## Open / Not Yet Decided

- Whether to eventually build the toolchain natively on Ubuntu (faster
  edit/compile loop, no Docker overhead) instead of the current
  Docker-based approach. Not a blocker — current setup works fine.
- Tile-type representation for dungeon.c (e.g. plain enum vs a struct
  per tile) — to be decided when starting Milestone 3 step 1.
- Debug printf lag/overwrite issue from buffer-swapping not tracked by
  console_fb (see "Known rough edge" under Double Buffering above) —
  cosmetic, not urgent.
- Sprite positioning model (tile-grid-based multi-cell sprites vs. free
  pixel positioning) — to be decided when sprite rendering side-quest is
  picked up.
- Exact objective types for the randomized-objectives idea (Design point
  6) — to be decided alongside Milestone 3 step 4/5.
- Whether a settings menu is actually needed for V1, or if main menu +
  pause menu (with Quit -> reboot) covers it — to be decided when the
  menu side-quest is picked up.
