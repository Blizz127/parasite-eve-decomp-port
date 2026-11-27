#!/usr/bin/env bash
# Package the Windows x64 build of the Parasite Eve native PC port as a
# private-preview zip.  NO game data: the player supplies their own Disc 1.
#
#   pc_port/tools/package_windows_x64.sh <exe> <tag> <outdir>
#
# Produces <outdir>/parasite-eve-<tag>-windows-x64.zip (+ .sha256) with one
# top-level directory parasite-eve-<tag>-windows-x64/:
#
#   bin/parasite-eve-port.exe             the cross-built PE32+ binary
#   bin/gamecontrollerdb.txt              SDL_GameControllerDB mappings (zlib)
#   bin/gamecontrollerdb-LICENSE.txt
#   parasite-eve.cmd                      launcher: Disc 1 from disc\ or PE_DISC
#   README-WINDOWS.txt                    bring-your-own-disc, controls, limits
#   LICENSES/FidelityFX-FSR-LICENSE.txt   MIT, AMD (FSR 1 kernels are compiled in)
#   LICENSES/SDL_GameControllerDB-LICENSE.txt
#   build-info.json                       commit, time, exe sha256, no game data
#
# Mirrors pc_port/package_preview.sh (Linux) conventions: required explicit
# PE_PREVIEW_DEV_MENU=1|0, git-ignored output dir, exact-file-list guard,
# game-data extension refusal, sha256 sidecar.  Reproducible: sorted entries,
# fixed timestamps (SOURCE_DATE_EPOCH, default = HEAD commit time).
#
# Env:
#   PE_PREVIEW_DEV_MENU=1|0   REQUIRED (1: F8 dev menu on by default, 0: off)
#   PE_SOURCE_COMMIT          commit recorded in build-info (default: git HEAD)
#   PE_SOURCE_NOTE            free-text provenance note
#   PE_WINDOWS_SCALE          default window scale in the launcher (default 3)
# Nothing is pushed or published by this script.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
die() { echo "ERROR: $*" >&2; exit 1; }

[[ $# -eq 3 ]] || die "usage: $0 <parasite-eve-port.exe> <tag> <outdir>"
EXE="$1"; TAG="$2"; OUTDIR="$3"
[[ -f "$EXE" ]] || die "exe not found: $EXE"
[[ "$TAG" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$ ]] || die "tag must match [A-Za-z0-9][A-Za-z0-9._-]* (got '$TAG')"
file -b "$EXE" | grep -q 'PE32+ executable.*x86-64' || die "not a PE32+ x86-64 exe: $EXE"

DEV_MENU="${PE_PREVIEW_DEV_MENU:-}"
[[ "$DEV_MENU" == 0 || "$DEV_MENU" == 1 ]] || die "PE_PREVIEW_DEV_MENU must be 1 (checkpoint build: F8 dev menu on) or 0 (public preview: off); it is ${DEV_MENU:-unset}"
SCALE="${PE_WINDOWS_SCALE:-3}"
[[ "$SCALE" =~ ^[1-9]$ ]] || die "PE_WINDOWS_SCALE must be 1..9"
SOURCE_COMMIT="${PE_SOURCE_COMMIT:-$(git rev-parse HEAD)}"
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{7,40}$ ]] || die "PE_SOURCE_COMMIT must be a hex commit id"
SOURCE_NOTE="${PE_SOURCE_NOTE:-}"
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(git log -1 --format=%ct HEAD)}"

