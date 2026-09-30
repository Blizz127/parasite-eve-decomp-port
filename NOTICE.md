# Notice

## Ownership

Parasite Eve, and all of its code, data, artwork, music, text and
trademarks, are owned by Square Enix. This project is not affiliated with,
endorsed by or sponsored by Square Enix or Sony Interactive Entertainment.
"PlayStation" is a trademark of Sony Interactive Entertainment.

## What this repository contains

- A matching decompilation of Parasite Eve (USA, SLUS-00662): C source that
  compiles back to the retail code, plus the splat configuration and build
  tools.
- A native PC port layer (`pc_port/`) that runs the decompiled game code on
  Linux, and its tests.
- Analysis tools, scripts and documentation.

The MIT license in [LICENSE](LICENSE) applies to this project's original
work only. Function bodies imported from another decompilation are listed
in [UPSTREAM_FILES.md](UPSTREAM_FILES.md). They keep their authors'
copyright.

## What this repository does not contain

- Any game content: no disc images, no files extracted from the disc, and
  no audio, textures, movies, text or other assets.
- Any retail assembly or retail byte tables. The build generates them from
  your own disc.
- Any BIOS image, or bytes from one.
- Any Sony Psy-Q SDK headers, libraries, compiler binaries or decompiled SDK
  code. The SDK functions of the game are left out of `src/`, along with
  the port files that define or translate them.
- Prebuilt binaries, other than the release build on the Releases page.

Every file was checked before publishing:
- the retail-data guard `tools/analysis/retail_data_guard.py --strict`, run
  against the retail EXE, PE.IMG and the known-table signatures, found no
  retail byte runs;
- the Psy-Q guard `tools/analysis/psyq_port_guard.py --strict` found no
  Psy-Q code or tables;
- a scan for secrets and credentials found none.

## Your own disc

To run anything built from this code you need your own legally obtained
copy of Parasite Eve (USA) Disc 1. The port reads all game data from your
disc image at runtime. Nothing here lets you play the game without it.

## Purpose

The decompilation and port are made for preservation, research and
interoperability: keeping the game playable on current hardware for people
who own it.

## Third-party components

Third-party components keep their own licenses. See
[CREDITS.md](CREDITS.md) and [pc_port/THIRD_PARTY.md](pc_port/THIRD_PARTY.md).
