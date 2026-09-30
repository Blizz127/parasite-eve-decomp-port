#!/usr/bin/env bash
# Build the native PC port (pc_port/) from a clean checkout, end to end.
#
# Why this script exists: `pc_port/game/decomp/` is git-ignored GENERATED
# output (`.gitignore`), and `pc_port/CMakeLists.txt` globs it.  On a clean
# checkout that directory does not exist, so the documented
# `cmake -S pc_port -B pc_port/build` configures 0 decomp-derived TUs and then
# fails at link with undefined references to the leaves those TUs provide
# (func_800614A0, func_800527B4, func_80064A48, func_8005E54C, func_800527C0,
# ...).  The generator must run first, and it must run again whenever matching
# advances, or the generated tree drifts behind src/.
#
# Stages (each one fails loudly):
#   1. generate  — tools/analysis/gen_decomp_ports.py writes pc_port/game/decomp
#                  and gen_overlay_ports.py writes pc_port/game/decomp_ovl
#   2. verify    — the same tool re-checks that every emitted TU matches src/
#   3. configure — cmake -S pc_port -B <build dir>
#   4. build     — cmake --build <build dir>
#   5. test      — ./<build dir>/pe-native-tests
#   guard        — tools/analysis/retail_data_guard.py: no retail disc bytes in
#                  tracked files (runs after stage 2; uses the user's disc via
#                  the git-ignored disc cache when present, else signatures)
#
# Idempotent: re-running regenerates in place, reuses the CMake cache and
# rebuilds only what changed.  Nothing git-tracked is written; the only outputs
# are the two git-ignored trees pc_port/game/decomp/ and the build directory.
#
# Usage:
#   scripts/build_pc_port.sh                 # generate + configure + build + test
#   scripts/build_pc_port.sh --clean         # remove the build dir first
#   scripts/build_pc_port.sh --no-tests      # stop after the build
#   scripts/build_pc_port.sh --build-dir DIR # default: pc_port/build
#   scripts/build_pc_port.sh --no-guard      # skip the retail-data guard
#   scripts/build_pc_port.sh --build-type T  # CMAKE_BUILD_TYPE (default: Release)
#
# Environment:
#   CMAKE   — path to a cmake binary (otherwise resolved from PATH, then from
#             the known fallbacks below)
#   JOBS    — parallel build jobs (default: nproc)
#
# The default build type is Release because this is the binary that ships:
# an unoptimised (empty CMAKE_BUILD_TYPE) build is too slow to run a movie
# in real time (FMV002 at 2560x1600 took 222.6 s for its 68.1 s; see
# docs/port/KNOWN_DIVERGENCES.md, host display row).  Pass --build-type ""
# for the old unoptimised configuration, or Debug for a debugger build.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ ! -f "$ROOT/configs/USA/disc1.yaml" || ! -d "$ROOT/pc_port" ]]; then
    echo "ERROR: $ROOT does not look like the Parasite-Eve-Decompilation root." >&2
    exit 1
fi

BUILD_DIR="$ROOT/pc_port/build"
RUN_TESTS=1
CLEAN=0
RUN_GUARD=1
BUILD_TYPE=Release

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)      CLEAN=1; shift ;;
        --no-tests)   RUN_TESTS=0; shift ;;
        --no-guard)   RUN_GUARD=0; shift ;;
        --build-dir)  BUILD_DIR="$2"; shift 2 ;;
        --build-type) BUILD_TYPE="$2"; shift 2 ;;
        -h|--help)    sed -n '2,45p' "${BASH_SOURCE[0]}"; exit 0 ;;
        *) echo "ERROR: unknown argument '$1' (see --help)." >&2; exit 1 ;;
    esac
done

JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

