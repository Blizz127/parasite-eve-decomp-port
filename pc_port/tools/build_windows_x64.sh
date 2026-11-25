#!/usr/bin/env bash
# Cross-build parasite-eve-port.exe (x86_64-windows-gnu, MinGW-w64 ABI/UCRT)
# from this tree with zig cc.  Host: Linux.  No MinGW gcc, no downloads: zig
# ships the MinGW-w64 headers and import libraries.
#
#   pc_port/tools/build_windows_x64.sh            # configure + build
#   BUILD_DIR=... JOBS=4 pc_port/tools/build_windows_x64.sh
#
# Toolchain files: pc_port/cmake/zig-windows-x86_64.cmake (zig as C compiler,
# ar, ranlib) and pc_port/cmake/windows.cmake (Win32 window backend
# host_window_win32.c, WinMM audio host_audio_winmm.c, XInput pad; the SDL
# backend is not wired for Windows).  docs/port/WINDOWS.md.
#
# Idempotent: re-running reconfigures (cheap) and rebuilds incrementally.
# Requires the git-ignored generated trees pc_port/game/decomp/ and
# pc_port/game/decomp_ovl/ to be present (this script never generates or
# edits them; it fails loudly if they are missing).
#
# Env (all optional):
#   PE_ZIG       zig executable            (default: ~/.local/bin/zig, else PATH)
#   CMAKE        cmake executable          (default: ~/.local/bin/cmake, else PATH)
#   BUILD_DIR    build tree                (default: pc_port/build-win; git-ignored)
#   JOBS         parallel jobs             (default: 4)
#   NICE         nice level, "" = none     (default: 19)
#   BUILD_TYPE   CMake build type          (default: Release)
#   BUILD_TESTS  ON|OFF                    (default: OFF; ON also builds the Win32 test exes)
#   ZIG_CACHE    zig cache root            (default: $BUILD_DIR/zig-cache)
#   BUILD_TIMEOUT seconds for the build    (default: 5400)
# Output: $BUILD_DIR/parasite-eve-port.exe and a "BUILD_WINDOWS_X64 PASS ..." line.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
PC="$ROOT/pc_port"

die() { echo "BUILD_WINDOWS_X64 FAIL: $*" >&2; exit 1; }

pick() { # pick <env value> <local path> <name>
    if [[ -n "$1" ]]; then echo "$1"
    elif [[ -x "$2" ]]; then echo "$2"
    else command -v "$3" || true; fi
}
PE_ZIG="$(pick "${PE_ZIG:-}" "$HOME/.local/bin/zig" zig)"
CMAKE="$(pick "${CMAKE:-}" "$HOME/.local/bin/cmake" cmake)"
[[ -n "$PE_ZIG" && -x "$PE_ZIG" ]] || die "zig not found (set PE_ZIG)"
[[ -n "$CMAKE" && -x "$CMAKE" ]] || die "cmake not found (set CMAKE)"

BUILD_DIR="${BUILD_DIR:-$PC/build-win}"
JOBS="${JOBS:-4}"
NICE="${NICE-19}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_TESTS="${BUILD_TESTS:-OFF}"
BUILD_TIMEOUT="${BUILD_TIMEOUT:-5400}"
export ZIG_GLOBAL_CACHE_DIR="${ZIG_CACHE:-$BUILD_DIR/zig-cache}/global"
export ZIG_LOCAL_CACHE_DIR="${ZIG_CACHE:-$BUILD_DIR/zig-cache}/local"
mkdir -p "$BUILD_DIR" "$ZIG_GLOBAL_CACHE_DIR" "$ZIG_LOCAL_CACHE_DIR"
# Keep temp files off a quota-limited /tmp tmpfs.
export TMPDIR="${TMPDIR:-$BUILD_DIR/tmp}"
mkdir -p "$TMPDIR"

for d in game/decomp game/decomp_ovl; do
    [[ -d "$PC/$d" && -n "$(find "$PC/$d" -name '*.c' -print -quit)" ]] \
        || die "generated tree pc_port/$d/ is missing or empty (git-ignored; copy it from a generated worktree, e.g. rsync -a <wt>/pc_port/$d/ pc_port/$d/)"
done

GEN=()
if command -v ninja >/dev/null 2>&1 || [[ -x "$HOME/.local/bin/ninja" ]]; then
    export PATH="$HOME/.local/bin:$PATH"
    GEN=(-G Ninja)
fi
# Do not mix generators in one build dir.
if [[ -f "$BUILD_DIR/CMakeCache.txt" && ${#GEN[@]} -eq 0 ]]; then GEN=(); fi

PRE=()
[[ -n "$NICE" ]] && PRE=(nice -n "$NICE")

echo "==> configure ($BUILD_DIR, $BUILD_TYPE, zig $("$PE_ZIG" version))"
"${PRE[@]}" "$CMAKE" -S "$PC" -B "$BUILD_DIR" "${GEN[@]}" \
    -DCMAKE_TOOLCHAIN_FILE="$PC/cmake/zig-windows-x86_64.cmake" \
    -DPE_ZIG="$PE_ZIG" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTS="$BUILD_TESTS" \
    -DPE_PORT_SANITIZERS=OFF \
    -DPE_PSYQ_GUARD_STRICT=OFF

echo "==> build parasite-eve-port (-j$JOBS, timeout ${BUILD_TIMEOUT}s)"
target=(--target parasite-eve-port)
if [[ "$BUILD_TESTS" == ON && "${BUILD_WIN_TEST_TARGETS:-1}" == 1 ]]; then target+=(--target pe-win32-input-tests); fi
timeout "$BUILD_TIMEOUT" "${PRE[@]}" "$CMAKE" --build "$BUILD_DIR" -j"$JOBS" "${target[@]}"

EXE="$BUILD_DIR/parasite-eve-port.exe"
[[ -f "$EXE" ]] || die "no $EXE after build"
if command -v file >/dev/null 2>&1; then
    file -b "$EXE" | grep -q 'PE32+ executable.*x86-64' || die "not a PE32+ x86-64 executable: $(file -b "$EXE")"
fi
echo "BUILD_WINDOWS_X64 PASS $(sha256sum "$EXE" | cut -d' ' -f1)  $(stat -c%s "$EXE") bytes  $EXE"
