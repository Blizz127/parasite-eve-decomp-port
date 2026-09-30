#!/usr/bin/env python3
"""Run the original audio FIFO consumer and its complete command call graphs."""
import hashlib
import itertools
import struct
import sys
from pe_battle_hud_oracle import ROOT, execute

CALLBACKS={0:0x8008CA7C,0xF0:0x8008C374,0xF1:0x8008C46C,
    0x9A:0x8008C814,0x9C:0x8008C9DC,0x80:0x8008C55C,
    0x81:0x8008C654,0x82:0x8008C5D8,0xC0:0x8008B1FC,
    0x21:0x8008B1D0,0x24:0x8008B168,0x92:0x8008C70C,
    0x11:0x8008C3E4,0xC1:0x8008B2CC,0xC2:0x8008B410,0xA0:0x8008B698,0xA1:0x8008B780,0xA2:0x8008BA3C,0xA9:0x8008B978}
# command, active effects, active music, pause flags, group, handle,
# mono/stereo/second/empty header, music owner, protected voice, base age
CASES=[]
for cmd in CALLBACKS:
    for mask,music,pause in ((0,0,0),(0xFFF000,0xA00505,0),(0xC00000,1,3)):
        CASES.append((cmd,mask,music,pause,0,0x405,0,0,0,7))
for group in (0,0xFF,0x2000000,0x40000000,0x80000000):
    for hold in (0,1):
        CASES.append((0x21,0xFFF000,0,0,group,5 if group==0x80000000 else 0x405,0,0,hold,7))
CASES += [(0x21,0xFFF000,0,0,0xFF,0xFFFF,0,0,0,7)]
for head in (0,1,2,3):
    for mask,group,pause in ((0,0,0),(0xA00000,0,0),(0xFFF000,0,0),(0xFFF000,0xFF,2)):
        CASES.append((0x24,mask,0xFFFF, pause,group,0x411,head,0,0,7))
for owner in (7,8,9):
    CASES.append((0xC0,0xFFF000,0x11,0,0,0,0,owner,0,7))
for handle in (0,7,8,9):
    CASES.append((0x11,0xFFF000,0xA00505,0,0,handle,0,0,0,7))
for duration,target,owner in ((0,0x40,0),(8,0x20,7),(8,0x7F,8),(16,0,9)):
    CASES.append((0xC1,0xFFF000,0xA00505,0,target,duration,0,owner,0,7))
CASES.append((0x24,0xFFF000,0,0,0,0x411,0,0,0,-2))
# Explicit ramp endpoints; preserve default +12=0x80 in the earlier cases.
CASES = [c + (0x80,) for c in CASES]
for duration in (0,1,3,0x10000,0x7FFFFFFF,0x80000000,0xFFFFFFFF):
    for first,last in ((0,127),(127,0),(64,64),(0xFFFFFF80,0xFFFFFFFF)):
        for owner in (0,7,8,9):
            CASES.append((0xC2,0xFFF000,0xA00505,0,first,duration,0,owner,0,7,last))
for cmd in (0xA0,0xA2):
    for active in (0,0xFFF000,0x801000):
        for group,handle in ((0,0x405),(0,0x999),(0xFF,0x405),(0x2000000,0),(0xFFFFFFFF,0)):
            for value in (0,1,127,128,255,0xFFFFFFFF):
                CASES.append((cmd,active,0xA00505,0,group,handle,0,0,0,7,value))
RANGES=((0xB6980,0xD0),(0xB8628,72),(0xB8A20,112),(0xB8AC0,0x42B8),
        (0x9D2C0,0x24),(0x9D2F4,4))
for active in (0,0xFFF000,0x801000):
    for duration in (0,1,30,255,32767,32768,65535,0xFFFFFFFF):
        for target in (0,1,127,255):
            CASES.append((0xA9,active,0,0,target,duration,0,0,0,7,0))
for active,group_handle,duration,target in itertools.product(
        (0,0xFFF000,0x801000),((0,0x405),(0,0x999),(0xFF,0x405),(0x2000000,0),(0xFFFFFFFF,0)),
        (0,1,30,255,32767,32768,65535,0xFFFFFFFF),(0,1,127,255)):
    group,handle=group_handle
    CASES.append((0xA1,active,0,0,group,handle,0,target,0,7,duration))

