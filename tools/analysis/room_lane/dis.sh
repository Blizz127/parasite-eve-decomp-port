#!/bin/bash
# dis.sh <room> <off> <size>: raw disassembly of a room chunk-2 range at VRAM 0x8018EFE8

b=build/extracted/disc1/peimg/room_$1_c2.bin; [ -f $b ] || python3 tools/extract/peimg.py room $1 >/dev/null
t=${ROOM_LANE_WORK:-build/room_lane}/dis.bin; python3 -c "import sys;d=open('$b','rb').read();open('$t','wb').write(d[$2:$2+$3])"
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL --no-show-raw-insn -M reg-names=numeric --adjust-vma=$(printf '0x%X' $((0x8018EFE8+$2))) $t | sed -n '/^ *8/p'
