#!/usr/bin/env bash
# Phase 1: verify user-supplied NTSC-U disc images and extract the main
# executables. Inputs are bin+cue images under rom/image/ (git-ignored);
# all outputs go to build/extracted/ (git-ignored). Nothing is written to
# git-tracked paths — results are recorded manually in docs/disc_info.md
# with the exact commands.
#
# Usage: scripts/extract_us.sh [1|2|all]   (default: all)
#
# Discs present under rom/image/ are processed; missing discs are skipped
# with a warning. Fails if no requested disc is found or any step errors.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE_DIR="${PE_IMAGE_DIR:-$ROOT/rom/image}"  # override for testing
OUT_ROOT="$ROOT/build/extracted"
PSXISO="$ROOT/tools/extract/psxiso.py"
EXEINFO="$ROOT/tools/analysis/psxexe_info.py"
HASHER="$ROOT/tools/verify/hashfile.py"

process_disc() {
    local n="$1" serial="$2" exe="$3"
    local cue bin bin_name out boot boot_exe

    cue="$(find "$IMAGE_DIR" -iname "*disc ${n}*.cue" -print -quit)"
    if [[ -z "$cue" ]]; then
        echo "SKIP: disc $n ($serial): no '*Disc ${n}*.cue' under $IMAGE_DIR" >&2
        return 10
    fi

    echo "=================================================================="
    echo "Disc $n ($serial) — cue: $cue"

    bin_name="$(tr -d '\r' < "$cue" | sed -n 's/^ *FILE "\(.*\)" BINARY$/\1/p' | head -n1)"
    [[ -n "$bin_name" ]] || { echo "ERROR: no FILE entry in cue" >&2; return 1; }
    bin="$(dirname "$cue")/$bin_name"
    [[ -f "$bin" ]] || { echo "ERROR: bin not found: $bin" >&2; return 1; }

    if (( $(grep -c 'TRACK ' "$cue") != 1 )); then
        echo "WARNING: cue has multiple tracks; tools assume single data track" >&2
    fi

    # Fresh output dir every run: stale files from a previous run must never
    # masquerade as this run's results.
    out="$OUT_ROOT/disc$n"
    rm -rf "$out"
    mkdir -p "$out"

    # NOTE: process_disc is invoked in a tested context (|| in process()),
    # which suppresses `set -e` inside this function body. Every step below
    # therefore carries an explicit `|| return 1` — do not remove them.
    echo "--- image hashes (redump-comparable) ---"
    python3 "$HASHER" "$bin" | tee "$out/image_hashes.txt" \
        || { echo "ERROR: hashing failed for $bin" >&2; return 1; }

    echo "--- ISO9660 volume info ---"
    python3 "$PSXISO" info "$bin" \
        || { echo "ERROR: not a readable MODE2/2352 PS1 image: $bin" >&2; return 1; }

    echo "--- file listing -> $out/filelist.txt ---"
    python3 "$PSXISO" list "$bin" > "$out/filelist.txt" \
        || { echo "ERROR: ISO9660 file listing failed for $bin" >&2; return 1; }
    echo "$(wc -l < "$out/filelist.txt") files on disc"

    echo "--- SYSTEM.CNF ---"
    python3 "$PSXISO" extract "$bin" "SYSTEM.CNF" "$out/SYSTEM.CNF" >/dev/null \
        || { echo "ERROR: SYSTEM.CNF extraction failed from $bin" >&2; return 1; }
    tr -d '\r' < "$out/SYSTEM.CNF"

    boot="$(tr -d '\r' < "$out/SYSTEM.CNF" | sed -n 's/^BOOT *= *//p')"
    boot_exe="$(basename "${boot#cdrom:}" | sed 's/;1$//' | tr -d '\\/')"
    if [[ "$boot_exe" != "$exe" ]]; then
        echo "ERROR: SYSTEM.CNF boots '$boot_exe', expected '$exe'" >&2
        return 1
    fi

    echo "--- extracting $exe ---"
    python3 "$PSXISO" extract "$bin" "$exe" "$out/$exe" \
        || { echo "ERROR: extraction of $exe failed" >&2; return 1; }
    python3 "$HASHER" "$out/$exe" | tee "$out/exe_hashes.txt" \
        || { echo "ERROR: hashing failed for $out/$exe" >&2; return 1; }

    echo "--- PS-X EXE header ---"
    python3 "$EXEINFO" "$out/$exe" \
        || { echo "ERROR: $out/$exe is not a valid PS-X EXE" >&2; return 1; }

    echo "OK: disc $n extracted to $out"
}

want="${1:-all}"
found=0
process() {
    local n="$1" serial="$2" exe="$3" rc=0
    process_disc "$n" "$serial" "$exe" || rc=$?
    if (( rc == 0 )); then
        found=$((found+1))
    elif (( rc != 10 )); then
        exit "$rc"
    fi
}
case "$want" in
    1)   process 1 SLUS-00662 SLUS_006.62 ;;
    2)   process 2 SLUS-00668 SLUS_006.68 ;;
    all) process 1 SLUS-00662 SLUS_006.62
         process 2 SLUS-00668 SLUS_006.68 ;;
    *)   echo "usage: $0 [1|2|all]" >&2; exit 2 ;;
esac

if (( found == 0 )); then
    echo "ERROR: no requested disc images found under $IMAGE_DIR" >&2
    exit 1
fi
echo "=================================================================="
echo "Done: $found disc(s) processed. Record results in docs/disc_info.md."
