#!/usr/bin/env bash
# Package the "Day 1" release of the Parasite Eve native PC port (linux-x64).
#
# Modeled on package_preview.sh, with three differences: it defaults to
# FULLSCREEN (not windowed), it resolves the disc image from an env var or a
# config file (not only a launcher argument), and it writes version.txt +
# MANIFEST.txt (every packaged file, size + sha256) instead of a single
# build-info.json. Like the preview package, it contains NO game data: the
# player supplies their own Disc 1 image at launch.
#
# The tarball holds:
#   parasite-eve-day1-<version>/bin/parasite-eve-port   native Release binary
#   parasite-eve-day1-<version>/parasite-eve            launcher script
#   parasite-eve-day1-<version>/README.txt              test note + controls
#   parasite-eve-day1-<version>/version.txt              version string
#   parasite-eve-day1-<version>/MANIFEST.txt             file list, size+sha256
#
# Usage:
#   PE_BINARY=build/pcbuild-r3/parasite-eve-port ./pc_port/package_release.sh
#
# Env (all optional):
#   PE_BINARY          native binary (default: build/pcbuild-r3/parasite-eve-port)
#   PE_RELEASE_VERSION version string (default: day1-r3-<short source commit>)
#   PE_SOURCE_COMMIT   source commit recorded in version.txt (default: git HEAD)
#   PE_RELEASE_LIMITS  path to the "test note" text embedded in the README
#                      (default: build/lanes/lead/limits_r3.txt if present)
#   PE_OUTPUT_DIR      output dir (default: build/release; must be git-ignored)
#   PE_SKIP_DATA_SCAN  1 = skip the no-game-data retail-byte scan (debug only)
#   PE_ALLOW_UNOPTIMISED 1 = package a binary whose CMakeCache.txt build type is
#                      not Release/RelWithDebInfo/MinSizeRel (debug only; such a
#                      build cannot play movies in real time)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BINARY="${PE_BINARY:-$ROOT/build/pcbuild-r3/parasite-eve-port}"
OUT_DIR="${PE_OUTPUT_DIR:-$ROOT/build/release}"
SOURCE_COMMIT="${PE_SOURCE_COMMIT:-$(git rev-parse HEAD)}"
SHORT_COMMIT="${SOURCE_COMMIT:0:8}"
VERSION="${PE_RELEASE_VERSION:-day1-r3-$SHORT_COMMIT}"
NAME="parasite-eve-day1-$VERSION"
TARBALL_NAME="$NAME-linux-x64.tar.gz"
LIMITS_FILE="${PE_RELEASE_LIMITS:-$ROOT/build/lanes/lead/limits_r3.txt}"

die() { echo "ERROR: $*" >&2; exit 1; }

[[ -f "$BINARY" && -x "$BINARY" ]] || die "native binary not found: $BINARY
       Build it: cmake -S pc_port -B build/pcbuild-r3 -DCMAKE_BUILD_TYPE=Release && cmake --build build/pcbuild-r3 -j2"
file -b "$BINARY" | grep -q 'ELF 64-bit LSB.*x86-64' || die "not a linux x86-64 ELF: $BINARY"
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{7,40}$ ]] || die "PE_SOURCE_COMMIT must be a hex commit id"
git check-ignore -q "$OUT_DIR" 2>/dev/null || die "output dir is not git-ignored: $OUT_DIR"

# --- build type (the r4b tarball shipped an empty CMAKE_BUILD_TYPE, i.e. an
# unoptimised binary, and FMV playback ran at ~1/3 speed). ------------------
CACHE="$(dirname "$BINARY")/CMakeCache.txt"
if [[ -f "$CACHE" ]]; then
    BTYPE="$(sed -n 's/^CMAKE_BUILD_TYPE:[A-Z]*=//p' "$CACHE")"
    case "$BTYPE" in
        Release|RelWithDebInfo|MinSizeRel) ;;
        *) [[ "${PE_ALLOW_UNOPTIMISED:-0}" == 1 ]] ||
           die "binary was built with CMAKE_BUILD_TYPE='$BTYPE' ($CACHE); rebuild with scripts/build_pc_port.sh (Release by default)" ;;
    esac
