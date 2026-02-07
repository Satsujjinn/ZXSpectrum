# THE GAME — Sinclair ZX Spectrum (48K)

A minimal art game for the **Sinclair ZX Spectrum 48K**: you are light. Move through three chambers. Reach the star.

- **Title:** THE GAME  
- **Controls:** 5/6/7/8 (left/right/down/up) or Q/P/A/O  
- **Goal:** Reach the bright **\*** in each room to advance; three rooms, then “LIGHT”.

C project built with **z88dk**, debugged with **DeZog** in VS Code/Cursor.

## Toolchain

- **[z88dk](https://z88dk.org)** – C compiler and libraries for Z80 (ZX Spectrum, etc.)
- **DeZog** – VS Code extension for debugging Z80/.TAP (uses `game.tap` + `game.map`)

## Setup

1. **Install z88dk** (macOS with Homebrew):
   ```bash
   brew install z88dk
   ```
   Or follow [z88dk – Getting started](https://z88dk.org/site/gettingstarted/) for your OS.

2. **Install DeZog** in Cursor/VS Code (Extension: “DeZog – Z80 Debugger”).

3. **Optional:** Install an emulator (e.g. [Fuse](https://fuse-emulator.sourceforge.io/), [ZEsarUX](https://sourceforge.net/projects/zesarux/)) to run `game.tap` outside the debugger.

## Build & run

| Action        | Command / shortcut |
|---------------|--------------------|
| Build         | `make` or **Tasks → Run Build Task** |
| Run in Fuse   | `make run` (opens `game.tap` in Fuse) |
| Debug (DeZog) | **F5** or Run → Start Debugging (“DeZog (ZX Spectrum)”) |

**In Fuse:** Load the tape with **LOAD ""** then press **Enter** (or use the tape menu to start loading). For `-create-app` builds the tape has a short BASIC loader, then the machine-code program runs.

Outputs:

- `game.tap` – tape image (load in emulator or DeZog)
- `game.map` – symbol map for DeZog

## Project layout

```
.
├── Makefile           # Builds game.tap + game.map
├── src/
│   └── main.c         # Entry point
├── .vscode/
│   ├── launch.json    # DeZog launch config
│   └── tasks.json     # make task
└── README.md
```

## ZX Spectrum basics (48K)

- **CPU:** Z80 @ 3.5 MHz  
- **RAM:** 48 KB  
- **Display:** 256×192 pixels, 32×24 attribute cells (ink/paper/bright/flash)  
- **Colours:** 8 (black, blue, red, magenta, green, cyan, yellow, white)

z88dk gives you `arch/zx.h` (e.g. `zx_cls`, `zx_border`), `input.h` (keyboard), and standard C. For more, see [z88dk platform: ZX](https://www.z88dk.org/wiki/doku.php?id=platform%3Azx).

Happy coding on the Speccy.