def fixture(exe,c):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
    def sh(a,v):struct.pack_into('<H',r,a,v&0xFFFF)
    for a,n in RANGES:r[a:a+n]=bytes(n)
    cmd,active,music,pause,group,handle,head,owner,hold,age,last=c
    sw(0x9D2C8,0x800B6980);sw(0x9D2C4,0x80);sw(0x9D2DC,pause);sw(0x9D2F4,1)
    for s in (0xB6980,0xB69E8):
        sw(s+4,music);sw(s+0x1C,music);sw(s+0x14,0xFFFFFF);sw(s+0x50,0xAABBCCDD)
    sh(0xB69D4,7);sh(0xB6A3C,8)
    for a in (0xBCD50,0xBCD54,0xBCD58,0xBCD5C,0xBCD60,0xBCD6C,0xBCD70,0xBCD74):sw(a,active)
    for i in range(48):
        a=0xB8AC0+i*0x11C;sw(a+0xF0,i%24);sw(a+0xF4,0x80000000)
    for i in range(12):
        a=0xBC000+i*0x11C;sw(a+0xF0,i+12);sw(a+0xF4,0x40000000)
        sw(a+0x28,0x400+i);sw(a+0x2C,(0 if i%3==0 else (0xFF if i%3==1 else 0x2000000)))
        sw(a+0x38,0x100000 if hold else 0);sw(a+0x50,age+i if age>=0 else age)
        if cmd in (0xA1,0xA9):sh(a+0xD8,(i*7919+0x8000)&65535)
    for i in range(16):r[0xB2900+i]=(i*17+3)&255
    r[0xB8A20:0xB8A90]=bytes((i+0x60)&255 for i in range(112))
    sh(0x148000,0xFFFF if head in (2,3) else 0)
    sh(0x148002,0xFFFF if head in (0,3) else 16)
    sw(0xB8628,cmd);sw(0xB862C,0x80148000 if cmd==0x24 else (0x1F3 if cmd==0xC0 else handle))
    sw(0xB8630,group);sw(0xB8634,last);sw(0xB8638,owner if cmd in (0xC0,0xC1,0xC2,0xA1) else 0x7F);sw(0xB863C,handle)
    assert struct.unpack_from('<I',r,0x9C0C0+cmd*4)[0]==CALLBACKS[cmd]
    return r

def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    assert hashlib.sha256(exe[0x8B780-0x10000+0x800:0x8B900-0x10000+0x800]).hexdigest()=='29e7f2ce0e7fc181da4e8f3051148e981e0eec846274ec987110fee5721a13f8'
    assert hashlib.sha256(exe[0x8B978-0x10000+0x800:0x8BA3C-0x10000+0x800]).hexdigest()=='73962a5103b98e1198bf3156c27802ec9e5826ae6e02a86694f6c3a1f53116d0'
    out=['/* Generated by pe_audio_commands_oracle.py --write-header. */',
         'static const uint32_t ATK19_audio_callbacks[][2]={']
    out += [f'    {{0x{k:X}u,0x{v:X}u}},' for k,v in CALLBACKS.items()]
    out += ['};','static const int64_t ATK19_audio_cases[][11]={']
    out += ['    {'+','.join(map(str,c))+'},' for c in CASES]
    out += ['};','static const uint64_t ATK19_audio_expected[]={']
    for i,c in enumerate(CASES):
        r=fixture(exe,c)
        initial=bytes(r) if c[0] in (0xC2,0xA0,0xA1,0xA2,0xA9) else None
        seen=set()
        execute(r,0x8008CA84,visited_pcs=seen)
        if c[0]==0xA1:
            for pc in seen|{pc+4 for pc in seen}:
                off=pc-0x80010000+0x800
                assert initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[off:off+4],('instruction source',hex(pc))
        if initial is not None:
            # Compare every persistent changed byte, excluding the original
            # CPU stack. Fail if C2 begins touching state outside the hash.
            outside=bytearray(r[:0x1F0000])
            for a,n in RANGES:outside[a:a+n]=initial[a:a+n]
            assert outside==initial[:0x1F0000], 'command writes outside compared RAM'
        h=fingerprint(r);out.append(f'    UINT64_C(0x{h:016X}),');print(i,hex(h))
    out+=['};']
    out+=['static const struct { uint32_t group,duration; uint64_t hash; } DAY1_audio_selected_fade_faults[]={']
    for group,duration in ((0,0x10000),(0,0xFFFF0000),(0xFF,0x10000),(0xFF,0xFFFF0000)):
        c=(0xA1,0xFFF000,0,0,group,0x405,0,127,0,7,duration)
        r=fixture(exe,c);regs=execute(r,0x8008CA84,stop_at=(0x8008B800,0x8008B8B0))
        assert regs[2]==0
        out.append(f' {{0x{group:X}u,0x{duration:X}u,UINT64_C(0x{fingerprint(r):016X})}},')
    out+=['};']
    out+=['static const struct { uint32_t duration; uint64_t hash; } DAY1_audio_fade_faults[]={']
    for duration in (0x10000,0xFFFF0000):
        c=(0xA9,0xFFF000,0,0,127,duration,0,0,0,7,0)
        r=fixture(exe,c);regs=execute(r,0x8008CA84,stop_at=(0x8008B9EC,))
        assert regs[2]==0
        out.append(f' {{0x{duration:X}u,UINT64_C(0x{fingerprint(r):016X})}},')
    out+=['};']
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_audio_commands_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(CASES)} original audio command call graphs')
if __name__=='__main__':main()