fi

# --- library check (Task 3 contract, enforced here so a bad build cannot
# ship): only glibc-family libraries may be required at link time. ---------
LDD_OUT="$(ldd "$BINARY" 2>&1)" || die "ldd failed on $BINARY:\n$LDD_OUT"
while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    case "$line" in
        *linux-vdso.so*|*"/ld-linux"*) continue ;;
        *"libc.so"*) continue ;;
        *) die "unexpected link-time dependency (only glibc/vdso/ld allowed): $line" ;;
    esac
done <<<"$LDD_OUT"

mkdir -p "$OUT_DIR"
WORK="$(mktemp -d "$OUT_DIR/.pe-release-build.XXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
STAGE="$WORK/$NAME"
mkdir -p "$STAGE/bin"

install -m 0755 "$BINARY" "$STAGE/bin/parasite-eve-port"
strip --strip-unneeded "$STAGE/bin/parasite-eve-port" 2>/dev/null || true

cat > "$STAGE/parasite-eve" <<'LAUNCHER'
#!/usr/bin/env bash
# Parasite Eve native port — Day 1 launcher.
#   ./parasite-eve "/path/to/Parasite Eve (USA) (Disc 1).bin" [extra args]
#
# The disc image is found, in order:
#   1. $1 (a .bin, a .cue, or a folder holding them)
#   2. $PE_DISC1
#   3. disc1=<path> in ~/.config/parasite-eve-port/config
#
# Runs FULLSCREEN by default. Set PE_WINDOWED=1 for a windowed run
# (--windowed --scale 3). A copy of everything printed is written to
# ~/.local/state/parasite-eve-port/logs/run-<timestamp>.log (last ~10 kept;
# see README.txt "Bug reports"). Resolves its own directory via
# `readlink -f`, so it works via a symlink and from any cwd; the install
# dir itself is never written to.
set -euo pipefail
# Resolve our own dir even when invoked via a symlink (e.g. a Steam
# shortcut or a launcher entry pointing at this script from elsewhere) or
# from a different cwd.
SELF="${BASH_SOURCE[0]}"
if command -v readlink >/dev/null 2>&1; then SELF="$(readlink -f "$SELF" 2>/dev/null || echo "$SELF")"; fi
HERE="$(cd "$(dirname "$SELF")" && pwd)"
CONFIG="${XDG_CONFIG_HOME:-$HOME/.config}/parasite-eve-port/config"
LOG_DIR="${XDG_STATE_HOME:-$HOME/.local/state}/parasite-eve-port/logs"

DISC="${1:-}"
[[ -n "$DISC" && "$DISC" != -* ]] && shift || DISC=""
if [[ -z "$DISC" ]]; then DISC="${PE_DISC1:-}"; fi
if [[ -z "$DISC" && -f "$CONFIG" ]]; then
    DISC="$(sed -n 's/^[[:space:]]*disc1[[:space:]]*=[[:space:]]*//p' "$CONFIG" | tail -n1)"
fi
if [[ -z "$DISC" ]]; then
    echo "usage: $0 \"/path/to/Parasite Eve (USA) (Disc 1).bin\" [extra args]" >&2
    echo "       or set \$PE_DISC1, or write disc1=<path> to $CONFIG" >&2
    echo "       see README.txt for controls and requirements" >&2
    exit 2
fi
if [[ -d "$DISC" ]]; then
    found="$(find "$DISC" -maxdepth 2 -iname '*disc 1*.bin' -print -quit 2>/dev/null || true)"
    [[ -n "$found" ]] || found="$(find "$DISC" -maxdepth 2 -iname '*.bin' -print -quit 2>/dev/null || true)"
    DISC="$found"
