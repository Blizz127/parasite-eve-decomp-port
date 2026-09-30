# Third-party components of the native port

The port ships only our own code plus open-source dependencies with
compatible licences. Nothing from the game and no Sony Psy-Q SDK code or
tables (docs/ai_context/PSYQ_PORT_REPLACEMENT.md). Third-party sources are
fetched at setup time and never committed.

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

## System libraries (not bundled)

These are needed only once PsyCross is integrated:

| library | licence | note |
|---|---|---|
| SDL2 | zlib | PsyCross window and pad backend |
| OpenAL-soft | LGPL-2.0 | Dynamic link only. The default audio stays in-house (`pe_spu` + `host_audio`). |
| OpenGL | platform | PsyCross GPU backend |
