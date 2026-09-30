# Windows 10/11 native build and run guide

The native port compiles and runs on 64-bit Windows 10/11 with no third-party
dependencies. The only platform-specific file is the window backend:
`platform/host_window_win32.c` (Win32 GDI) replaces `platform/host_window.c`
(X11) automatically when CMake detects Windows. All translated game code,
guest RAM, disc access, and tests are shared with the Linux build.

## Requirements

- Windows 10 (1809+) or 11, x64.
- One toolchain:
  - **Visual Studio 2022** (17.x) with the *Desktop development with C++*
    workload (includes MSVC and CMake), **or**
  - **Visual Studio Build Tools 2022** with the same workload (no IDE), **or**
  - **MinGW-w64** GCC (alternative; see below).
- `cmake` 3.16+ (bundled with VS; otherwise from https://cmake.org).
- A Parasite Eve (USA) Disc 1 image (`.bin` + sidecar `.cue`) you dumped
  yourself. It is opened read-only at runtime and is never committed.

## Build (Visual Studio 2022, recommended)

From a *Developer PowerShell* (or any prompt with `cmake` on PATH),
in the repository root:

```powershell
cmake -S pc_port -B pc_port\build -G "Visual Studio 17 2022" -A x64
cmake --build pc_port\build --config Release
```

Binaries land in `pc_port\build\Release\`:

- `parasite-eve-port.exe` — the game.
- `pe-native-tests.exe` — the native test suite.
- `pe-field-runtime-link-test.exe` — runtime link check.

## Tests

Run from the repository root (the tests expect it as working directory):

```powershell
.\pc_port\build\Release\pe-native-tests.exe
.\pc_port\build\Release\pe-field-runtime-link-test.exe
```

Or via CTest:

```powershell
ctest --test-dir pc_port\build -C Release
```

`PE_TEST_FILTER` and `PE_DISC1_BIN` work as on Linux
(`$env:PE_TEST_FILTER = "OTC1"`). The one environment-gated test that needs
a local disc image fails the same way it does on Linux without one.

## Run

```powershell
# Headless deterministic boot (no window)
.\pc_port\build\Release\parasite-eve-port.exe --headless --bootstrap-disc `
  --max-frames 1 `
  --screenshot $env:TEMP\pe-black.ppm --trace $env:TEMP\pe-boot.trace

# Real Disc 1 boot (image opened read-only, never copied)
.\pc_port\build\Release\parasite-eve-port.exe --headless `
  --disc-image "D:\dumps\Parasite Eve (USA) (Disc 1).bin" `
  --max-frames 1 `
  --screenshot $env:TEMP\pe-black.ppm --trace $env:TEMP\pe-boot.trace

# Windowed (default scale 2 → 640x480; --scale 3 → 960x720)
.\pc_port\build\Release\parasite-eve-port.exe --bootstrap-disc --hold-ms 5000 --debug-overlay
.\pc_port\build\Release\parasite-eve-port.exe `
  --disc-image "D:\dumps\Parasite Eve (USA) (Disc 1).bin" --scale 3
```

The game runs exactly as far as the Linux build: same translated code, same
stub frontier, same framebuffer content. Windowed mode opens a
`Parasite Eve Native Port` window showing the 320x240 framebuffer.

## Controls (windowed)

| Key | PSX pad |
| --- | ------- |
| Enter / Space / Z / X | Cross |
| Arrow keys | D-pad Up / Right / Down / Left |
| Escape, or closing the window | Quit |

## Notes and limitations

- The port is little-endian x86-64 code with no VLAs, no GCC extensions, and
  no POSIX-only calls outside the X11 backend file (which Windows does not
  compile). MSVC builds in C11 mode (`/std:clatest` default is fine).
- Disc images stay user-supplied under any local path; `--disc-image`
  accepts both `\` and `/` separators.
- AddressSanitizer flags are GCC/Clang-only in `CMakeLists.txt`; MSVC builds
  simply skip them (`-DPE_PORT_SANITIZERS=ON` is a no-op on MSVC).
- The era-gated matching-decomp rebuild (`scripts/build_us.sh`, docker
  `pe-mipsel-img`) is Linux-only and unaffected by this port work; no `src/`
  matching leaves are involved.

## Build (MinGW-w64, alternative)

```powershell
cmake -S pc_port -B pc_port\build-mingw -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build pc_port\build-mingw
```

Binaries land in `pc_port\build-mingw\`. The window backend links stock
`gdi32`/`user32` — no extra `-l` flags needed.

## Troubleshooting

- If window creation fails, `HostWindow_Open` prints the Win32 error code and
  the game continues without a window: translated code still runs and
  `--screenshot` is still written.
- Always pass `--screenshot` explicitly: the built-in default path
  (`/tmp/pe-port-black.ppm`) rarely exists on Windows, and a failed
  screenshot write is silent.
- Screenshots are binary PPM (`--screenshot`) written with `"wb"`; they are
  byte-identical to Linux output for the same run flags and seed fixture.
- Non-ASCII paths (e.g. `Parasite Eve (USA)...`) work: the port passes paths
  straight to `fopen` with no ANSI/OEM translation of its own.
