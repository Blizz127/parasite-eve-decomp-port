#!/bin/bash
# try.sh <target> <file.c> <vram> <size> [flags]  -> copies file into a func-named dir and runs ldiff
set -euo pipefail
cd "$(dirname "$0")/../../.."; export TMPDIR=${TMPDIR:-$PWD/build/tmp}

export LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu
t=$1; f=$2; v=$3; s=$4; shift 4
name=func_$(printf '%08X' $((v)))
d=${ROOM_LANE_WORK:-build/room_lane}/tryd; mkdir -p $d; cp "$f" $d/$name.c
python3 tools/analysis/room_lane/ldiff.py $t $d/$name.c $v $s "$@" || true
