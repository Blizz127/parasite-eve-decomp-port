#!/usr/bin/env bash
# Package the "Day 1 preview" of the Parasite Eve native PC port (linux-x64).
#
# Unlike package_runtime.sh this contains NO game data: the player supplies
# their own Disc 1 image at launch.  The tarball holds exactly:
#
#   parasite-eve-day1-preview/bin/parasite-eve-port   native Release binary
#   parasite-eve-day1-preview/parasite-eve-preview    launcher script
#   parasite-eve-day1-preview/README-PREVIEW.txt      controls + known limits
#   parasite-eve-day1-preview/build-info.json         commit, time, sha256
#
# Usage:
#   PE_BINARY=build/pcbuild-preview/parasite-eve-port ./pc_port/package_preview.sh
#
# Env (all optional):
#   PE_BINARY         native binary (default: build/pcbuild-preview/parasite-eve-port)
#   PE_SOURCE_COMMIT  source commit recorded in build-info (default: git HEAD)
#   PE_SOURCE_NOTE    free-text provenance note recorded in build-info
#   PE_OUTPUT_DIR     output dir (default: build/lanes/preview/dist; must be git-ignored)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

NAME="parasite-eve-day1-preview"
TARBALL_NAME="$NAME-linux-x64.tar.gz"
BINARY="${PE_BINARY:-$ROOT/build/pcbuild-preview/parasite-eve-port}"
OUT_DIR="${PE_OUTPUT_DIR:-$ROOT/build/lanes/preview/dist}"
SOURCE_COMMIT="${PE_SOURCE_COMMIT:-$(git rev-parse HEAD)}"
SOURCE_NOTE="${PE_SOURCE_NOTE:-}"

die() { echo "ERROR: $*" >&2; exit 1; }

[[ -f "$BINARY" && -x "$BINARY" ]] || die "native binary not found: $BINARY
       Build it: cmake -S pc_port -B build/pcbuild-preview -DCMAKE_BUILD_TYPE=Release && cmake --build build/pcbuild-preview -j8"
file -b "$BINARY" | grep -q 'ELF 64-bit LSB.*x86-64' || die "not a linux x86-64 ELF: $BINARY"
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{7,40}$ ]] || die "PE_SOURCE_COMMIT must be a hex commit id"
git check-ignore -q "$OUT_DIR" 2>/dev/null || die "output dir is not git-ignored: $OUT_DIR"

mkdir -p "$OUT_DIR"
WORK="$(mktemp -d "$OUT_DIR/.pe-preview-build.XXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
STAGE="$WORK/$NAME"
mkdir -p "$STAGE/bin"

install -m 0755 "$BINARY" "$STAGE/bin/parasite-eve-port"
strip --strip-unneeded "$STAGE/bin/parasite-eve-port" 2>/dev/null || true

cat > "$STAGE/parasite-eve-preview" <<'LAUNCHER'
#!/usr/bin/env bash
# Parasite Eve native port — Day 1 preview launcher.
#   ./parasite-eve-preview "/path/to/Parasite Eve (USA) (Disc 1).bin" [extra args]
# Extra args are passed to the binary (e.g. --scale 4).
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ $# -lt 1 || "$1" == "-h" || "$1" == "--help" ]]; then
    echo "usage: $0 \"/path/to/Parasite Eve (USA) (Disc 1).bin\" [extra args]" >&2
    echo "       see README-PREVIEW.txt for controls" >&2
    exit 2
fi
DISC="$1"; shift
if [[ -d "$DISC" ]]; then
    found="$(find "$DISC" -maxdepth 2 -iname '*disc 1*.bin' -print -quit 2>/dev/null || true)"
    [[ -n "$found" ]] || found="$(find "$DISC" -maxdepth 2 -iname '*.bin' -print -quit 2>/dev/null || true)"
    DISC="$found"
fi
if [[ "$DISC" == *.cue || "$DISC" == *.CUE ]]; then
    bin="$(sed -n 's/^[[:space:]]*FILE[[:space:]]*"\(.*\)".*/\1/p' "$DISC" | head -n1)"
    [[ -n "$bin" ]] && DISC="$(dirname "$DISC")/$bin"
fi
[[ -n "$DISC" && -f "$DISC" ]] || { echo "ERROR: disc image not found: ${DISC:-$1}" >&2; exit 1; }
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
SKIP=()
[[ "${PE_SKIP_MOVIE:-__DEFAULT_SKIP__}" == 1 ]] && SKIP=(--skip-movie)
exec "$HERE/bin/parasite-eve-port" --windowed "${SKIP[@]}" --scale 3 \
    --window-title "Parasite Eve - Day 1 preview" \
    --disc-image "$DISC" "$@"
LAUNCHER
chmod 0755 "$STAGE/parasite-eve-preview"

cat > "$STAGE/README-PREVIEW.txt" <<'README'
PARASITE EVE — NATIVE PC PORT — DAY 1 PREVIEW (linux-x64)
==========================================================

This is an early, unfinished research build. It contains NO game data:
you must supply your own Parasite Eve (USA) Disc 1 image.

RUN
---
  tar -xzf parasite-eve-day1-preview-linux-x64.tar.gz
  cd parasite-eve-day1-preview
  ./parasite-eve-preview "/path/to/Parasite Eve (USA) (Disc 1).bin"

The disc argument may be the Disc 1 .bin, its .cue, or the folder holding
them. The image must be a raw BIN (2352 bytes/sector), e.g. a Redump
BIN/CUE dump; .iso and .chd are not supported. Extra arguments go to the
game binary, e.g.  ./parasite-eve-preview disc1.bin --scale 4

