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
   style), NOT hand-drawn sprite tiles. Sprite art needs actual artwork
   we can't produce; colored characters on a monospace grid can be built
   entirely in code with zero art assets. Real tile art is a possible
   future upgrade, but that's an ART problem, not a code problem.
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
  face value, they can be outdated (see kmem_init gotcha below).

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

- **Milestone 3 — The actual roguelike (next up).** Broken into ordered
  sub-steps rather than tackled all at once, per one-feature-per-turn
  workflow:
  1. **`dungeon.c/h` — static test map.** Define a tile-type grid (floor,
     wall) and a fixed hand-authored test layout (not procedural yet).
     Render it via `render_draw_tile()` — walls and floor as distinct
     glyphs/colors. No player interaction yet, just confirm a multi-tile
     map draws correctly on hardware.
  2. **Collision.** Wire player movement (currently free-roaming from
     Milestone 2) to check the dungeon grid before moving — walking into
     a wall tile should be blocked. This is the first point `main.c`'s
     loop needs to know about both `dungeon.c` and `input.c` together.
  3. **`entities.c/h`.** Formalize player state (currently just loose
     `x`/`y` ints in `main.c`) into a proper struct — position, HP, and
     room for stats to be added later (XP, level). Monsters will reuse
     this same struct shape.
  4. **Procedural generation.** Replace the static test map with actual
     room/corridor dungeon generation. Deliberately last in this list —
     easiest to validate generation logic once rendering+collision
     already work against a known-good static map.
  5. **Monsters + simple AI, combat resolution, XP/leveling.** Deepen
     indefinitely per the design decisions above, once the floor itself
     is solid.

  Recommended starting point when resuming: **step 1, the static test
  map in `dungeon.c/h`.**

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
 
  ## Double Buffering — Validated on Hardware (side-quest, not part of
   Milestone 3's main sequence, but blocks the flicker fix)

Investigated after noticing render flicker (clear + full redraw every frame
with no back buffer). Confirmed true double buffering IS possible and have
now validated the mechanism end-to-end on real hardware, via a standalone
`dblbuf_test/` folder (sibling to `game/`, same Makefile template).

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

xenos_write32(D1GRPH_UPDATE, 1);
xenos_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, new_buffer_phys_addr);
xenos_write32(D1GRPH_UPDATE, 0);

This is the same lock/write/commit pattern libxenon's own `xenos_init()`
  uses internally — not a novel technique, just reused.

**Test results (dblbuf_test/):**
- Step 1: copied current screen content byte-for-byte into a second buffer,
  flipped to it, flipped back — confirmed non-destructive, no hang/corruption.
- Step 2: filled an entire second buffer with one solid color, flipped —
  screen went cleanly, uniformly solid-color full-screen, then flipped back
  correctly to the original text (including a printf issued *while the
  backbuffer was on-screen*, proving the two buffers are genuinely
  independent CPU-writable memory regions regardless of which one the GPU
  is scanning). No corruption, no partial fill, no hang across two full
  test runs on real hardware.
- Note: the "~3 seconds" delay text in dblbuf_test is an uncalibrated
  busy-wait loop, not a real timer — actual observed delay was ~5s/~15s
  (loop counts were in a consistent 1:3 ratio, delays scaled ~1:3 too, so
  the delay behaves predictably, just isn't calibrated to real seconds).

**Open question before this can be wired into `render.c` for real:**
Xenos framebuffers may use a tiled/swizzled memory layout for arbitrary
pixel positions (unconfirmed either way yet) — our solid-fill test dodged
this entirely since a uniform fill looks correct regardless of tiling.
Writing individual glyphs/tiles into an off-screen buffer will need to
either reuse whatever addressing logic `console.c`'s pixel-plotting
function already uses internally, or reimplement it for our own buffer.
This is the next research step before double buffering can replace the
current single-buffer `render.c`.

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
