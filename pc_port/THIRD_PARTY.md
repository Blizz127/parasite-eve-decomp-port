# Third-party components of the native port

The port contains no game data; the player supplies the disc. Third-party
material reaches it in three ways:

- PsyCross is fetched at setup time and never committed.
- The khasinski decomp tree (`vendor/`), the SDL_GameControllerDB mapping
  file and the FSR licence notice (`pc_port/third_party/`) are committed.
- The system libraries below are loaded at run time and never bundled.

**Psy-Q classification.** The port is not free of Psy-Q-classified code.
`tools/analysis/psyq_port_guard.py` (sha256
`1d5b80790ba7c2724f757463519f7e2fd4c631b3cbd26a01c63090b449137993`)
classifies 445 functions linked in the port binary as Psy-Q SDK library
functions, by signature, layout or address band. In all it reports 976
findings in 303 places.

- Every finding is on the guard's allowlist, so its default mode passes.
- Strict mode, which ignores the allowlist, fails.
- No Psy-Q table bytes are in the binary.

This was measured on 2026-10-20 on the binary
`c0c298fc0aedba73c5e215fd93b105b32bb530c58dc75526a8bd5717155e186d`, built
from 4ed2d1407. The commands were `python3 -I tools/analysis/psyq_port_guard.py
--binary <bin> --require-exe`, then the same with `--strict`.

These numbers are the guard's classification, not a legal finding. No
licence, waiver or ownership decision for that code is on record. The goal
state, no Psy-Q code at all, is described in
docs/ai_context/PSYQ_PORT_REPLACEMENT.md.

## PsyCross

| field | value |
|---|---|
| upstream | https://github.com/OpenDriver2/PsyCross |
| pin | `e56e4cde1c2b8a15e0d4e38b26cdd9202e0d17e6` (upstream `master` HEAD on 2026-09-28, verified with `git ls-remote`; also the Brave Fencer Musashi pin) |
| licence | MIT, Copyright (c) 2020 REDRIVER2 Project. The full text is `LICENSE` in the fetched checkout. Packaged builds ship it as `LICENSES/PsyCross.txt`. |
| fetch | `scripts/fetch_psycross.sh` puts it in `pc_port/third_party/psycross/` (git-ignored). The script is idempotent and checks the pin and that a LICENSE is present. |
| status | Fetched only. **Not linked into any target yet.** Integration arrives in later steps with `PE_WITH_PSYCROSS` (default OFF), our own CMake glob and build-directory patches. The checkout is never edited. |

### Files that must never be used

PsyCross embeds three Sony libgte tables that are byte-identical to retail
libgte. These tables were checked against the user's EXE (192 / 8192 / 1025
int16):

* `src/gte/sqrt_tbl.h` (SQRT)
* `src/gte/rcossin_tbl.h` (rcossin_tbl)
* `src/gte/ratan_tbl.h` (ratan_tbl)

They are excluded from any build that uses PsyCross. We generate the same
symbols from math in our own code (`pe_plat_gte_tables.c`, step P7). A
disc-time test compares them with the user's EXE ranges listed in
`configs/USA/psyq_data_ranges.tsv`. `tools/analysis/psyq_port_guard.py`
fails if their bytes appear in a port binary.

## AMD FidelityFX Super Resolution 1.0 (adapted kernels)

| field | value |
|---|---|
| upstream | https://github.com/GPUOpen-Effects/FidelityFX-FSR (the public reference EASU and RCAS kernels) |
| licence | MIT, Copyright (c) 2021 Advanced Micro Devices, Inc. The notice is kept in `pc_port/third_party/FidelityFX-FSR-LICENSE.txt`, and packaged builds must ship it beside the other licences. |
| use | A CPU port of the two kernels is compiled into `pc_port/platform/host_upscale.h` as the Display Scaler's FSR 1 mode. That mode is optional and off by default (docs/port/ENHANCEMENTS.md). No AMD source file is fetched or vendored. |

## khasinski/parasite-eve-decomp (vendored)

| field | value |
|---|---|
| upstream | khasinski/parasite-eve-decomp at `3067919e7b82f0334b873117eef07e7157f4ed69`, in `vendor/khasinski-parasite-eve-decomp/` |
| licence | None chosen upstream (`LICENSE.md` there, verbatim). Upstream-authored code stays under its authors' default copyright. |
| permission | `PROVENANCE.md` there records the repository owner's 2026-09-28 authorization to copy with attribution and per-function provenance tags. That is this project's authorization, not a licence from the upstream authors to third parties. |
| use | Sources carrying a `vendor-derived:` header, or a "Partly vendor-derived" comment. In binary `c0c298fc…`, 394 linked functions come from such sources: 392 whole translation units (383 room/overlay and 9 EXE) and 2 partly derived. The count comes from the generated TUs' source banners checked against the binary's defined symbols. Packaged builds list each function in `LICENSES/vendor-derived-functions.txt` and ship the three vendor notices verbatim. |
| open | Whether a binary containing this code may be redistributed beyond the owner's private use. |

## System libraries (not bundled)

The port opens these with `dlopen` at run time. None is shipped, so no
licence text ships for them:

| library | opened by | note |
|---|---|---|
| `libX11.so.6`, `libXrender.so.1` | `platform/host_window.c` | window and scaled blit |
| `libpulse-simple.so.0` | `platform/host_audio.c` | sound; without it the game is silent |
| `libSDL2-2.0.so.0` | `platform/host_pad_sdl.c` | controllers (see below) |

These are needed only once PsyCross is integrated:

| library | licence | note |
|---|---|---|
| SDL2 | zlib | PsyCross window and pad backend |
| OpenAL-soft | LGPL-2.0 | Dynamic link only. The default audio stays in-house (`pe_spu` + `host_audio`). |
| OpenGL | platform | PsyCross GPU backend |

## Controller input (shipped in the preview package)

| component | licence | note |
|---|---|---|
| SDL2 (`libSDL2-2.0.so.0`) | zlib | System library, dlopened at runtime by `platform/host_pad_sdl.c` (GameController + Joystick only, no SDL window). Not bundled; the `/dev/input/js*` path is used when it is missing. |
| SDL_GameControllerDB (`third_party/SDL_GameControllerDB/gamecontrollerdb.txt`) | zlib | Vendored mapping file, installed as `bin/gamecontrollerdb.txt` by `package_preview.sh`. Source commit in that directory's README.md. |
