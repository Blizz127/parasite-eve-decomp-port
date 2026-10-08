# Native-only build: status

Current release: [`native-only-ab6de50b`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-ab6de50b)
(2026-10-08, Linux x86-64). It was tested on a real disc image on the build
machine (Day 1 plays through to the opening of Day 2), but it is **untested
by hand past Day 1** and **not yet tested on real hardware** such as a
handheld or a gaming PC.

It supersedes [`native-only-328511c8`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-328511c8), which is still
available. The earlier [`native-only-2918a7f0`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-2918a7f0) **stops about
12 minutes into Day 1** (before the first Eve fight); don't use it.

## What "native-only" means

- The game binary runs only recompiled C. It has **no MIPS interpreter and
  no CPU emulation**.
- Earlier builds still ran one piece of retail PlayStation code in a small
  built-in MIPS interpreter: the sound driver's tick (sequencer and voice
  updates, 240 times a second). That interpreter is now gone from the game.
  It exists only inside two test programs, where it serves as a reference.
- A game function that has no C version yet is never emulated. The game
  **stops** and prints a line like this:

  ```
  CPU_BOUNDARY/REFUSED pc=0x8007436C ra=<caller> sym=func_8007436C ... (native only: no C for this callee; not emulated)
  ```

  The `pc` value is the missing function, and `ra` is the native function
  that called it. If you hit one of these lines, please report it: it names
  exactly what to port next. Setting `PE_BOUNDARY_POLICY=record` turns the
  stop off, so the game logs the call and carries on without it. That is for
  diagnosis only: the skipped function's work is simply missing.

## What works

- **Sound driver.** It runs as native C, and it is checked against the
  matched decompiled C. The check runs both on 8,785 random driver states
  (one tick each) and compares all of RAM and the SPU registers: 0
  mismatches. The check is sensitive: deliberately changing a single line of
  the native code makes it fail.
- **Day 1, natively, on a real disc image.** The automated Day 1 route
  (New Game through every Day 1 room and battle, including the first Eve
  fight and the sewer alligator boss) passes to the end of Day 1. The Day 2
  movie check plays the Day 1 ending movies and reaches the opening of Day 2
  (story flag 0x90, the police chief's office). The full test suite passes
  1636 of 1636 with the disc present.
- **Room code (new in native-only-ab6de50b).** Room scripts that read pointers
  out of room data, call through code pointers or use inline GTE (3D maths)
  instructions now run as native C generated from the matched
  decompilation. A warp sweep of every Day 2 room at story flag 0x90 gives
  **377 of 402 room slots clean**. 4 rooms still stop (m0159i, m0256i,
  m0349i, m0432i) on room effect functions with no matched C yet, 1 returns
  to the title on purpose (m0050i) and 20 slots are unused. Probes of the
  Day 3 rooms (m0092i–m0098i, m0191i, m0360i) also ran clean. The probes
  only set the story flag and press X; they don't play the rooms.
- **Missing room code stops loudly.** A room that reaches unported code
  now stops with `CPU_BOUNDARY/REFUSED` instead of silently skipping it.
- **Fixed in native-only-328511c8.** With the sound tick native, the
  game's sound command queue reached a command (0x9B, `func_8008C720`) that
  the hand-written queue code didn't handle. It quietly stopped the game
  about 12 minutes into Day 1. Commands without a hand-written case now run
  their matched decompiled C; every command in the table has one. When the
  game stops for missing code, it now also prints
  `[STOP] reason=unresolved-boundary requested by: +0x...` naming where the
  stop came from.
- **Other native C added in the native-only builds:**
  - the sequence start and bank restore routines (`func_8008A750`,
    `func_8008AC40`);
  - five opcode handlers that now call `func_8008A92C` directly;
  - the reverb work-area clear;
  - BIOS `printf`, which goes to the terminal as `[TTY] ...` instead of
    stopping the game.

## Not done yet

These calls have no native version yet, so reaching one **stops the game**
with the message above:

- **Unexpected interrupt.** The interrupt handler's "return from exception"
  path (`func_80074384`, ReturnFromException). It only runs when an
  interrupt arrives that nothing registered for.
- **Other BIOS calls:** `0x80074354`, `0x8007436C`, `0x80074394`,
  `0x800743A4` and `func_80072A64`.
- **Game and library functions:** `func_80038D74`, `func_8001D170`,
  `func_8005A318`, `func_80080E34`, `func_80048918`, `func_8007E1E4`,
  `func_8007E1F4` and `func_80073A34`.
- **Overlay functions:** `func_800D27FC`, `func_800D0E88` and
  `func_800DB25C`.
- **Memory-card test mode:** a `sprintf` call that is only used there.
- **Divide by zero.** It stops on purpose, because the PlayStation traps
  there as well.

There may be others on paths nobody has played yet. Every stop line names
its function.

## Not tested

- **Real hardware.** This build has been run on a real disc image on the
  build machine only, headless, with the automated route. It has not been
  played on a handheld or a desktop yet.
- **Day 2 and later.** The automated route ends at the opening of Day 2.
  Nothing after that has been checked on the native-only build; expect
  stops there.
- The hardware-tested Day 1 build is still [r5](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/day1-r5).

## You need your own disc

No game data is included: no game files, no BIOS. You need Parasite Eve
(USA) Disc 1 as a raw BIN/CUE image.

## What the binary contains

Like r5, this binary is compiled from the full development tree. It
contains about 445 Psy-Q SDK-derived functions and credited upstream code
from khasinski/parasite-eve-decomp, which is not under this repository's
MIT license (see [UPSTREAM_FILES.md](../../UPSTREAM_FILES.md)). The Psy-Q
guard passes against its allowlist with no new findings. The retail-data
guard finds no retail byte runs in the package. As in r5, the PlayStation
SPU's 40-byte interpolation table (transcribed from psx-spx) is the only
table taken from the console.
