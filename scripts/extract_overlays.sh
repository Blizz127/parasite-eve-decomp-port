#!/usr/bin/env bash
# scripts/extract_overlays.sh — extract the PE.IMG subsystem overlays.
#
# Reads the overlay tables from the locally extracted retail EXE, carves each
# subsystem overlay listed in configs/USA/overlays/manifest.yaml out of
# build/extracted/disc1/PE.IMG into build/extracted/disc1/peimg/<id>.bin, and
# verifies every blob's SHA-1 against the manifest. All output is git-ignored
# (build/); the manifest carries hashes and geometry only — the bytes are never
# committed.
#
# PE.IMG itself is not produced by scripts/extract_us.sh. When it is absent,
# this script extracts it from the user's disc 1 image under rom/image/ with
# tools/extract/psxiso.py (the same reader extract_us.sh uses); when no image is
# present either, it fails loudly with the remedy.
#
# Usage:
#   scripts/extract_overlays.sh            extract + verify every subsystem overlay
#   scripts/extract_overlays.sh --check    verify already-extracted blobs only
#   scripts/extract_overlays.sh --only ovl_0700 [--only ...]
#   scripts/extract_overlays.sh --room m0418i [--chunk 2]
#                                          additionally extract one room package chunk
#
# Idempotent: an existing blob with the recorded SHA-1 is left untouched; a
# wrong one is reported and the script exits 1 (nothing is overwritten with
# bytes that do not hash as recorded).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MANIFEST="$ROOT/configs/USA/overlays/manifest.yaml"
PEIMG_TOOL="$ROOT/tools/extract/peimg.py"
PSXISO="$ROOT/tools/extract/psxiso.py"
IMAGE_DIR="${PE_IMAGE_DIR:-$ROOT/rom/image}"
PEIMG="$ROOT/build/extracted/disc1/PE.IMG"
EXE="$ROOT/build/extracted/disc1/SLUS_006.62"
OUT_DIR="build/extracted/disc1/peimg"

CHECK_ONLY=0
ONLY=()
ROOM=""
CHUNK=2
while [[ $# -gt 0 ]]; do
    case "$1" in
        --check) CHECK_ONLY=1 ;;
        --only) [[ $# -ge 2 ]] || { echo "--only needs an id" >&2; exit 2; }; ONLY+=("--only" "$2"); shift ;;
        --room) [[ $# -ge 2 ]] || { echo "--room needs a name" >&2; exit 2; }; ROOM="$2"; shift ;;
        --chunk) [[ $# -ge 2 ]] || { echo "--chunk needs a number" >&2; exit 2; }; CHUNK="$2"; shift ;;
        -h|--help)
            awk 'NR == 1 { next } /^#/ { sub(/^# ?/, ""); print; next } { exit }' "${BASH_SOURCE[0]}"
            exit 0
            ;;
        *) echo "usage: $0 [--check] [--only ID]... [--room m####i [--chunk N]]" >&2; exit 2 ;;
    esac
    shift
done

# 1. Root guard.
if [[ ! -f "$ROOT/configs/USA/disc1.yaml" || ! -f "$MANIFEST" ]]; then
    echo "ERROR: $ROOT does not look like the Parasite-Eve-Decompilation root (no configs/USA/disc1.yaml / overlay manifest)." >&2
    exit 1
fi
command -v python3 >/dev/null || { echo "ERROR: python3 not found on PATH" >&2; exit 1; }

# 2. Output must be git-ignored: extracted game data can never be committable.
if ! git -C "$ROOT" check-ignore -q "$OUT_DIR"; then
    echo "ERROR: $OUT_DIR is not covered by .gitignore; refusing to extract game data into a committable path." >&2
    exit 1
fi

# 3. Retail EXE (the tables live in it).
if [[ ! -f "$EXE" ]]; then
    echo "ERROR: missing $EXE — run 'scripts/extract_us.sh 1' first (needs your disc 1 image under rom/image/)." >&2
    exit 1
fi

# 4. PE.IMG: present, or extractable from the disc image, or fail loudly.
if [[ ! -f "$PEIMG" ]]; then
    if [[ "$CHECK_ONLY" -eq 1 ]]; then
        echo "ERROR: missing $PEIMG (nothing to check). Run $0 without --check." >&2
        exit 1
    fi
    cue="$(find "$IMAGE_DIR" -iname "*disc 1*.cue" -print -quit 2>/dev/null || true)"
    if [[ -z "$cue" ]]; then
        echo "ERROR: missing $PEIMG and no '*Disc 1*.cue' under $IMAGE_DIR to extract it from." >&2
        echo "Remedy: place your disc 1 bin+cue under rom/image/ (git-ignored) and re-run, or" >&2
        echo "        python3 $PSXISO extract <disc1.bin> PE.IMG $PEIMG" >&2
        exit 1
    fi
    bin_name="$(tr -d '\r' < "$cue" | sed -n 's/^ *FILE "\(.*\)" BINARY$/\1/p' | head -n1)"
    bin="$(dirname "$cue")/$bin_name"
    [[ -f "$bin" ]] || { echo "ERROR: bin not found: $bin" >&2; exit 1; }
    echo "PE.IMG absent; extracting it from $bin"
    mkdir -p "$(dirname "$PEIMG")"
    python3 "$PSXISO" extract "$bin" "PE.IMG" "$PEIMG" >/dev/null
fi

# 5. Tables must agree with the manifest, then extract/verify.
echo "== overlay extraction =="
echo "  root:     $ROOT"
echo "  manifest: ${MANIFEST#"$ROOT/"}"
echo "  image:    ${PEIMG#"$ROOT/"}"
python3 "$PEIMG_TOOL" --root "$ROOT" tables
if [[ "$CHECK_ONLY" -eq 1 ]]; then
    python3 "$PEIMG_TOOL" --root "$ROOT" check "${ONLY[@]}"
else
    python3 "$PEIMG_TOOL" --root "$ROOT" extract "${ONLY[@]}"
fi
if [[ -n "$ROOM" ]]; then
    python3 "$PEIMG_TOOL" --root "$ROOT" --skip-image-hash room "$ROOM" --chunk "$CHUNK"
fi

# 6. The extraction must not have created anything git can see.
if git -C "$ROOT" status --porcelain -- "$OUT_DIR" | grep -q .; then
    echo "ERROR: extraction produced files git does not ignore under $OUT_DIR:" >&2
    git -C "$ROOT" status --porcelain -- "$OUT_DIR" >&2
    exit 1
fi
echo "OK: overlay blobs are local-only under $OUT_DIR (git-ignored); hashes recorded in $OUT_DIR/hashes.txt"
