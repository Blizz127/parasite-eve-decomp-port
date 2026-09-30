# Ubuntu Linux native build and run guide

Verified on Ubuntu 26.04 LTS (x86-64) with GCC 15.2 + CMake 4.2.3, from a
clean out-of-tree build. Older Ubuntu LTS releases (20.04+) work the same;
any GCC ≥ 11 and CMake ≥ 3.16 satisfy the build.

## Requirements

```bash
sudo apt install build-essential cmake
```

That is everything for headless builds and the test suite. For **windowed**
mode you also need the X11 client library at *runtime* (loaded via `dlopen`,
no dev headers needed — it ships with most desktops):

```bash
sudo apt install libx11-6
```

Without it, windowed runs print `[WINDOW] libX11.so.6 not found` and continue
without a window; headless runs are unaffected.

You also need your own Parasite Eve (USA) Disc 1 image (`.bin` + sidecar
`.cue`) for real-disc runs. It is opened read-only and never committed.

## Build

```bash
cmake -S pc_port -B pc_port/build
cmake --build pc_port/build
```

Binaries land in `pc_port/build/`:

- `parasite-eve-port` — the game.
- `pe-native-tests` — the native test suite.
- `pe-field-runtime-link-test` — runtime link check.

## Tests

```bash
ctest --test-dir pc_port/build
# or directly (from the repo root — the tests expect it as working dir):
./pc_port/build/pe-native-tests
```

Expected result: everything passes except `B54KY_192CE8`, which requires a
local disc image (`missing local/pe_disc1.path`) and fails without one.
Point `PE_DISC1_BIN` at your image to run it too:

```bash
PE_DISC1_BIN="/path/Parasite Eve (USA) (Disc 1).bin" ./pc_port/build/pe-native-tests
```

`PE_TEST_FILTER` selects a test family (e.g. `PE_TEST_FILTER=OTC1`).

## Run

```bash
# Headless deterministic boot (no window needed)
./pc_port/build/parasite-eve-port --headless --bootstrap-disc \
  --max-frames 1 \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Real Disc 1 boot (image opened read-only, never copied)
./pc_port/build/parasite-eve-port --headless \
  --disc-image "/path/Parasite Eve (USA) (Disc 1).bin" \
  --max-frames 1 \
  --screenshot /tmp/pe-black.ppm --trace /tmp/pe-boot.trace

# Windowed (needs an X display; default scale 2 → 640x480)
./pc_port/build/parasite-eve-port --bootstrap-disc --hold-ms 5000 --debug-overlay
```

A successful headless boot exits 0 and writes a 320x240 binary PPM
(`P6`, 230415 bytes). The game runs as far as the current translated-code
frontier (see `pc_port/README.md`); `--strict-stubs` reports the exact
frontier function on stderr.

## Controls (windowed)

| Key | PSX pad |
| --- | ------- |
| Enter / Space / Z / X | Cross |
| Arrow keys | D-pad Up / Right / Down / Left |
| Escape, or closing the window | Quit |

## Troubleshooting

- `ctest` step `native-tests` reports failed with only
  `B54KY_192CE8 ... FAIL: missing local/pe_disc1.path` → normal without a
  disc image; supply `PE_DISC1_BIN` (see above).
- `[WINDOW] libX11.so.6 not found` → `sudo apt install libx11-6`, or run
  with `--headless`.
- Screenshots are byte-identical across machines for the same run flags and
  seed fixture; compare with `cmp` against a known-good PPM.