mkdir -p "$OUTDIR"
OUTDIR="$(cd "$OUTDIR" && pwd)"
# Inside the repo the output dir must be git-ignored; outside it nothing can be committed.
case "$OUTDIR/" in
    "$ROOT"/*) git check-ignore -q "$OUTDIR/x" 2>/dev/null || die "output dir is inside the repo but not git-ignored (game data must never land in git): $OUTDIR" ;;
esac

NAME="parasite-eve-$TAG-windows-x64"
WORK="$(mktemp -d "$OUTDIR/.pe-win-build.XXXXXX")"
trap 'rm -rf -- "$WORK"' EXIT
STAGE="$WORK/$NAME"
mkdir -p "$STAGE/bin" "$STAGE/LICENSES"

install -m 0755 "$EXE" "$STAGE/bin/parasite-eve-port.exe"
install -m 0644 "$ROOT/pc_port/third_party/SDL_GameControllerDB/gamecontrollerdb.txt" "$STAGE/bin/gamecontrollerdb.txt"
install -m 0644 "$ROOT/pc_port/third_party/SDL_GameControllerDB/LICENSE" "$STAGE/bin/gamecontrollerdb-LICENSE.txt"
install -m 0644 "$ROOT/pc_port/third_party/SDL_GameControllerDB/LICENSE" "$STAGE/LICENSES/SDL_GameControllerDB-LICENSE.txt"
install -m 0644 "$ROOT/pc_port/third_party/FidelityFX-FSR-LICENSE.txt" "$STAGE/LICENSES/FidelityFX-FSR-LICENSE.txt"

# --- launcher (CRLF) -------------------------------------------------------
cat > "$STAGE/parasite-eve.cmd" <<'LAUNCHER'
@echo off
rem Parasite Eve native port - Windows launcher (see README-WINDOWS.txt).
rem Disc 1 comes from PE_DISC (full path to the .bin) or the first *.bin in
rem the disc\ folder next to this file.  Extra arguments go to the game.
setlocal EnableExtensions
set "HERE=%~dp0"
set "DISC=%PE_DISC%"
if not defined DISC (
  for %%F in ("%HERE%disc\*.bin") do if not defined DISC set "DISC=%%~fF"
)
if not defined DISC goto :nodisc
if not exist "%DISC%" goto :nodisc
if not defined PE_DEV_MENU set "PE_DEV_MENU=__DEV_MENU__"
"%HERE%bin\parasite-eve-port.exe" --windowed --scale __SCALE__ --window-title "Parasite Eve" --disc-image "%DISC%" %*
exit /b %ERRORLEVEL%
:nodisc
echo Parasite Eve: Disc 1 image not found.
echo Put your own raw BIN/CUE dump of Parasite Eve (USA) Disc 1 in the disc\ folder
echo next to this file (the .bin and its .cue), or set PE_DISC to the .bin path.
echo See README-WINDOWS.txt.
rem Keep a double-clicked console open so the message can be read (PE_NO_PAUSE=1 skips).
if not defined PE_NO_PAUSE pause
exit /b 2
LAUNCHER

# --- README (CRLF) ---------------------------------------------------------
cat > "$STAGE/README-WINDOWS.txt" <<'README'
PARASITE EVE - NATIVE PC PORT - WINDOWS x64
===========================================

This is an early, unfinished research build. It is a compiled native build:
every function that the original binary took from the Psy-Q SDK has been
replaced by clean-room host providers; 0 Psy-Q-classified functions linked.
The game runs as compiled host C, with no interpreter and no CPU emulation.
It contains NO game data, NO BIOS and NO Sony / Psy-Q SDK files.
You must supply your own Parasite Eve (USA) Disc 1 image.

RUN
---
1. Unzip anywhere (a normal user folder; the game writes its save under
   %APPDATA%\parasite-eve-port).
2. Create a folder named "disc" next to parasite-eve.cmd and put your own
   Disc 1 raw BIN/CUE dump in it (Redump style: a 2352-byte/sector .bin with
   its .cue beside it; .iso and .chd are not supported).
3. Double-click parasite-eve.cmd (or run it from Command Prompt; the log goes
   to that console). To use a disc elsewhere:
       set PE_DISC=D:\dumps\Parasite Eve (USA) (Disc 1).bin
       parasite-eve.cmd
   Extra arguments go to the game, e.g.:  parasite-eve.cmd --scale 4
   --fullscreen opens a borderless full-screen window.

CONTROLS
--------
Keyboard: arrows = D-pad; Enter/Space/Z/X = Cross; C = Circle; S = Square;
V = Triangle; Q/E = L1/R1; 1/3 = L2/R2; Tab = Select; P = Start;
F6 = fast-forward toggle; F8 = dev/Options menu; Escape = quit.
Dev/Options menu (F8): __DEV_MENU_WORD__ in this build (launcher default
PE_DEV_MENU=__DEV_MENU__; PE_DEV_MENU=1 or PE_DEV_MENU=0 in the environment
overrides it).
Controller: Xbox-compatible pads through XInput (loaded at run time; no
controller or no XInput = keyboard only).

FILES THE GAME WRITES
---------------------
%APPDATA%\parasite-eve-port\memcard1.mcd   memory card (your saves)
%APPDATA%\parasite-eve-port\controls.conf  key remaps
%APPDATA%\parasite-eve-port\last-stop.txt  stop report
Delete nothing there unless you want to lose your saves.

REQUIREMENTS
------------
64-bit Windows 10 or later. No other runtime to install (the exe imports only
KERNEL32, USER32, GDI32, WINMM and the Windows UCRT).

KNOWN LIMITS
------------
* Verified under Wine on Linux only; real-Windows hardware, controller and
  audio-device behaviour are NOT yet tested. See build-info.json for the
  source commit.
* See docs/port/WINDOWS.md in the source repository for the full list.

LICENSES
--------
LICENSES\FidelityFX-FSR-LICENSE.txt   - AMD FidelityFX FSR 1 (MIT); the optional
                                        Display Scaler FSR mode is a CPU port
                                        of those kernels.
LICENSES\SDL_GameControllerDB-LICENSE.txt, bin\gamecontrollerdb.txt
                                      - SDL_GameControllerDB mappings (zlib).
README

python3 - "$STAGE" "$DEV_MENU" "$SCALE" <<'PY'
import sys, pathlib
stage, dev, scale = pathlib.Path(sys.argv[1]), sys.argv[2], sys.argv[3]
l = stage / "parasite-eve.cmd"
t = l.read_text().replace("__DEV_MENU__", dev).replace("__SCALE__", scale)
l.write_bytes(t.replace("\r\n", "\n").replace("\n", "\r\n").encode())
r = stage / "README-WINDOWS.txt"
word = "ENABLED" if dev == "1" else "DISABLED"
rt = r.read_text().replace("__DEV_MENU_WORD__", word).replace("__DEV_MENU__", dev)
r.write_bytes(rt.replace("\r\n", "\n").replace("\n", "\r\n").encode())
PY

BIN_SHA="$(sha256sum "$STAGE/bin/parasite-eve-port.exe" | cut -d' ' -f1)"
LAUNCHER_SHA="$(sha256sum "$STAGE/parasite-eve.cmd" | cut -d' ' -f1)"
BUILD_TIME="$(date -u -d "@$SOURCE_DATE_EPOCH" +%Y-%m-%dT%H:%M:%SZ)"
python3 - "$STAGE/build-info.json" "$SOURCE_COMMIT" "$SOURCE_NOTE" "$BUILD_TIME" "$BIN_SHA" "$LAUNCHER_SHA" "$DEV_MENU" "$TAG" <<'PY'
import json, sys
out, commit, note, t, bsha, lsha, dev, tag = sys.argv[1:]
json.dump({
    "schema": 1, "name": "parasite-eve-" + tag + "-windows-x64", "tag": tag,
    "platform": "windows-x64", "source_commit": commit, "source_note": note,
    "build_time_utc": t,
    "binary": {"path": "bin/parasite-eve-port.exe", "sha256": bsha},
    "launcher": {"path": "parasite-eve.cmd", "sha256": lsha},
    "dev_menu_default": int(dev), "contains_game_data": False,
    "device_checks": "NOT RUN (Wine only)",
}, open(out, "w"), indent=2)
open(out, "a").write("\n")
PY
chmod 0644 "$STAGE/build-info.json"

# --- guards ----------------------------------------------------------------
grep -qF "set \"PE_DEV_MENU=$DEV_MENU\"" "$STAGE/parasite-eve.cmd" || die "launcher dev-menu default not applied"
! grep -rq "__DEV_MENU__\|__SCALE__" "$STAGE" || die "unreplaced placeholder in the stage"
mapfile -t files < <(cd "$WORK" && find "$NAME" -type f | LC_ALL=C sort)
expected=("$NAME/LICENSES/FidelityFX-FSR-LICENSE.txt" "$NAME/LICENSES/SDL_GameControllerDB-LICENSE.txt"
          "$NAME/README-WINDOWS.txt" "$NAME/bin/gamecontrollerdb-LICENSE.txt" "$NAME/bin/gamecontrollerdb.txt"
          "$NAME/bin/parasite-eve-port.exe" "$NAME/build-info.json" "$NAME/parasite-eve.cmd")
[[ "${files[*]}" == "${expected[*]}" ]] || die "unexpected staged files: ${files[*]}"
for f in "${files[@]}"; do
    case "${f,,}" in
        *.bin|*.cue|*.iso|*.img|*.chd|*.ccd|*.mdf|*.ecm|*.pbp|*.str|*.xa|*.mcd|*/rom/*|*/disc/*) die "refusing to package game data: $f" ;;
    esac
done
bsize="$(stat -c%s "$STAGE/bin/parasite-eve-port.exe")"
(( bsize < 64 * 1024 * 1024 )) || die "exe unexpectedly large ($bsize bytes)"

# --- zip (python zipfile: no zip(1) dependency; sorted, fixed mtime) -------
ZIP="$OUTDIR/$NAME.zip"
python3 - "$WORK" "$NAME" "$WORK/$NAME.zip" "$SOURCE_DATE_EPOCH" <<'PY'
import os, sys, time, zipfile
work, name, out, epoch = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
ts = time.gmtime(max(epoch, 315532800))[:6]
files = []
for d, _, fs in os.walk(os.path.join(work, name)):
    files += [os.path.join(d, f) for f in fs]
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for p in sorted(files):
        arc = os.path.relpath(p, work)
        zi = zipfile.ZipInfo(arc, ts)
        zi.compress_type = zipfile.ZIP_DEFLATED
        zi.external_attr = ((0o755 if arc.endswith(".exe") else 0o644) << 16)
        with open(p, "rb") as f:
            z.writestr(zi, f.read())
PY
mv -f "$WORK/$NAME.zip" "$ZIP"
ZIP_SHA="$(sha256sum "$ZIP" | cut -d' ' -f1)"
echo "$ZIP_SHA  $NAME.zip" > "$ZIP.sha256"
echo "==> $ZIP"
echo "    $ZIP_SHA  $NAME.zip"
echo "    exe sha256 $BIN_SHA (commit $SOURCE_COMMIT)"
python3 -m zipfile -l "$ZIP"
