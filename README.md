# parasite-eve-decomp-port

Parasite Eve decomp and port: a matching decompilation of Parasite Eve
(PlayStation, USA, SLUS-00662) and a native PC port that runs the decompiled
game code as compiled host C.

> **You need your own disc.** This repository and its releases contain no game
> data, no BIOS and no Sony SDK code. The port reads everything from your own
> copy of Parasite Eve (USA) Disc 1. See [NOTICE.md](NOTICE.md).
>
> **Current build: native-only-d5efe5c0 (2026-11-25).** Release
> [`native-only-d5efe5c0`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-d5efe5c0)
> ships a Linux x86-64 archive and a Windows x64 archive.
> Jump to [How to play](#how-to-play).

## About this project

This is a passion project. I'm working hard on it, but it's made for fun and
for everyone's enjoyment: free, non-commercial, and made by a fan. If you
enjoy it, that's the whole point.

## How to play

This is a native build of Parasite Eve, not an emulator. You need your own
copy of the game; nothing from the game is included. No BIOS is needed. It is
an early research preview: **Day 1 is the verified playable slice**; later
days run but are less tested. Expect rough edges beyond Day 1.

### 1. Download

From the
[`native-only-d5efe5c0` release](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-d5efe5c0),
download the archive for your system, plus `SHA256SUMS` if you want to check
the file:

| System | Asset |
|---|---|
| Linux x86-64 | `parasite-eve-native-only-d5efe5c0-linux-x64.tar.gz` |
| Windows x64 | `parasite-eve-native-only-d5efe5c0-windows-x64.zip` |

SHA-256 of the two archives (the same values are in `SHA256SUMS`):

```
6b2db2ad65cc56de73503b9104e078a333f472d35ef98e0d22ae3099caf0ff28  parasite-eve-native-only-d5efe5c0-linux-x64.tar.gz
1afdea4af4cca77afd1e1f577f18b910a335eaae6fb823c448b7545a2fefa448  parasite-eve-native-only-d5efe5c0-windows-x64.zip
```

To check on Linux, run `sha256sum -c SHA256SUMS` in the folder that holds the
downloads.

### 2. Your disc

You need **Parasite Eve (USA) Disc 1** as a raw **BIN/CUE** image. This is
the "Redump" kind: one raw `.bin` (2352 bytes per sector) next to its `.cue`.
An `.iso` or `.chd` does not work directly.

If you have a `.chd`, convert it with
`chdman extractcd -i "Parasite Eve (USA) (Disc 1).chd" -o "Parasite Eve (USA) (Disc 1).cue" -ob "Parasite Eve (USA) (Disc 1).bin"`.
`chdman` comes with MAME.

Nothing needs to be extracted from the disc. The port reads the image
directly.

### 3a. Run on Linux

```sh
tar -xzf parasite-eve-native-only-d5efe5c0-linux-x64.tar.gz
cd parasite-eve-day1-preview
./parasite-eve-preview "/path/to/Parasite Eve (USA) (Disc 1).bin"
```

The disc argument may be the Disc 1 `.bin`, its `.cue`, or the folder that
holds them. Extra arguments go to the game, for example
`./parasite-eve-preview disc1.bin --scale 4`.

Requirements: 64-bit Linux with glibc and `libX11.so.6` (loaded at runtime;
Wayland desktops and Steam Game Mode use XWayland). Controllers use
`libSDL2-2.0.so.0` when present. Sound uses `libpulse-simple.so.0`
(PipeWire's pulse compatibility provides it; the game is silent without it).
Under Steam Game Mode / gamescope it starts fullscreen; `PE_FULLSCREEN=0`
keeps a window. The package's `README-PREVIEW.txt` lists the controller
environment variables and more detail.

### 3b. Run on Windows

1. Unzip anywhere (a normal user folder). The game writes its save under
   `%APPDATA%\parasite-eve-port`.
2. Create a folder named `disc` next to `parasite-eve.cmd` and put your Disc 1
   `.bin` and `.cue` in it.
3. Double-click `parasite-eve.cmd`, or run it from Command Prompt. To use a
   disc kept elsewhere:

```bat
set PE_DISC=D:\dumps\Parasite Eve (USA) (Disc 1).bin
parasite-eve.cmd
```

Extra arguments go to the game, for example `parasite-eve.cmd --scale 4`;
`--fullscreen` opens a borderless full-screen window. Requirements: 64-bit
Windows 10 or later, nothing else to install. The package's
`README-WINDOWS.txt` has the list of files the game writes.

**The Windows build was tested under Wine only.** It is cross-compiled from
the same sources as the Linux build. Controller mapping, fullscreen behaviour,
audio devices and everything else on real Windows are unverified.

### Controls

| PlayStation | Gamepad (Xbox labels) | Keyboard |
|---|---|---|
| D-pad / move | D-pad or left stick | Arrow keys |
| Cross (confirm, act, fire) | A | Enter, Space, Z or X |
| Circle (cancel) | B | C |
| Square | X | S |
| Triangle (menu) | Y | V |
| L1 / R1 | LB / RB | Q / E |
| L2 / R2 | LT / RT | 1 / 3 |
| Select | Back / View | Tab |
| Start | Start / Menu | P |
| Fast-forward on/off | L3 (click left stick) | F6 |
| Fast-forward while held | R3 (click right stick) | none |
| Quit | none | Esc |

Keyboard and controllers work at the same time. On Linux, controllers are
read through SDL2's game controller layer and you can plug one in or remove
it while the game runs; on Windows, Xbox-compatible pads work through XInput.
In Steam, a non-Steam game shortcut gets Steam Input's virtual pad; set the
controller layout to "Gamepad" if Steam shows a desktop or mouse layout.
The developer menu is off in this build.

### Troubleshooting

- **"not a raw 2352-byte/sector BIN".** You gave it an `.iso` or a converted
  image. Use the `.bin` from a BIN/CUE dump (or convert the `.chd`).
- **"no X display" (Linux).** Run it from a desktop session or Game Mode.
- **No sound (Linux).** Sound needs `libpulse-simple.so.0`. If sound
  crackles, try `PE_AUDIO_LATENCY_MS=150`.
- **The controller does nothing (Linux).** Try `PE_PAD_DEBUG=1` to print the
  controller devices; if Square and Triangle are swapped, set
  `PE_PAD_SWAP_XY=1`.
- **The game stops with a `CPU_BOUNDARY/REFUSED` line.** That names a game
  function that has no native version yet. Please report it.
- **Reporting a bug.** Include the build id from `build-info.json` in the
  package folder, the room and time, and the console output.

### FAQ

- **Do I need a PlayStation BIOS?** No.
- **Is the game included?** No. There are no game files, music or movies. You
  must own the disc, and the port reads your copy.
- **How far does it go?** Day 1 (the opera and the first boss) is the
  verified playable slice. Later days run but are less tested.
- **Windows?** Yes, as a Wine-tested preview; see above.
- **Can I build the port from this repository?** Not yet; see
  [Build from source](#build-from-source).

## What is in the current build

- **Native claim.** The game executes as compiled host C, with no interpreter
  and no CPU emulation. The symbol-based Psy-Q audit
  (`psyq_port_guard.py --binary --require-exe`, run against the original
  executable) finds **0 Psy-Q-classified functions linked** in the Linux
  binary, and the memory-card BIOS entries forward to the port's own card
  model, which saves to a local card file. Provenance correction (2026-11-26): a file-level provenance audit of the host providers found that the providers covering about 109 of the original SDK functions (SPU, counter and GPU state words, GPU packet and image-transfer workers, CD stream and IRQ delivery, libgte state, libsnd init, save manager) were translated from the matched decompilation or carry no black-box derivation evidence. They are not clean-room, their sources stay unpublished, and they are being re-derived from black-box vectors and psx-spx family by family; each follow-up release lists what moved. The other providers (CD leaves, GTE commands, BIOS card, string and memory entries, MDEC, pad) were written clean-room from vectors.
- **Verified.** Native test suite 1935 run, 1924 passed, 0 failed, 6 skipped
  (the six quarantined adapter tests), with the disc attached. On the Linux
  binary: cold boot 60 of 60 frames identical to the baseline with audio
  equal; the Day 1 pad-only route with movies to the first boss victory,
  187 of 187 sampled frames identical to the baseline with route audio equal;
  and a save and Continue round trip, which passes with the card unchanged by
  loading. The Windows package was smoke-tested under Wine (headless): it
  boots, reaches the title and first player control, and every screenshot
  matches the Linux binary's.
- **Retail-data scan.** One 40-byte run in each binary: the SPU Gaussian
  interpolation table `k_gauss`, a documented hardware constant table. No
  other retail, BIOS or disc bytes.
- **Known limits.** Windows is tested under Wine only. Later days are less
  tested. This is a preview; please report issues.

Details and the full list of what was checked:
[docs/port/NATIVE_ONLY_STATUS.md](docs/port/NATIVE_ONLY_STATUS.md) and the
[release notes](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-d5efe5c0).

## What is here

- `src/`: the matching decompilation. This repository has **1,772 EXE
  functions in C** that compile back to the retail bytes, plus the overlay
  functions. The development tree has 2,252 matched EXE functions. The
  difference is the Psy-Q SDK functions and two functions with retail
  tables, which are split from your disc as assembly instead.
- `pc_port/`: the native port, including the host layer (window, input,
  audio, saves), the host providers that replace the Psy-Q SDK code (clean-room
  ones published, translated ones held until re-derived), the disc checks, the
  opt-in cheats, and its tests.
- `configs/`: splat and symbol configuration (addresses and names only).
- `tools/`, `scripts/`, `dev/`: the analysis tools, including the retail-data
  and Psy-Q guards, the build scripts and the build container.
- `docs/`: project, port and divergence documentation.

## Build from source

The matching build is turnkey: from a clean clone and your own disc, it
rebuilds the retail executable byte for byte (SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`). Every retail byte that isn't
published here is split from your disc at build time. That covers the
assembly, the data, and the Psy-Q SDK functions whose source isn't published.
You need git and Docker.

```sh
git clone https://github.com/Blizz127/parasite-eve-decomp-port.git
cd parasite-eve-decomp-port

# Your disc: the Redump BIN/CUE of Disc 1, copied (or linked) here
mkdir -p "rom/image/Parasite Eve (USA) (Disc 1)"
cp ~/Games/parasite-eve/disc/"Parasite Eve (USA) (Disc 1)".{bin,cue} "rom/image/Parasite Eve (USA) (Disc 1)/"

# The build container (Debian with a mipsel cross toolchain)
docker build -t parasite-eve-mipsel:trixie -f dev/mipsel/Dockerfile .

# Toolchain setup, extract, split, build and verify
docker run --rm -u "$(id -u):$(id -g)" -e HOME=/tmp -v "$PWD":/workspace \
  parasite-eve-mipsel:trixie bash -c '
    scripts/setup_env.sh && scripts/setup_era.sh &&
    export PATH=/workspace/.venv/bin:$PATH &&
    scripts/extract_us.sh 1 && scripts/split_us.sh &&
    scripts/build_us.sh && scripts/verify_us.sh'
```

- **`setup_env.sh`** installs the pinned splat into `.venv/`.
- **`setup_era.sh`** downloads the era compilers
  ([decompals/old-gcc](https://github.com/decompals/old-gcc)) and
  [maspsx](https://github.com/mkst/maspsx) at a pinned commit, and keeps this
  repo's patched maspsx files.
- The build ends with `Compare: EXACT SHA-1 452fb033…`, and
  `verify_us.sh` prints `VERIFY_US=PASS`.

Everything the build writes (`build/`, `asm/`, `assets/`) is git-ignored,
and your disc is only read.

**The native port** (`pc_port/`) does not build from this repository alone
yet. This is work in progress. The public source now holds the clean-room
host providers that replace the Psy-Q SDK code, but a few providers are still
held while their provenance is audited, and the curated public copies of
`pc_port/CMakeLists.txt` and the test harness lag the development tree and are
being made public-ready. The prebuilt release archives are the runnable
deliverable until then. The matching retail rebuild above is unaffected.

## Status

Since 2026-10-07 the port is **native only**: the game binary has no MIPS
interpreter or CPU emulation, and the sound driver runs as native C. A game
function that has no native version yet is never emulated; the game stops and
prints a `CPU_BOUNDARY/REFUSED` line that names it. As of 2026-11-25 the
Linux binary links 0 Psy-Q-classified functions by the symbol-based audit (see
the provenance correction above: about 109 SDK functions are still covered by
translated providers under re-derivation); the current release is
[`native-only-d5efe5c0`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-d5efe5c0).
Day 1 is the verified playable slice; later days run but are less tested.
Nothing has been played on real Windows hardware, and the device checks
(controller, borderless window, on-screen keyboard) have not been run.
See [docs/port/NATIVE_ONLY_STATUS.md](docs/port/NATIVE_ONLY_STATUS.md). Every
known difference from the PlayStation original is logged in
[docs/port/KNOWN_DIVERGENCES.md](docs/port/KNOWN_DIVERGENCES.md).

## Earlier builds

These are kept for history. Use `native-only-d5efe5c0` above.

- [`native-only-e26d98fd`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-e26d98fd)
  (2026-10-10, Linux x86-64): native C only, no MIPS interpreter. It played
  Day 1 through to the opening of Day 2 on a real disc image.
- [`day1-r5`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/day1-r5)
  (Linux x86-64): the hardware-tested Day 1 build. Day 1 plays from start to
  finish with the movies. It contains about 445 Psy-Q-derived functions and
  credited upstream code (see [UPSTREAM_FILES.md](UPSTREAM_FILES.md)).
- [`native-only-2918a7f0`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-2918a7f0)
  stops about 12 minutes into Day 1; don't use it.

## Credits and license

- Credits: [CREDITS.md](CREDITS.md), covering khasinski's decompilation, the
  tools and the libraries. The imported files are listed in
  [UPSTREAM_FILES.md](UPSTREAM_FILES.md). The Windows package also ships the
  AMD FidelityFX FSR 1 (MIT) license and the SDL_GameControllerDB (zlib)
  license under `LICENSES\`.
- License: [LICENSE](LICENSE) (MIT), for this project's original work only.
- Legal notice and disclaimer: [NOTICE.md](NOTICE.md). Parasite Eve is ©
  Square Enix. This project is not affiliated with or endorsed by Square
  Enix.