fi
if [[ "$DISC" == *.cue || "$DISC" == *.CUE ]]; then
    bin="$(sed -n 's/^[[:space:]]*FILE[[:space:]]*"\(.*\)".*/\1/p' "$DISC" | head -n1)"
    [[ -n "$bin" ]] && DISC="$(dirname "$DISC")/$bin"
fi
[[ -n "$DISC" && -f "$DISC" ]] || { echo "ERROR: disc image not found: ${DISC:-<none>}" >&2; exit 1; }
size="$(stat -L -c%s "$DISC")"   # -L: the image may be a symlink
if (( size % 2352 != 0 )); then
    echo "ERROR: $DISC is not a raw 2352-byte/sector BIN (size $size)." >&2
    echo "       Use the .bin from a BIN/CUE dump of Disc 1 (not .iso/.chd)." >&2
    exit 1
fi
if [[ -z "${DISPLAY:-}" ]]; then
    echo "ERROR: no X display (DISPLAY is unset). Run from a desktop session;" >&2
    echo "       on Wayland (KDE/GNOME/gamescope) XWayland provides DISPLAY." >&2
    exit 1
fi
cd "$HERE"
MODE=(--fullscreen)
[[ "${PE_WINDOWED:-0}" == 1 ]] && MODE=(--windowed --scale 3)

# Logs go under XDG_STATE_HOME, not next to the binary: the install dir
# may be read-only or replaced on update. Keep the last ~10 runs.
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/run-$(date -u +%Y%m%dT%H%M%SZ).log"
ls -1t "$LOG_DIR"/run-*.log 2>/dev/null | tail -n +10 | xargs -r rm -f -- || true

exec "$HERE/bin/parasite-eve-port" "${MODE[@]}" \
    --window-title "Parasite Eve - Day 1" \
    --disc-image "$DISC" "$@" 2>&1 | tee "$LOG_FILE"
LAUNCHER
chmod 0755 "$STAGE/parasite-eve"

LIMITS="* See version.txt for the source commit."
if [[ -f "$LIMITS_FILE" ]]; then
    # The source note's bug-report line describes an older run.log location
    # (next to the launcher); this package writes logs under XDG_STATE_HOME
    # instead (see "BUG REPORTS" below), so correct that one line rather
    # than ship a factually wrong instruction.
    LIMITS="$(sed 's#log file run\.log next to the launcher (it is written automatically)\.#log file: newest run-<timestamp>.log in ~/.local/state/parasite-eve-port/logs/ (written automatically).#' "$LIMITS_FILE")"
fi

cat > "$STAGE/README.txt" <<README
PARASITE EVE — NATIVE PC PORT — DAY 1 (linux-x64)
==================================================

This is a research build. It contains NO game data: you must supply your
own Parasite Eve (USA) Disc 1 image.

RUN
---
  tar -xzf $TARBALL_NAME
  cd $NAME
  ./parasite-eve "/path/to/Parasite Eve (USA) (Disc 1).bin"

