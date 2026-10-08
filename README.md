# parasite-eve-decomp-port

Parasite Eve decomp and port: a matching decompilation of Parasite Eve
(PlayStation, USA, SLUS-00662) and a native Linux port that runs the
decompiled game code.

> **You need your own disc.** This repository contains no game data, no BIOS
> and no Sony SDK code. The port reads everything from your own copy of
> Parasite Eve. See [NOTICE.md](NOTICE.md).
>
> **Just want to play?** Jump to [How to play](#how-to-play).
>
> **New (2026-10-07): native-only build.** Release
> [`native-only-328511c8`](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/native-only-328511c8)
> runs only native C: no MIPS interpreter and no CPU emulation in the game.
> A function that isn't ported yet stops the game with a
> `CPU_BOUNDARY/REFUSED` message instead of being emulated. On a real disc
> image it plays Day 1 through to the opening of Day 2. It has not been
> played on real hardware yet, and Day 2 onward is unverified. What works and
> what doesn't: [docs/port/NATIVE_ONLY_STATUS.md](docs/port/NATIVE_ONLY_STATUS.md).
> The earlier `native-only-2918a7f0` stops about 12 minutes into Day 1;
> don't use it. The hardware-tested Day 1 build is still
> [r5](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/day1-r5).

## About this project

This is a passion project. I'm working hard on it, but it's made for fun and for everyone's enjoyment — free, non-commercial, and made by a fan. If you enjoy it, that's the whole point.

## How to play

This is a native Linux build of Parasite Eve, not an emulator. You need your
own copy of the game; nothing from the game is included. No BIOS is needed.
**Day 1 plays from start to finish.**

### 1. Download

From the [r5 release](https://github.com/Blizz127/parasite-eve-decomp-port/releases/tag/day1-r5),
download **`parasite-eve-day1-day1-r5-16a267b1-linux-x64.tar.gz`**. You can
also download `SHA256SUMS` if you want to check the file.

> **What is in the r5 build.** The r5 binary is compiled from the full
> development tree, so it contains two things this repository's source
> leaves out:
>
> - **Psy-Q SDK-derived code:** about 445 functions that translate Sony's
>   Psy-Q libraries (graphics, CD, pad, sound, GTE and others).
> - **khasinski's upstream code:** credited, and not under this
>   repository's MIT license (see [UPSTREAM_FILES.md](UPSTREAM_FILES.md)).
>
> It includes no game assets, apart from the PlayStation SPU's 40-byte
> sound-interpolation table. All of this is being replaced by the project's
> own code in later releases (a Psy-Q-free port layer).

### 2. Install

Unpack it into `~/Games`:

```sh
mkdir -p ~/Games
tar xzf ~/Downloads/parasite-eve-day1-day1-r5-16a267b1-linux-x64.tar.gz -C ~/Games
```

That gives you `~/Games/parasite-eve-day1-day1-r5-16a267b1/`, with the
launcher `parasite-eve` inside.

### 3. Your disc

You need **Parasite Eve (USA) Disc 1** as a raw **BIN/CUE** image. This is
the "Redump" kind: one `.bin` of 495,531,120 bytes, next to its `.cue`. An
`.iso` or `.chd` does not work directly.

- **Ripped with a disc tool to BIN/CUE:** use the `.bin` as it is.
- **You have a `.chd`:** convert it with
  `chdman extractcd -i "Parasite Eve (USA) (Disc 1).chd" -o "Parasite Eve (USA) (Disc 1).cue" -ob "Parasite Eve (USA) (Disc 1).bin"`.
  `chdman` comes with MAME. On Bazzite or Steam Deck, install `mame-tools`
  in a distrobox.

Put the `.bin` and `.cue` in a folder of their own:

```
~/Games/parasite-eve/disc/Parasite Eve (USA) (Disc 1).bin
~/Games/parasite-eve/disc/Parasite Eve (USA) (Disc 1).cue
```

Nothing needs to be extracted. The port reads the image directly and checks
it on start.

### 4. Run it

```sh
~/Games/parasite-eve-day1-day1-r5-16a267b1/parasite-eve ~/Games/parasite-eve/disc
```

You can pass the folder, the `.bin` or the `.cue`. It starts fullscreen. For
a window instead:

```sh
PE_WINDOWED=1 ~/Games/parasite-eve-day1-day1-r5-16a267b1/parasite-eve ~/Games/parasite-eve/disc
```

To run it without typing the disc path every time, save the path once:

```sh
mkdir -p ~/.config/parasite-eve-port
echo "disc1=$HOME/Games/parasite-eve/disc" > ~/.config/parasite-eve-port/config
```

After that, `~/Games/parasite-eve-day1-day1-r5-16a267b1/parasite-eve` alone
is enough, and you can double-click it in a file manager.

### 5. Steam Deck / Bazzite Game Mode

1. In Desktop Mode, save the disc path once (the `config` step above).
2. In Steam, choose **Games → Add a Non-Steam Game → Browse**. Pick
   `~/Games/parasite-eve-day1-day1-r5-16a267b1/parasite-eve`.
3. Switch to Game Mode and start it from your library. No launch options are
   needed.
4. Controller: the built-in controls work as a standard Xbox-style gamepad.
   Use Steam's default "Gamepad" layout.

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
| Fast-forward while held | R3 (click right stick) | — |
| Quit | — | Esc |

Keyboard and controllers work at the same time. You can plug a controller in
or remove it while the game runs.

**Dev menu (cheats; off unless you press them):** F1 shows the help. The
other keys are F2 god mode, F3 infinite ammo and full PE, F4 win the current
battle, F5 key items, F6 fast-forward and F7 fast-forward speed. F8 opens the
warp menu: Up/Down chooses, Enter warps, F8 closes it. Use warps while
walking around in the field. "CHEATS" shows in the corner while any cheat is
on.

### Saves

Memory card saves go to `~/.local/share/parasite-eve-port/memcard1.mcd`.
They survive updates, because the install folder is never written to.

### Troubleshooting

- **Nothing happens when I double-click it.** The launcher didn't find your
  disc. Run it from a terminal with the disc path (step 4) and read the
  message.
- **"not a raw 2352-byte/sector BIN".** You gave it an `.iso` or a converted
  image. Use the `.bin` from a BIN/CUE dump (or convert the `.chd`, step 3).
- **The disc is rejected at start.** The port checks for USA Disc 1
  (SLUS-00662). A different region, Disc 2, or a modified image is refused,
  with a message saying why.
- **"no X display".** Run it from a desktop session or Game Mode.
  Wayland desktops provide one through XWayland.
- **`GLIBC_2.34' not found`.** Your Linux is too old. The build needs
  glibc 2.34 or newer: a 2021-or-later distribution, including SteamOS 3
  and Bazzite.
- **No sound.** Sound needs `libpulse-simple.so.0`, which PipeWire provides
  on current distributions. If sound crackles, try
  `PE_AUDIO_LATENCY_MS=150`.
- **The controller does nothing.** Check that `ls -l /dev/input/js*` lists a
  device your user can read. If Square and Triangle are swapped, set
  `PE_PAD_SWAP_XY=1`.
- **Logs:** every run writes `~/.local/state/parasite-eve-port/logs/run-<time>.log`
  (the last 10 are kept). Include the newest one, with the time and room and
  a screenshot, when you report a bug.

Known issues in r5:

- Using some equipment or tool items from the field menu may stop the game.
  Healing items are fine.
- Loading screens are shorter than on a real PlayStation.
- Aya's gun model may be missing when she readies it.

### FAQ

- **Do I need a PlayStation BIOS?** No.
- **Is the game included?** No. There are no game files, music or movies.
  You must own the disc, and the port reads your copy.
- **How far does it go?** Day 1 plays from start to finish: the opera,
  Carnegie Hall, the sewers and the alligator boss, with the movies. Days 2
  to 6 and Disc 2 are in progress.
- **Windows?** Not yet. This release is Linux x86-64 only.

## What is here

- `src/`: the matching decompilation. This repository has **1,772 EXE
  functions in C** that compile back to the retail bytes, plus the overlay
  functions. The development tree has 2,252 matched EXE functions. The
  difference is the Psy-Q SDK functions and two functions with retail
  tables, which are split from your disc as assembly instead.
- `pc_port/`: the native port, including the host layer (window, input,
  audio, saves), the disc checks, the opt-in cheats, and its tests.
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

**The native port** (`pc_port/`) can't be built from this repository yet.
The files that still translate Psy-Q SDK code are being replaced by the
project's own port layer, and they are not published. The prebuilt release
is the way to play until then.

## Status

Day 1 is playable from start to finish on the port (r5), and movies play in
real time. Since 2026-10-07 the port is **native only**: the game binary has
no MIPS interpreter or CPU emulation, and the sound driver runs as native C.
The native-only build plays Day 1 to the opening of Day 2 on a real disc
image; it has not been played on real hardware yet. The calls that still stop
it are listed in
[docs/port/NATIVE_ONLY_STATUS.md](docs/port/NATIVE_ONLY_STATUS.md). Every known difference from the PlayStation original is logged in
[docs/port/KNOWN_DIVERGENCES.md](docs/port/KNOWN_DIVERGENCES.md).

## Credits and license

- Credits: [CREDITS.md](CREDITS.md), covering khasinski's decompilation, the
  tools and the libraries. The imported files are listed in
  [UPSTREAM_FILES.md](UPSTREAM_FILES.md).
- License: [LICENSE](LICENSE) (MIT), for this project's original work only.
- Legal notice and disclaimer: [NOTICE.md](NOTICE.md). Parasite Eve is ©
  Square Enix. This project is not affiliated with or endorsed by Square
  Enix.
