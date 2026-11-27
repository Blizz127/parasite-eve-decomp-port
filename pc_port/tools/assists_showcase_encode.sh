#!/usr/bin/env bash
# Encode the assists_showcase.py capture into the ASSISTS showcase video
# (1920x1080, 60 fps, H.264 + AAC).  Side-by-side clips (OFF | ON) are cut by
# VBlank from two runs of the same deterministic route, so they stay in step;
# the audio is the OFF run's (the port's own dump, 735 samples per VBlank).
# Frames are scaled nearest-neighbour; captions are burned in with libass.
#
# Usage: assists_showcase_encode.sh <capture dir> <out.mp4>   (FFMPEG=path overrides)
set -euo pipefail
CAP=$(cd "$1" && pwd); OUT=$2
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
FF=${FFMPEG:-$ROOT/build/reference/ffmpeg}
[ -x "$FF" ] || FF=ffmpeg
W=$(mktemp -d "${TMPDIR:-/tmp}/assists-encode.XXXXXX"); trap 'rm -rf "$W"' EXIT
ENC=(-c:v libx264 -preset slow -crf 16 -pix_fmt yuv420p -r 60 -c:a aac -b:a 192k -ar 44100 -ac 2)
FULL="scale=1440:1080:flags=neighbor,pad=1920:1080:240:0:black,setsar=1,fps=60"
HALF="scale=960:720:flags=neighbor,setsar=1,fps=60"