The disc argument may be the Disc 1 .bin, its .cue, or the folder holding
them; it can also be given via \$PE_DISC1 or a \`disc1=<path>\` line in
~/.config/parasite-eve-port/config, so a Steam/launcher shortcut needs no
argument once one of those is set. The image must be a raw BIN
(2352 bytes/sector), e.g. a Redump BIN/CUE dump; .iso and .chd are not
supported.

Runs FULLSCREEN by default (sized to the display, picture letterboxed or
pillarboxed to keep the 4:3 aspect ratio). Set PE_WINDOWED=1 for a
windowed run instead. Extra arguments after the disc path go straight to
the game binary, e.g. PE_WINDOWED=1 ./parasite-eve disc1.bin --scale 4

DISC REQUIREMENT
----------------
The port validates the disc at startup (pe_disc_check) against the known
retail dump:
  Region/disc   USA Disc 1 (SLUS-00662)
  Disc image    495531120 bytes, sha256 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
  Boot EXE      SLUS_006.62, sha1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
A wrong, damaged or unsupported image is rejected with a clear message
instead of crashing during boot.

SAVES
-----
Memory card saves are written OUTSIDE the install directory, to
~/.local/share/parasite-eve-port/memcard1.mcd (override with
\$PE_MEMCARD1). That directory is created automatically on first save, so
nothing needs to be prepared ahead of time and nothing here needs write
access once installed.

TEST NOTE
---------
$LIMITS

CONTROLS                 KEYBOARD                 CONTROLLER (Xbox layout)
--------                 --------                 ------------------------
D-pad / move             Arrow keys               D-pad or left stick
Cross (confirm / act)    Enter, Space, Z or X     A
Circle (cancel)          C                        B
Square                   S                        X
Triangle (menu)          V                        Y
L1 / R1                  Q / E                    LB / RB
L2 / R2                  1 / 3                    LT / RT
Select                   Tab                      Back / View
Start                    P                        Start / Menu
Fast-forward toggle      F6                       L3 (click left stick)
Fast-forward while held  -                        R3 (click right stick)
Quit                     Esc                      -

Keyboard and controller work at the same time. Controllers are read
through the Linux joystick interface (/dev/input/js*) and may be plugged
in or removed while the game runs; up to four are merged. This covers
Xbox pads, Steam Deck / Legion Go built-in controls and anything Steam
Input or Handheld Daemon (HHD) presents as an Xbox controller.

Controller environment variables:
  PE_JOYSTICK=/dev/input/js1   use only this device (default: js0..js7)
  PE_JOYSTICK=off              ignore controllers
  PE_PAD_SWAP_XY=1             swap Square/Triangle (pads whose driver
                               reports X/Y the other way round)
  PE_PAD_DEBUG=1               print raw controller events and the pad word
If no controller is detected, check \`ls -l /dev/input/js*\` exists and is
readable by your user. If Steam is running, it may hide the physical pad
and expose a virtual one instead — either works.

BUG REPORTS
-----------
Please include: the time and room the bug happened in, a screenshot, and
the newest run-<timestamp>.log from
~/.local/state/parasite-eve-port/logs/ (a fresh one is written every time
you play; the last ~10 runs are kept, older ones are cleaned up
automatically).

RUNTIME REQUIREMENTS
---------------------
64-bit Linux with glibc and libX11.so.6 (loaded at runtime; present on
Bazzite / SteamOS / any desktop distro). Sound uses libpulse-simple.so.0
(PipeWire's pulse compatibility provides it; the game is silent, but still
runs, without it). No other libraries needed.
PE_AUDIO_LATENCY_MS=150 raises the audio buffer (default 100 ms) if sound
crackles on a busy system.
README
chmod 0644 "$STAGE/README.txt"

echo "$VERSION" > "$STAGE/version.txt"
chmod 0644 "$STAGE/version.txt"

# --- r2-compatible entry points for the existing Banshee launcher entry ------
# r2 (package_preview.sh) shipped a top-level `parasite-eve-preview "<disc>"`
# launcher and a build-info.json; the launcher's installed entry runs that name.
cat > "$STAGE/parasite-eve-preview" <<'COMPAT'
#!/usr/bin/env bash
# Compatibility entry point (r2 layout): same as ./parasite-eve.
set -euo pipefail
HERE="$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")" && pwd)"
exec "$HERE/parasite-eve" "$@"
COMPAT
chmod 0755 "$STAGE/parasite-eve-preview"
BIN_SHA="$(sha256sum "$STAGE/bin/parasite-eve-port" | cut -d' ' -f1)"
LAUNCH_SHA="$(sha256sum "$STAGE/parasite-eve-preview" | cut -d' ' -f1)"
cat > "$STAGE/build-info.json" <<JSON
{
  "schema": 1,
  "name": "parasite-eve-day1",
  "version": "$VERSION",
  "platform": "linux-x64",
  "source_commit": "$SHORT_COMMIT",
  "binary": { "path": "bin/parasite-eve-port", "sha256": "$BIN_SHA" },
  "launcher": { "path": "parasite-eve-preview", "sha256": "$LAUNCH_SHA" },
  "contains_game_data": false
}
JSON
chmod 0644 "$STAGE/build-info.json"

# --- no-game-data guard: scan every staged file for retail byte runs, the
# same check tools/analysis/retail_data_guard.py runs on tracked sources
# (build/tmp/relscan.py wraps it for an arbitrary directory; we invoke the
# guard directly on the staged file list so it works whether or not a disc
# cache is populated on this box, falling back to signatures-only).
#
# The compiled binary is not a tracked source file, so it is not covered by
# tools/analysis/retail_data_guard_allow.txt's path-based exemptions even
# for tables already reviewed and allowlisted there (e.g. pe_spu_gauss.inc's
# public-domain SPU 4-point gaussian interpolation table, whose low-entropy
# head is a known, lead-approved coincidental collision with PE.IMG bytes,
# not game data). GUARD_SYMBOL_ALLOW lists the *compiled symbol names* for
# exactly those already-allowlisted tables; a hit is only waved through when
# every byte of it falls inside one of those symbols' address range in the
# staged binary, verified via nm — anything else still fails hard. --------
GUARD_SYMBOL_ALLOW=(k_gauss)  # pe_spu_gauss.inc — see retail_data_guard_allow.txt
if [[ "${PE_SKIP_DATA_SCAN:-0}" != 1 ]]; then
    mapfile -t staged_files < <(cd "$STAGE" && find . -type f | sed 's|^\./||' | LC_ALL=C sort)
    scan_paths=()
    for f in "${staged_files[@]}"; do scan_paths+=("$STAGE/$f"); done
    GUARD_JSON="$WORK/guard-hits.json"
    if ! python3 tools/analysis/retail_data_guard.py --quiet-baseline --json "$GUARD_JSON" "${scan_paths[@]}"; then
        # retail_data_guard.py reports paths relative to the repo ROOT (see
        # its iter_files()), not the absolute paths we passed it. Symbol
        # lookup must use the PRE-STRIP binary ($BINARY): "strip
        # --strip-unneeded" on the staged copy drops local/static symbols
        # like k_gauss (it is `static const`), so nm on the staged, stripped
        # binary would find nothing there — but stripping only removes the
        # symbol table, not section contents, so file offsets/addresses in
        # .rodata are identical between the two.
        REL_BINPATH="$(python3 -c 'import os,sys; print(os.path.relpath(sys.argv[1], sys.argv[2]))' "$STAGE/bin/parasite-eve-port" "$ROOT")"
        python3 - "$GUARD_JSON" "$BINARY" "$REL_BINPATH" "${GUARD_SYMBOL_ALLOW[@]}" <<'PY' || die "retail_data_guard.py found retail byte runs in the staged package that are not covered by GUARD_SYMBOL_ALLOW (see above) — refusing to package game data"
import json, re, subprocess, sys
hits_path, binpath, rel_binpath, *allow_names = sys.argv[1:]
report = json.loads(open(hits_path).read())
# Mirror the tool's own pass/fail split: only non-"address-table" classed
# hits are failures (we never pass --strict), the rest are informational.
hits = [h for h in report if h.get("class") != "address-table"]
if not hits:
    sys.exit(0)  # nothing that actually failed the guard

def parse_addr(v):
    if isinstance(v, int):
        return v
    m = re.search(r"0x([0-9A-Fa-f]+)", str(v))
    if not m:
        return None
    return int(m.group(1), 16)

nm = subprocess.run(["nm", "-n", binpath], capture_output=True, text=True).stdout
syms = []
for line in nm.splitlines():
    parts = line.split()
    if len(parts) >= 3 and all(c in "0123456789abcdef" for c in parts[0]):
        syms.append((int(parts[0], 16), parts[-1]))
syms.sort()

def symbol_at(addr):
    if addr is None:
        return None
    name = None
    for a, n in syms:
        if a <= addr: name = n
        else: break
    return name

bad = []
for h in hits:
    if h["file"] != rel_binpath:
        bad.append(h); continue
    a, b = parse_addr(h["from"]), parse_addr(h["to"])
    name_a, name_b = symbol_at(a), symbol_at(b)
    if a is None or name_a not in allow_names or name_a != name_b:
        bad.append(h)
for h in bad:
    a = parse_addr(h["from"])
    print(f"[guard] NOT covered by GUARD_SYMBOL_ALLOW: {h['file']}:{h['from']}-{h['to']} "
          f"({h['bytes']} bytes, symbol={symbol_at(a)})", file=sys.stderr)
if bad:
    sys.exit(1)
print(f"[guard] {len(hits)} hit(s) all fall inside allowlisted symbol(s) {allow_names} "
      f"(already lead-approved for pe_spu_gauss.inc's source form — see "
      f"tools/analysis/retail_data_guard_allow.txt) — treating as OK", file=sys.stderr)
sys.exit(0)
PY
    fi
fi

# Guard: only the five expected payload files, no disc images, nothing from rom/.
mapfile -t files < <(cd "$WORK" && find "$NAME" -type f | LC_ALL=C sort)
expected=("$NAME/MANIFEST.txt" "$NAME/README.txt" "$NAME/bin/parasite-eve-port" "$NAME/build-info.json" "$NAME/parasite-eve" "$NAME/parasite-eve-preview" "$NAME/version.txt")
# MANIFEST.txt is generated last (it lists itself), so exclude it from this
# pre-manifest listing and check again after it is written.
pre_expected=("$NAME/README.txt" "$NAME/bin/parasite-eve-port" "$NAME/build-info.json" "$NAME/parasite-eve" "$NAME/parasite-eve-preview" "$NAME/version.txt")
[[ "${files[*]}" == "${pre_expected[*]}" ]] || die "unexpected staged files: ${files[*]}"
for f in "${files[@]}"; do
    case "${f,,}" in
        *.bin|*.cue|*.iso|*.img|*.chd|*.ccd|*.mdf|*.ecm|*.pbp|*.str|*.xa|*/rom/*) die "refusing to package game data: $f" ;;
    esac
done
bsize="$(stat -c%s "$STAGE/bin/parasite-eve-port")"
(( bsize < 64 * 1024 * 1024 )) || die "binary unexpectedly large ($bsize bytes)"

# --- MANIFEST.txt: every packaged file, size + sha256 (written last so it
# can list every other file; it is added to the tarball too). -------------
: > "$STAGE/MANIFEST.txt"
for f in "${files[@]}"; do
    rel="${f#"$NAME"/}"
    sz="$(stat -c%s "$WORK/$f")"
    sha="$(sha256sum "$WORK/$f" | cut -d' ' -f1)"
    printf '%s  %d  %s\n' "$sha" "$sz" "$rel" >> "$STAGE/MANIFEST.txt"
done
chmod 0644 "$STAGE/MANIFEST.txt"

mapfile -t files < <(cd "$WORK" && find "$NAME" -type f | LC_ALL=C sort)
[[ "${files[*]}" == "${expected[*]}" ]] || die "unexpected final staged files: ${files[*]}"

TARBALL="$OUT_DIR/$TARBALL_NAME"
tar --owner=0 --group=0 --numeric-owner --sort=name -C "$WORK" -czf "$WORK/$TARBALL_NAME" "$NAME"
mv -f "$WORK/$TARBALL_NAME" "$TARBALL"
TAR_SHA="$(sha256sum "$TARBALL" | cut -d' ' -f1)"
echo "$TAR_SHA  $TARBALL_NAME" > "$TARBALL.sha256"
echo "==> $TARBALL"
echo "    sha256 $TAR_SHA"
echo "    version $VERSION (commit $SOURCE_COMMIT)"
tar -tzvf "$TARBALL"
