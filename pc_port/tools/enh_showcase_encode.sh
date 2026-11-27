#!/usr/bin/env bash
# Encode the enh_showcase.py capture into the showcase video (1920x1080, 60 fps,
# H.264 + AAC).  Each segment is cut by the VBlank stamps in its meta.json; the
# port's audio dump is 735 samples per emulated VBlank at 44100 Hz, so the audio
# cut uses the same VBlanks and stays in sync.  Frames are scaled 4.5x
# nearest-neighbour and pillarboxed; captions are burned in with libass.
#
# Usage: enh_showcase_encode.sh <capture dir> <out.mp4>   (FFMPEG=path overrides)
set -euo pipefail
SHOW=$(cd "$1" && pwd); OUT=$2
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
FF=${FFMPEG:-$ROOT/build/reference/ffmpeg}
[ -x "$FF" ] || FF=ffmpeg
W=$(mktemp -d "${TMPDIR:-/tmp}/enh-encode.XXXXXX"); trap 'rm -rf "$W"' EXIT
VF="scale=1440:1080:flags=neighbor,pad=1920:1080:240:0:black,setsar=1,fps=60"
ENC=(-c:v libx264 -preset slow -crf 16 -pix_fmt yuv420p -r 60 -c:a aac -b:a 192k -ar 44100 -ac 2)

ass() {  # ass <file> <duration s> <line1> [line2]
    cat > "$1" <<EOF
[Script Info]
ScriptType: v4.00+
PlayResX: 1920
PlayResY: 1080
[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: Cap,DejaVu Sans,46,&H00FFFFFF,&H00FFFFFF,&H00000000,&H90000000,1,0,0,0,100,100,0,0,3,2,0,2,40,40,46,1
Style: Sub,DejaVu Sans,30,&H00E0E0E0,&H00E0E0E0,&H00000000,&H90000000,0,0,0,0,100,100,0,0,3,2,0,2,40,40,112,1
[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
Dialogue: 0,0:00:00.00,0:00:$(printf '%05.2f' "$2"),Cap,,0,0,0,,$3
EOF
    [ -n "${4:-}" ] && echo "Dialogue: 0,0:00:00.00,0:00:$(printf '%05.2f' "$2"),Sub,,0,0,0,,$4" >> "$1"
    return 0
}

clip() {  # clip <seg> <first vbl> <last vbl> <idx> <caption> [sub]
    local d="$SHOW/seg_$1" a=$2 b=$3 n=$(( $3 - $2 + 1 )) dur
    dur=$(python3 -c "print(f'{$n/60:.3f}')")
    ass "$W/c$4.ass" "$dur" "$5" "${6:-}"
    "$FF" -v error -y -framerate 60 -start_number "$a" -i "$d/v%06d.ppm" \
        -ss "$(python3 -c "print(f'{$a*735/44100:.6f}')")" -t "$dur" -i "$d/audio.wav" \
        -frames:v "$n" -vf "$VF,subtitles=$W/c$4.ass,fade=t=in:st=0:d=0.2,fade=t=out:st=$(python3 -c "print(f'{$n/60-0.2:.3f}')"):d=0.2" \
        -af "afade=t=in:d=0.15,afade=t=out:st=$(python3 -c "print(f'{$n/60-0.2:.3f}')"):d=0.2" \
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

m() { python3 -c "import json,sys; print(json.load(open('$SHOW/seg_$1/meta.json'))['$2'])"; }

: > "$W/list.txt"
card 0 3.0 "Parasite Eve — native port" "Opt-in enhancement: Movement Speed (Options ▸ Enhancements). Off by default; 100 % is retail."
clip 100   $(( $(m 100 start_vbl) - 20 )) $(( $(m 100 stop_vbl) + 30 )) 1 "Movement speed 100 % (retail)" "Walk, then run (Circle): the same stretch each time"
clip 150   $(( $(m 150 menu_vbl) - 10 )) $(( $(m 150 stop_vbl) + 30 )) 2 "Movement speed 150 %" "Set in the dev menu: Options ▸ Enhancements"
clip 200   $(( $(m 200 menu_vbl) - 10 )) $(( $(m 200 stop_vbl) + 30 )) 3 "Movement speed 200 %"
clip 200ar $(( $(m 200ar closed_vbl) - 110 )) $(( $(m 200ar stop_vbl) + 30 )) 4 "200 % + Always Run" "D-pad only, no Circle held"
card 5 3.0 "Only Aya's field walk/run is scaled" "Battle, enemies, cutscenes, scripted moves, timers and RNG are untouched. Captured headless at native game speed with the port's own audio."
"$FF" -v error -y -f concat -safe 0 -i "$W/list.txt" -c copy -movflags +faststart "$OUT"
"$FF" -hide_banner -i "$OUT" 2>&1 | grep -E 'Duration|Stream' || true
