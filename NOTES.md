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
- **Milestone 2 — Input + basic render loop (next up):** Read controller
  input; render a simple character grid to screen. Establish the basic
  game loop shape: read input -> update state -> draw.
- **Milestone 3 — The actual roguelike:** Dungeon generation, turn-based
  movement, inventory, monsters with simple AI, XP/leveling, etc. Deepen
  indefinitely over time per design decisions above.

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