# ── cmake resolution ─────────────────────────────────────────────────────
# `cmake` is not on PATH on every host that carries this tree.  Resolve it
# explicitly and say exactly what is missing rather than dying inside a
# 'command not found'.  A relocated cmake may also need its own library path.
resolve_cmake() {
    if [[ -n "${CMAKE:-}" ]]; then
        command -v "$CMAKE" >/dev/null 2>&1 && { echo "$CMAKE"; return 0; }
        echo "ERROR: CMAKE='$CMAKE' is not executable." >&2
        return 1
    fi
    if command -v cmake >/dev/null 2>&1; then
        command -v cmake
        return 0
    fi
    local candidate
    for candidate in /usr/bin/cmake /usr/local/bin/cmake /opt/cmake/bin/cmake \
                     /tmp/pe-tools/root/usr/bin/cmake; do
        if [[ -x "$candidate" ]]; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

if ! CMAKE_BIN="$(resolve_cmake)"; then
    cat >&2 <<'EOF'
ERROR: no usable cmake found.

  The native port is configured and built with cmake, and it is not on PATH
  on this host.  Either install it:

      sudo apt-get install -y cmake          # Debian/Ubuntu
      brew install cmake                     # macOS

  or point this script at an existing binary:

      CMAKE=/path/to/cmake scripts/build_pc_port.sh

  If that binary is relocated (not under a system prefix) it may also need
  its own shared libraries, e.g.

      CMAKE=/tmp/pe-tools/root/usr/bin/cmake \
      LD_LIBRARY_PATH=/tmp/pe-tools/root/usr/lib/x86_64-linux-gnu \
          scripts/build_pc_port.sh
EOF
    exit 1
fi

# A relocated cmake usually sits next to its own lib tree (libarchive.so.13 and
# friends); add it so the caller does not have to.
CMAKE_PREFIX="$(cd "$(dirname "$CMAKE_BIN")/.." && pwd)"
for libdir in "$CMAKE_PREFIX/lib/x86_64-linux-gnu" "$CMAKE_PREFIX/lib" \
              "$CMAKE_PREFIX/lib64"; do
    if [[ -d "$libdir" && "$CMAKE_PREFIX" != "/usr" && "$CMAKE_PREFIX" != "/usr/local" ]]; then
        export LD_LIBRARY_PATH="$libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    fi
done

if ! "$CMAKE_BIN" --version >/dev/null 2>&1; then
    echo "ERROR: '$CMAKE_BIN' will not run (missing shared libraries?)." >&2
    echo "       Try: LD_LIBRARY_PATH=<its lib dir> $0" >&2
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "ERROR: python3 is required to generate pc_port/game/decomp." >&2
    exit 1
fi

echo "== pc_port build =="
echo "   root      : $ROOT"
echo "   cmake     : $CMAKE_BIN ($("$CMAKE_BIN" --version | head -1))"
echo "   build dir : $BUILD_DIR"
echo "   jobs      : $JOBS"

# ── 1+2. generate the decomp-derived TUs, then prove the tree is settled ─
# `--verify` re-derives every TU from src/ and exits 1 on any difference, so
# generate-then-verify is the idempotence check.  It can legitimately fail once
# on a tree where another worker is matching leaves concurrently (src/ moved
# between the two passes); that is transient, so retry a bounded number of
# times and only then call it a generator bug.
GEN="$ROOT/tools/analysis/gen_decomp_ports.py"
GEN_OVL="$ROOT/tools/analysis/gen_overlay_ports.py"
settled=0
for attempt in 1 2 3; do
    echo
    echo "-- [1/5] generating decomp-derived port TUs (attempt $attempt)"
    python3 "$GEN"
    # Overlay leaves -> pc_port/game/decomp_ovl/<ovl>/ (namespaced symbols).
    python3 "$GEN_OVL"
    echo
    echo "-- [2/5] verifying generated TUs against src/"
    if python3 "$GEN" --verify && python3 "$GEN_OVL" --verify; then
        settled=1
        break
    fi
    echo "   (src/ moved during the run; regenerating)" >&2
done
if [[ "$settled" -ne 1 ]]; then
    cat >&2 <<'EOF'
ERROR: generated port TUs still drift from src/ after 3 generate+verify passes.
       Either src/ is being rewritten continuously (another worker matching
       leaves — re-run when it settles) or the generator is not idempotent,
       which is a real bug in tools/analysis/gen_decomp_ports.py.
EOF
    exit 1
fi

# ── guard: no retail bytes in tracked files ─────────────────────────────
# Owner decision 2026-09-28: everything from the game is read from the user's
# disc at runtime; nothing retail-derived is committed.  Results are cached per
# blob under build/disc-cache/, so only changed files are rescanned.
if [[ "$RUN_GUARD" -eq 1 ]]; then
    echo
    echo "-- retail-data guard (tracked files)"
    python3 "$ROOT/tools/analysis/retail_data_guard.py"
fi

# ── 3. configure ─────────────────────────────────────────────────────────
if [[ "$CLEAN" -eq 1 ]]; then
    echo
    echo "-- removing $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

echo
echo "-- [3/5] configuring"
"$CMAKE_BIN" -S "$ROOT/pc_port" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# ── 4. build ─────────────────────────────────────────────────────────────
echo
echo "-- [4/5] building"
"$CMAKE_BIN" --build "$BUILD_DIR" -j "$JOBS"

# ── 5. tests ─────────────────────────────────────────────────────────────
if [[ "$RUN_TESTS" -eq 0 ]]; then
    echo
    echo "-- [5/5] skipped (--no-tests)"
    exit 0
fi

TESTS="$BUILD_DIR/pe-native-tests"
if [[ ! -x "$TESTS" ]]; then
    echo "ERROR: $TESTS was not produced by the build." >&2
    exit 1
fi

echo
echo "-- [5/5] running the native suite"
# Disc-gated cases SKIP unless a retail disc is configured (PE_DISC1_BIN or
# local/pe_disc1.path); the suite reports that split in its own summary.
"$TESTS"
