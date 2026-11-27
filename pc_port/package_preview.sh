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
#   parasite-eve-day1-preview/bin/gamecontrollerdb.txt SDL_GameControllerDB (+ -LICENSE.txt)
#
# Usage:
#   PE_BINARY=build/pcbuild-preview/parasite-eve-port ./pc_port/package_preview.sh
#
# Env (all optional):
#   PE_BINARY         native binary (default: build/pcbuild-preview/parasite-eve-port)
#   PE_SOURCE_COMMIT  source commit recorded in build-info (default: git HEAD)
#   PE_SOURCE_NOTE    free-text provenance note recorded in build-info
#   PE_OUTPUT_DIR     output dir (default: build/lanes/preview/dist; must be git-ignored)
#   PE_PREVIEW_DEV_MENU=1|0  REQUIRED. 1: the launcher exports PE_DEV_MENU=1 (Back+Start / F8 dev
#                     menu on; owner checkpoint builds). 0: off (public preview). Unset = refuse to
#                     package (a silent 0 shipped native-only-c062ded9-ctrl with no working dev menu).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# Default (private Day-1 preview) names; PE_PACKAGE_TAG=<tag> (e.g. native-only-d5efe5c0) names the folder
# parasite-eve-<tag> and the archive parasite-eve-<tag>-linux-x64.tar.gz instead (public release packages).
PACKAGE_TAG="${PE_PACKAGE_TAG:-}"
if [[ -n "$PACKAGE_TAG" ]]; then
    [[ "$PACKAGE_TAG" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$ ]] || { echo "ERROR: PE_PACKAGE_TAG must match [A-Za-z0-9][A-Za-z0-9._-]*" >&2; exit 1; }
    NAME="parasite-eve-$PACKAGE_TAG"
else
    NAME="parasite-eve-day1-preview"
fi
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
# SDL_GameControllerDB mappings (zlib), read by the SDL2 pad backend from the exe dir.
install -m 0644 "$ROOT/pc_port/third_party/SDL_GameControllerDB/gamecontrollerdb.txt" "$STAGE/bin/gamecontrollerdb.txt"
install -m 0644 "$ROOT/pc_port/third_party/SDL_GameControllerDB/LICENSE" "$STAGE/bin/gamecontrollerdb-LICENSE.txt"

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
# Dev menu (Back/View + Start/Menu on a controller, F8 on the keyboard): on for
# checkpoint builds packaged with PE_PREVIEW_DEV_MENU=1; PE_DEV_MENU=0 turns it off.
export PE_DEV_MENU="${PE_DEV_MENU:-__DEV_MENU__}"
exec "$HERE/bin/parasite-eve-port" --windowed "${SKIP[@]}" --scale 3 \
    --window-title "Parasite Eve" \
    --disc-image "$DISC" "$@"
LAUNCHER
chmod 0755 "$STAGE/parasite-eve-preview"


VARIANT="${PE_PREVIEW_VARIANT:-current}"
LIMITS_FILE="${PE_PREVIEW_LIMITS:-}"
if [[ -n "$LIMITS_FILE" ]]; then LIMITS="$(cat "$LIMITS_FILE")"; else LIMITS="* See build-info.json for the source commit."; fi
DEFAULT_SKIP="${PE_PREVIEW_DEFAULT_SKIP:-0}"
DEV_MENU="${PE_PREVIEW_DEV_MENU:-}"
[[ "$DEV_MENU" == 0 || "$DEV_MENU" == 1 ]] || die "PE_PREVIEW_DEV_MENU must be 1 (owner/checkpoint build: Back+Start / F8 dev menu on) or 0 (public preview: off); it is ${DEV_MENU:-unset}"
DEV_MENU_WORD=$([[ "$DEV_MENU" == 1 ]] && echo ENABLED || echo DISABLED)
python3 "$ROOT/pc_port/tools/render_preview_readme.py" --template "$ROOT/pc_port/package_readme_preview.txt" \
    --out "$STAGE/README-PREVIEW.txt" --tarball "$TARBALL_NAME" --dir "$NAME" --limits "$LIMITS" \
    --default-skip "$DEFAULT_SKIP" --dev-menu "$DEV_MENU" --tag "$PACKAGE_TAG" --commit "$SOURCE_COMMIT" --note "$SOURCE_NOTE"
chmod 0644 "$STAGE/README-PREVIEW.txt"
python3 - "$STAGE" "$DEFAULT_SKIP" "$DEV_MENU" <<'PY'
import sys, pathlib
stage, skip, devmenu = sys.argv[1:]
l = pathlib.Path(stage, "parasite-eve-preview")
l.write_text(l.read_text().replace("__DEFAULT_SKIP__", skip).replace("__DEV_MENU__", devmenu))
PY
BIN_SHA="$(sha256sum "$STAGE/bin/parasite-eve-port" | cut -d' ' -f1)"
LAUNCHER_SHA="$(sha256sum "$STAGE/parasite-eve-preview" | cut -d' ' -f1)"
BUILD_TIME="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
python3 - "$STAGE/build-info.json" "$SOURCE_COMMIT" "$SOURCE_NOTE" "$BUILD_TIME" "$BIN_SHA" "$LAUNCHER_SHA" "$DEV_MENU" "$NAME" <<'PY'
import json, sys
out, commit, note, t, bsha, lsha, devmenu, name = sys.argv[1:]
info = {
    "schema": 1,
    "name": name,
    "platform": "linux-x64",
    "source_commit": commit,
    "source_note": note,
    "build_time_utc": t,
    "binary": {"path": "bin/parasite-eve-port", "sha256": bsha},
    "launcher": {"path": "parasite-eve-preview", "sha256": lsha},
    "dev_menu_default": int(devmenu),
    "contains_game_data": False,
}
with open(out, "w") as f:
    json.dump(info, f, indent=2)
    f.write("\n")
PY
chmod 0644 "$STAGE/build-info.json"

# Guard: the packaged launcher must carry the chosen dev-menu default and no placeholder.
grep -qF "export PE_DEV_MENU=\"\${PE_DEV_MENU:-$DEV_MENU}\"" "$STAGE/parasite-eve-preview" || die "launcher dev-menu default not applied (wanted $DEV_MENU)"
! grep -rqF "__DEV_MENU" "$STAGE" || die "unreplaced __DEV_MENU placeholder in the stage"
grep -qF "\"dev_menu_default\": $DEV_MENU" "$STAGE/build-info.json" || die "build-info.json lacks dev_menu_default=$DEV_MENU"
# Guard: only the expected payload files (incl. the SDL_GameControllerDB mapping + licence), no disc images, nothing from rom/.
mapfile -t files < <(cd "$WORK" && find "$NAME" -type f | LC_ALL=C sort)
expected=("$NAME/README-PREVIEW.txt" "$NAME/bin/gamecontrollerdb-LICENSE.txt" "$NAME/bin/gamecontrollerdb.txt" "$NAME/bin/parasite-eve-port" "$NAME/build-info.json" "$NAME/parasite-eve-preview")
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