ass() {  # ass <file> <duration s> <line1> [line2] [left-title] [right-title]
    cat > "$1" <<ASS
[Script Info]
ScriptType: v4.00+
PlayResX: 1920
PlayResY: 1080
[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: Cap,DejaVu Sans,46,&H00FFFFFF,&H00FFFFFF,&H00000000,&H90000000,1,0,0,0,100,100,0,0,3,2,0,2,40,40,46,1
Style: Sub,DejaVu Sans,30,&H00E0E0E0,&H00E0E0E0,&H00000000,&H90000000,0,0,0,0,100,100,0,0,3,2,0,2,40,40,112,1
Style: TL,DejaVu Sans,36,&H00FFFFFF,&H00FFFFFF,&H00000000,&H90000000,1,0,0,0,100,100,0,0,3,2,0,7,40,40,70,1
Style: TR,DejaVu Sans,36,&H0080FFFF,&H00FFFFFF,&H00000000,&H90000000,1,0,0,0,100,100,0,0,3,2,0,9,40,40,70,1
[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
Dialogue: 0,0:00:00.00,0:00:$(printf '%05.2f' "$2"),Cap,,0,0,0,,$3
ASS
    [ -n "${4:-}" ] && echo "Dialogue: 0,0:00:00.00,0:00:$(printf '%05.2f' "$2"),Sub,,0,0,0,,$4" >> "$1"
    [ -n "${5:-}" ] && echo "Dialogue: 0,0:00:00.00,0:00:$(printf '%05.2f' "$2"),TL,,0,0,0,,$5" >> "$1"
    [ -n "${6:-}" ] && echo "Dialogue: 0,0:00:00.00,0:00:$(printf '%05.2f' "$2"),TR,,0,0,0,,$6" >> "$1"
    return 0
}
secs() { python3 -c "print(f'{$1/60:.3f}')"; }
astart() { python3 -c "print(f'{$1*735/44100:.6f}')"; }
fades() { echo "fade=t=in:st=0:d=0.2,fade=t=out:st=$(python3 -c "print(f'{$1/60-0.2:.3f}')"):d=0.2"; }
afades() { echo "afade=t=in:d=0.15,afade=t=out:st=$(python3 -c "print(f'{$1/60-0.2:.3f}')"):d=0.2"; }

pair() {  # pair <left run> <right run> <first vbl> <last vbl> <idx> <caption> <sub> <left title> <right title>
    local L="$CAP/$1" R="$CAP/$2" a=$3 n=$(( $4 - $3 + 1 )) dur
    dur=$(secs "$n"); ass "$W/c$5.ass" "$dur" "$6" "$7" "$8" "$9"
    "$FF" -v error -y -framerate 60 -start_number "$a" -i "$L/v%06d.ppm" \
        -framerate 60 -start_number "$a" -i "$R/v%06d.ppm" \
        -ss "$(astart "$a")" -t "$dur" -i "$L/audio.wav" \
        -filter_complex "[0:v]$HALF[l];[1:v]$HALF[r];[l][r]hstack=inputs=2,pad=1920:1080:0:180:black,subtitles=$W/c$5.ass,$(fades "$n")[v]" \
        -map "[v]" -map 2:a -frames:v "$n" -af "$(afades "$n")" "${ENC[@]}" "$W/c$5.mp4"
    echo "file '$W/c$5.mp4'" >> "$W/list.txt"
}
clip() {  # clip <run> <first vbl> <last vbl> <idx> <caption> [sub]
    local d="$CAP/$1" a=$2 n=$(( $3 - $2 + 1 )) dur
    dur=$(secs "$n"); ass "$W/c$4.ass" "$dur" "$5" "${6:-}"
    "$FF" -v error -y -framerate 60 -start_number "$a" -i "$d/v%06d.ppm" \
        -ss "$(astart "$a")" -t "$dur" -i "$d/audio.wav" \
        -frames:v "$n" -vf "$FULL,subtitles=$W/c$4.ass,$(fades "$n")" -af "$(afades "$n")" \
        "${ENC[@]}" "$W/c$4.mp4"
    echo "file '$W/c$4.mp4'" >> "$W/list.txt"
}
card() {  # card <idx> <seconds> <line1> <line2>
    ass "$W/c$1.ass" "$2" "{\\an5\\fs64}$3\\N\\N{\\fs34\\b0}$4"
    "$FF" -v error -y -f lavfi -i "color=c=black:s=1920x1080:r=60:d=$2" -f lavfi -i "anullsrc=r=44100:cl=stereo" \
        -t "$2" -vf "subtitles=$W/c$1.ass,fade=t=in:st=0:d=0.3,fade=t=out:st=$(python3 -c "print($2-0.3)"):d=0.3" \
        "${ENC[@]}" "$W/c$1.mp4"
    echo "file '$W/c$1.mp4'" >> "$W/list.txt"
}
m() { python3 -c "import json; print(json.load(open('$CAP/$1/meta.json'))$2)"; }
v() { python3 -c "print(2*$1-2)"; }   # frame -> VBlank on this route

: > "$W/list.txt"
card 0 3.0 "Parasite Eve — native port: Assists" "Dev menu ▸ Assists. All OFF by default; the defaults are the retail game."
# 1) the dev menu ASSISTS page: all OFF, each toggled once and back
clip menu $(v $(m menu "['marks']['page']")) $(( $(v $(m menu "['marks']['closed']")) + 20 )) 1 \
    "Dev menu ▸ Assists" "Every row starts OFF; settings are remembered in enhancements.conf"
# 2) god mode: the same Eve-fight window, OFF | ON
pair eve_off eve_god $(v 10640) $(v 11340) 2 "God Mode (No Damage)" \
    "Same fight, same hits: HP 45 → 42 → 35 → 28 on the left, 45 on the right" "OFF (retail)" "GOD MODE ON"
# 3) 1-hit kills: the rat fight, OFF | ON
pair rat_off rat_1hit $(v 18895) $(v 19300) 3 "1-Hit Kills" \
    "One shot ends the fight; EXP, BP and drops are awarded as usual" "OFF (retail)" "1-HIT KILLS ON"
# 4) EXP/BP: the same rat fight's results, x1 | x3
pair rat_off rat_exp3 $(v 19270) $(v 19420) 4 "EXP/BP Multiplier" \
    "The same battle: EXP 2 / BP 3 on the left, EXP 6 / BP 9 on the right" "x1 (retail)" "x3"
card 5 3.0 "Off by default — the retail game is untouched" "Captured headless at native game speed with the port's own audio; both halves are the same deterministic route."
"$FF" -v error -y -f concat -safe 0 -i "$W/list.txt" -c copy -movflags +faststart "$OUT"
"$FF" -hide_banner -i "$OUT" 2>&1 | grep -E 'Duration|Stream' || true
