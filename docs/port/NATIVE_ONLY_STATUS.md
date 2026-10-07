# Native-only build: status

Release: [`native-only-2918a7f0`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-2918a7f0)
(2026-10-07, Linux x86-64). **UNTESTED on a real disc.**

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
- **The development line this build comes from.** It passes the full Day 1
  route natively. The Day 2 movie check reaches story flag 0x90.
- **Other native C added in this build:**
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

- This exact build has **not been run on a real disc**. Earlier builds of the
  same line passed the Day 1 route on a disc. This build has not been through
  those checks yet.
- **Why it hasn't been through them.** It was built on a machine without the
  disc. The overlay fingerprints (hashes the port uses to recognise loaded
  overlays) were copied from the registry of the previous disc-built
  checkpoint (`93fe826a`). The two entry sets are identical, apart from two
  new EXE entries that don't use a fingerprint.
- If you want the tested Day 1 build, use [r5](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/day1-r5).

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
