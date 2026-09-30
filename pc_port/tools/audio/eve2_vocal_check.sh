#!/usr/bin/env bash
# Eve 2 vocal-layer regression check (audio3, divergence #20).
# Input: a port WAV capture (PE_AUDIO_WAV) of a --route-pad run through the
# Eve 2 battle, and the battle-start time in that capture (seconds; from the
# BATTLE_EDGE frame, or pass the capture time of the battle-music start).
# Reference: retail part 02 audio (owner video, git-ignored), 80-145 s.
# Method: find the port offset that best matches retail part 02 @ 1:28 over
# 100-4000 Hz (scan +-8 s), then correlate the 600-1500 Hz log spectrogram in
# the three 3 s windows that hold vocal blobs (retail 1:44-1:53).
# Scores (mean r): current tree 0.59, r2 build (no vocals) 0.30,
# current tree with EA100/101 suppressed 0.48.  FAIL below 0.42.
# usage: eve2_vocal_check.sh <capture.wav> <approx_music_start_s>
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../../.." && pwd)"
ff="$root/build/reference/ffmpeg"
ref="$root/build/reference/retail-video/part02.audio.webm"
cap="$1"; t0="$2"
tmp="$(mktemp -d "${TMPDIR:-/tmp}/eve2chk.XXXXXX")"; trap 'rm -r "$tmp"' EXIT
bc="${TMPDIR:-/tmp}/pe-bandcorr"; [ -x "$bc" ] || gcc -O2 -o "$bc" "$here/bandcorr.c" -lm
"$ff" -v error -y -ss 80 -t 65 -i "$ref" -ac 2 -ar 44100 "$tmp/ret.wav"
s=$(awk -v t="$t0" 'BEGIN{printf "%.1f", t-20}')
"$ff" -v error -y -ss "$s" -t 80 -i "$cap" -ac 2 -ar 44100 "$tmp/port.wav"
best=-2; bo=0
for o in $(seq 12 0.5 28); do
  r=$("$bc" "$tmp/ret.wav" 8 "$tmp/port.wav" "$o" 8 8 100 4000 | awk 'NR==2{print $2}')
  if awk -v a="$r" -v b="$best" 'BEGIN{exit !(a>b)}'; then best=$r; bo=$o; fi
done
tb=$(awk -v o="$bo" 'BEGIN{printf "%.1f", o-5}')   # retail slice 3 s <-> port o-5 s
score=$("$bc" "$tmp/ret.wav" 3 "$tmp/port.wav" "$tb" 30 3 600 1500 |
  awk '$1=="18.0"||$1=="21.0"||$1=="27.0"{s+=$2;n++} END{printf "%.3f", n?s/n:-1}')
echo "align r=$best at capture $(awk -v s="$s" -v o="$bo" 'BEGIN{print s+o}') s; vocal-window mean r=$score"
if awk -v a="$best" 'BEGIN{exit !(a<0.45)}'; then echo "FAIL: could not align to retail (r<0.45)"; exit 2; fi
if awk -v a="$score" 'BEGIN{exit !(a<0.42)}'; then echo "FAIL: Eve 2 vocal layer missing"; exit 1; fi
echo "PASS: Eve 2 vocal layer present"
