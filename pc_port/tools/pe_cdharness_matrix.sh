#!/usr/bin/env bash
# usage: CDX_WORK=... DISC=... DISC2=... PERSIST=... [BIN=...] bash pe_cdharness_matrix.sh [all|boot|credits]
# The sw-cdharness coverage matrix (boot legs, PE_TEST_CD_* fixtures, Day-6 credits legs).  Every leg is a separate isolated process;
# boot legs run 4 at a time, the credits legs 5 at a time.  Coverage harness, not play evidence (fixtures change only the host CD device model).
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd); T=$ROOT/pc_port/tools
: "${CDX_WORK:?}" "${DISC:?}" "${DISC2:?}" "${PERSIST:?}"; export CDX_WORK DISC DISC2 PERSIST PE_DISC1_BIN="$DISC"
cold() { bash $T/pe_cdharness_cold.sh "$@"; }
day6() { tag=$1; shift; CDH_ENV="$*" bash $T/pe_cdharness_day6.sh $tag --room m0050i --story 2F2 --until ending --frames 60000 --linger 0 > $CDX_WORK/$tag.out 2>&1; }
LEGS=${1:-all}
if [ "$LEGS" != credits ]; then
# boot legs
cold 600 boot600 &
cold 9000 boot9000-movie &
cold 600 fx-drq5 PE_TEST_CD_DRQ_DELAY=5 PE_TEST_CD_SECTORS=2 &
cold 600 fx-dma5 PE_TEST_CD_DMA_BUSY=5 &
cold 9000 fx-moviedev PE_TEST_CD_MOVIE_DEVICE=1 &
wait
for c in 1 2 9 14 27; do cold 600 fx-stall$c PE_TEST_CD_CMD_STALL=$c,400000000 & done
cold 600 fx-rderr-0_boundary PE_TEST_CD_READ_ERROR=0,boundary &
wait
# Day-1 route to field_start (cdgen47 settings and without pad-only aids; with and without movies)
bash $T/pe_cdharness_route.sh field_start route-padonly-movies PE_ROUTE_AIDS_PAD_ONLY=1 PE_ROUTE_PLAY_MOVIES=1 > /dev/null &
bash $T/pe_cdharness_route.sh field_start route-aids-movies PE_ROUTE_PLAY_MOVIES=1 > /dev/null &
bash $T/pe_cdharness_route.sh field_start route-aids-nomovies > /dev/null &
wait
# default-off equality: final screenshot / WAV / periodic shots of a 1500-frame boot must be equal for the wrapped binary, the unwrapped one (EQ_BINS="name:path ...")
for b in pe-cdh:${BIN:-$ROOT/build/cap/pe-cdh} ${EQ_BINS:-}; do n=${b%%:*}; p=${b#*:}; BIN=$p cold 1500 eq-$n > /dev/null; R=$CDX_WORK/run-eq-$n; echo "eq $n shot=$(sha256sum < $R/final.ppm | cut -c1-16) wav=$(sha256sum < $R/cold.wav | cut -c1-16) shots=$(cat $R/shots/*.ppm | sha256sum | cut -c1-16)"; done
# pad-code credits attempt (pad input only; the dumps are READ-ONLY RAM snapshots of D_8009D1A0 / D_8009D26C / D_8009D2A8)
mkdir -p $CDX_WORK/padcode-dumps
FRAMES=1900 bash $T/pe_cdharness_padcode.sh padcode-dump m0042i 88 PE_D3_RAM_DUMP=1590:$CDX_WORK/padcode-dumps/a.bin,1625:$CDX_WORK/padcode-dumps/b.bin,1800:$CDX_WORK/padcode-dumps/c.bin &
FRAMES=4000 bash $T/pe_cdharness_padcode.sh padcode-full m0042i 88 &
WALL=400 cold 600 fx-rderr-0_int5 PE_TEST_CD_READ_ERROR=0,int5 &
WALL=400 cold 600 fx-rderr-16_int5 PE_TEST_CD_READ_ERROR=16,int5 &
fi
if [ "$LEGS" != boot ]; then
# credits legs (FROM_VB=4600: the boot mount and the dev warp's Disc 2 remount at VBlank 1998 both run below that; the credits arm's CD traffic starts at about VBlank 4636)
V="PE_TEST_CD_FROM_VB=4600"
day6 credits1 &
day6 credits-full PE_CDX_MAXTUPLES=1000000000 &
day6 credits-stall17 $V PE_TEST_CD_CMD_STALL=17,400000000 &
day6 credits-drq $V PE_TEST_CD_DRQ_DELAY=3 PE_TEST_CD_SECTORS=3 &
day6 credits-dma $V PE_TEST_CD_DMA_BUSY=3 PE_TEST_CD_SECTORS=3 &
day6 credits-stall13 $V PE_TEST_CD_CMD_STALL=13,400000000 &
day6 credits-stall14 $V PE_TEST_CD_CMD_STALL=14,400000000 &
day6 credits-stall27 $V PE_TEST_CD_CMD_STALL=27,400000000 &
day6 credits-stall8 $V PE_TEST_CD_CMD_STALL=8,400000000 &
day6 credits-stall9 $V PE_TEST_CD_CMD_STALL=9,400000000 &
day6 credits-rderr PE_TEST_CD_FROM_VB=4600 PE_TEST_CD_READ_ERROR=204600,int5 &
# long stalls (59 s / 118 s of emulated time): does the poll's timeout seam PE_Cd_TimeoutRecover ever fire?
day6 credits-stall8-2G $V PE_TEST_CD_CMD_STALL=8,2000000000 &
day6 credits-stall9-2G $V PE_TEST_CD_CMD_STALL=9,2000000000 &
day6 credits-stall8-4G $V PE_TEST_CD_CMD_STALL=8,4000000000 &
day6 credits-stall9-4G $V PE_TEST_CD_CMD_STALL=9,4000000000 &
day6 credits-stall13-4G $V PE_TEST_CD_CMD_STALL=13,4000000000 &
day6 credits-stall14-4G $V PE_TEST_CD_CMD_STALL=14,4000000000 &
# disc tray swap while the opening movie streams through the host feeder: the one leg in which the stream handler runs with no PE_TEST_CD_* fixture (via the movie DMA callback func_801214D4)
CDH_ENV="PE_ROUTE_PLAY_MOVIES=1 PE_ROUTE_AIDS_PAD_ONLY=1" bash $T/pe_cdharness_day6.sh tray-during-movie --room m0050i --story 2F2 --until ending --frames 1200 --wall 90 > $CDX_WORK/tray-during-movie.out 2>&1 &
# opening movie through the device (feeder-bypass fixture) with a ReadS stall / read error aimed past the first mount
cold 9000 fx-moviedev-stall27 PE_TEST_CD_MOVIE_DEVICE=1 PE_TEST_CD_FROM_VB=400 PE_TEST_CD_CMD_STALL=27,400000000 &
cold 9000 fx-moviedev-rderr PE_TEST_CD_MOVIE_DEVICE=1 PE_TEST_CD_FROM_VB=400 PE_TEST_CD_READ_ERROR=189745,int5 &
wait
fi
wait