It opens a window (X11; on Wayland desktops and Steam Game Mode, XWayland
is used automatically). Drag to resize; the 4:3 picture is kept. Esc quits.

The preview starts a NEW GAME directly: the Square logo, opening movie and
title screen are skipped (the movie path is not finished yet).

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
through the Linux joystick interface (/dev/input/js*) and may be plugged in
or removed while the game runs; up to four are merged. This covers Xbox
pads, Steam Deck / Legion Go built-in controls and anything Steam Input or
Handheld Daemon (HHD) presents as an Xbox controller.

Fast-forward runs the game uncapped and mutes the sound (it fades out and
back in, so there is no chipmunk audio or static); the window title shows
the active speed.

Controller environment variables:
  PE_JOYSTICK=/dev/input/js1   use only this device (default: js0..js7)
  PE_JOYSTICK=off              ignore controllers
  PE_PAD_SWAP_XY=1             swap Square/Triangle (pads whose driver
                               reports X/Y the other way round)
  PE_PAD_DEBUG=1               print raw controller events and the pad word
e.g.  PE_PAD_DEBUG=1 ./parasite-eve-preview disc1.bin
If no controller is detected, check `ls -l /dev/input/js*` exists and is
readable by your user. If Steam is running, it may hide the physical pad and
expose a virtual one instead — either works.

KNOWN LIMITS
------------
__LIMITS__

RUNTIME REQUIREMENTS
--------------------
64-bit Linux with glibc and libX11.so.6 (loaded at runtime; present on
Bazzite / SteamOS / any desktop distro). Sound uses libpulse-simple.so.0
(PipeWire's pulse compatibility provides it; the game is silent without
it). No other libraries needed. PE_AUDIO_LATENCY_MS=150 raises the audio
buffer (default 100 ms) if sound crackles on a busy system.
README
chmod 0644 "$STAGE/README-PREVIEW.txt"

VARIANT="${PE_PREVIEW_VARIANT:-current}"
LIMITS_FILE="${PE_PREVIEW_LIMITS:-}"
if [[ -n "$LIMITS_FILE" ]]; then LIMITS="$(cat "$LIMITS_FILE")"; else LIMITS="* See build-info.json for the source commit."; fi
DEFAULT_SKIP="${PE_PREVIEW_DEFAULT_SKIP:-0}"
python3 - "$STAGE" "$LIMITS" "$DEFAULT_SKIP" <<'PY'
import sys, pathlib
stage, limits, skip = sys.argv[1:]
r = pathlib.Path(stage, "README-PREVIEW.txt"); r.write_text(r.read_text().replace("__LIMITS__", limits))
l = pathlib.Path(stage, "parasite-eve-preview"); l.write_text(l.read_text().replace("__DEFAULT_SKIP__", skip))
PY
BIN_SHA="$(sha256sum "$STAGE/bin/parasite-eve-port" | cut -d' ' -f1)"
LAUNCHER_SHA="$(sha256sum "$STAGE/parasite-eve-preview" | cut -d' ' -f1)"
BUILD_TIME="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
python3 - "$STAGE/build-info.json" "$SOURCE_COMMIT" "$SOURCE_NOTE" "$BUILD_TIME" "$BIN_SHA" "$LAUNCHER_SHA" <<'PY'
import json, sys
out, commit, note, t, bsha, lsha = sys.argv[1:]
info = {
    "schema": 1,
    "name": "parasite-eve-day1-preview",
    "platform": "linux-x64",
    "source_commit": commit,
    "source_note": note,
    "build_time_utc": t,
    "binary": {"path": "bin/parasite-eve-port", "sha256": bsha},
    "launcher": {"path": "parasite-eve-preview", "sha256": lsha},
    "contains_game_data": False,
}
with open(out, "w") as f:
    json.dump(info, f, indent=2)
    f.write("\n")
PY
chmod 0644 "$STAGE/build-info.json"

# Guard: only the four expected payload files, no disc images, nothing from rom/.
mapfile -t files < <(cd "$WORK" && find "$NAME" -type f | LC_ALL=C sort)
expected=("$NAME/README-PREVIEW.txt" "$NAME/bin/parasite-eve-port" "$NAME/build-info.json" "$NAME/parasite-eve-preview")
[[ "${files[*]}" == "${expected[*]}" ]] || die "unexpected staged files: ${files[*]}"
for f in "${files[@]}"; do
    case "${f,,}" in
        *.bin|*.cue|*.iso|*.img|*.chd|*.ccd|*.mdf|*.ecm|*.pbp|*.str|*.xa|*/rom/*) die "refusing to package game data: $f" ;;
    esac
done
bsize="$(stat -c%s "$STAGE/bin/parasite-eve-port")"
(( bsize < 64 * 1024 * 1024 )) || die "binary unexpectedly large ($bsize bytes)"

TARBALL="$OUT_DIR/$TARBALL_NAME"
tar --owner=0 --group=0 --numeric-owner --sort=name -C "$WORK" -czf "$WORK/$TARBALL_NAME" "$NAME"
mv -f "$WORK/$TARBALL_NAME" "$TARBALL"
TAR_SHA="$(sha256sum "$TARBALL" | cut -d' ' -f1)"
echo "$TAR_SHA  $TARBALL_NAME" > "$TARBALL.sha256"
echo "==> $TARBALL"
echo "    sha256 $TAR_SHA"
echo "    binary sha256 $BIN_SHA (commit $SOURCE_COMMIT)"
tar -tzvf "$TARBALL"
