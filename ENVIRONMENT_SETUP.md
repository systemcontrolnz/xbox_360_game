# Xbox 360 Roguelike — Environment Setup Reference

Reference doc for how the dev environment is built. Not needed day-to-day —
use this if rebuilding on a new machine, or if something breaks and you need
to retrace steps.

## Hardware / Console

- Console exploit: BadAvatar (software-only avatar/profile exploit chain,
  not a hardware RGH glitch). Gets unsigned code executing through Aurora
  dashboard; does NOT give NAND read/write/flash access.
- Console motherboard revision: Trinity.
- Boot chain actually in use: Aurora -> launch default.xex -> boots into
  XeLL-Reloaded -> XeLL auto-detects and executes `xenon.elf` at the root
  of any attached storage (USB stick, in our case).
- XeLL setup files/binaries (revision-specific, third-party) are kept in
  `stuff/LaunchXell/` locally. NOT committed to git (see .gitignore) —
  see the README inside that folder for exact sources/download links and
  which files apply to which motherboard revision.

## Dev Machine

- Dell Precision 3470, dual-boot Windows 10 / Ubuntu.
- All toolchain work happens on Ubuntu — no maintained Windows build path
  for xenon-toolchain / libxenon.
- Ubuntu 22.04.4 LTS, kernel 6.8.0-124-generic, confirmed fully updated
  6 July 2026 (see prior changelog for the update/cleanup process itself —
  not reproduced here, not project-specific).
- Project folder: `~/xbox_360_game`

## Git / GitHub

- Git identity set globally (user.name "platy", user.email
  systemcontrolnz@gmail.com).
- SSH key: existing `~/.ssh/id_ed25519` reused (was already present from
  other unrelated projects on this machine) — added to GitHub via
  Settings -> SSH and GPG keys.
- Repo: `git@github.com:systemcontrolnz/xbox_360_game.git`, pushed and
  working.
- `.gitignore` excludes:
  - `libxenon/` — third-party SDK, cloned fresh rather than vendored
  - `stuff/` — third-party/external reference material (XeLL setup
    binaries, scratch files) that isn't our own authored code

## Docker

Installed via:
```
sudo apt update
sudo apt install docker.io
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker   # or log out/in, to pick up group membership
```
Verified with `docker run hello-world`.

## libxenon (SDK) + Toolchain

- Repo: `Free60Project/libxenon`, cloned to `~/xbox_360_game/libxenon`
  (gitignored — treat as an external dependency, re-clone if needed:
  `git clone https://github.com/Free60Project/libxenon.git`)
- Build via prebuilt Docker image `free60/libxenon:latest`, which already
  contains a working PowerPC cross-compiler toolchain at
  `/usr/local/xenon` (env var `DEVKITXENON` is pre-set inside the
  container) — no need to build the cross-compiler from source.
- To build libxenon itself:
  ```
  cd ~/xbox_360_game
  docker run -it -v $PWD:/app free60/libxenon:latest
  # inside container:
  cd /app/libxenon/ports/xenon
  make
  ```
  Produces `libxenon.a`. Dozens of compiler warnings are normal (old
  codebase, modern compiler) — only `error:` lines matter.
- This currently uses the Docker shortcut (Phase 1 validation approach).
  Native toolchain build on Ubuntu directly (for a faster edit/compile
  loop without Docker overhead) has NOT been done yet — still an option
  to pursue later, not required for progress.

## Building Our Own Code (e.g. `hello/`)

Project code lives as sibling folders to `libxenon/` under
`~/xbox_360_game` (NOT nested inside the libxenon clone — that caused a
git-tracking issue early on, since a nested `.git` folder makes git treat
the inner folder as a separate repo).

Each buildable folder (e.g. `hello/`) has its own Makefile modeled on
`libxenon/devkitxenon/examples/xenon/graphics/cube/Makefile`, using
`$(DEVKITXENON)/rules` for the actual compile rules, linking `-lxenon -lm`.

To build:
```
cd ~/xbox_360_game
docker run -it -v $PWD:/app free60/libxenon:latest
# inside container:
cd /app/hello
make clean
make
```
Produces `hello.elf` (unstripped, 64-bit-ish PowerPC ELF, has debug
symbols from `-g`) and `hello.elf32` (stripped + converted — **this is
the correct file to actually run**, matches what XeLL's loader expects).

**Note on file permissions:** files created inside the Docker container
are owned by root. If editing them afterward from the normal Ubuntu user
hits "Permission denied", fix with:
```
sudo chown -R $USER:$USER ~/xbox_360_game/hello
```

## Getting a Build Onto the Console

1. Copy `hello.elf32` onto the USB stick that's normally attached for
   BadAvatar, renamed to exactly `xenon.elf`, at the root of the drive.
   (Back up any existing `xenon.elf` first if it's serving another
   purpose — e.g. it was previously a renamed mupen64 binary.)
2. Boot into XeLL the usual way (via Aurora -> default.xex).
3. XeLL auto-detects `xenon.elf` at the root and executes it.

**Important:** libxenon-built homebrew is NOT `.xex` format and does NOT
show up as a "game" in Aurora's normal game list — it's raw `.elf32`,
specifically launched via XeLL. This corrects the original assumption
that Aurora would launch a `.xex` directly.

## Known Gotcha: Black Screen / Reboot / "Segmentation fault!"

If a libxenon app crashes immediately (black screen + reboot, or a
"Segmentation fault!" soft-lock), the near-certain cause is missing video
init. Must call, in this order, before any `console_init()` or `printf`:

```c
xenos_init(VIDEO_MODE_AUTO);
console_init();
```

Confirmed via bisection testing on real hardware:
- `xenos_init()` alone -> works, program exits cleanly back to XeLL
- `+ console_init()` -> works, shows built-in diagnostic line
  ("Xenos FB with ... initialized.")
- `+ printf()` -> works, our own text renders correctly

This is now the known-working baseline pattern for any new libxenon app.
