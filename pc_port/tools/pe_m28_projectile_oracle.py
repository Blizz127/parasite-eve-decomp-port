#!/usr/bin/env python3
"""Verify m13/m28 projectile relocation and execute the complete m28 graph."""
import hashlib
import struct
import sys
from pe_m0013i_effect_oracle import ROOT,execute,find_disc,read_form1,fixture as previous_fixture,CASES,RANGES
from pe_m0013i_effect_oracle import EXTRA_RANGES as PREVIOUS_EXTRA
from pe_eve_charge_oracle import fingerprint as packet_fingerprint
from pe_scripted_exit_oracle import words

EXTRA_RANGES=tuple((0x193288,n) if a==0x18FCEC else (a,n) for a,n in PREVIOUS_EXTRA)
ENTRIES=(0x80192700,0x801924F8,0x800C6B90,0x800D3F64,0x800CE688,0x800CE78C,0x800187C0)


def relocation_check(old,new):
    # All 788 instruction words: only internal jumps and three data references differ.
    refs={0x22C:(0x24A5EFF4,0x24A5F1CC),0x254:(0x24A5EFFC,0x24A5F1D4),0x39C:(0x24E7F004,0x24E724F8)}
    jumps=0
    for offset in range(0,0xC50,4):
        a=struct.unpack_from('<I',old,0x1C+offset)[0]
        b=struct.unpack_from('<I',new,0x3510+offset)[0]
        if offset in refs:
            assert (a,b)==refs[offset];continue
        target=0x80000000|((a&0x3FFFFFF)<<2)
        if a>>26 in (2,3) and 0x8018F004<=target<0x8018FC54:
            a=(a&0xFC000000)|(((target+0x34F4)>>2)&0x3FFFFFF);jumps+=1
        assert a==b,('non-relocation difference',hex(offset),hex(a),hex(b))
    print('PASS: 788 original instruction words; three data relocations and',jumps,'internal jumps',flush=True)


def fixture(exe,overlay,case):
    r,s,args,ctrl=previous_fixture(exe,overlay,case)
    def put(a,data):r[a:a+len(data)]=data;s[a:a+len(data)]=data
    def w(a,v):put(a,struct.pack('<I',v))
    for a,n in ((0x18F1CC,16),(0x19151C,4),(0x1915AC,4),(0x191894,4),(0x1920D8,4),(0x193288,12)):
        put(a,overlay[a-0x18EFE8:a-0x18EFE8+n])
    w(0x150208,0x801924F8)
    if case['entry']==6:w(0x151E30,0x80193148)
    return r,s,args,ctrl


def fingerprint(r):
    value=packet_fingerprint(r)
    for a,n in EXTRA_RANGES:
        for b in r[a:a+n]:value=((value^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return value


def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    disc=find_disc(ROOT);old=read_form1(disc,12007,2);overlay=read_form1(disc,15016,69)
    assert hashlib.sha256(old).hexdigest()=='5be7c97af6b8fe3f51dc34350c6e16c2bc0a659e187ed7fd7aa5d6506ed063a1'
    assert hashlib.sha256(overlay).hexdigest()=='c15d03313a578f7c7ba07f255345df8e17734e38dd807162426d0bc3a7fd7e94'
    relocation_check(old,overlay)
    _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);patches=[];rows=[]
    for k,case in enumerate(CASES):
        r,s,args,ctrl=fixture(exe,overlay,case);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[case['entry']],args,initial_cop_control=ctrl)
        rows.append((case['entry'],first,len(patches),args,regs[2],fingerprint(r)))
    out=['/* Generated original M0028I projectile cases, including defined GPU packet fields. */']
    for name,data in (('ranges',RANGES),('extra_ranges',EXTRA_RANGES),('common',[(i*4,v) for i,v in enumerate(base) if v]),('patches',patches)):
        out.append(f'static const uint32_t DAY1_m28_projectile_{name}[][2]={{')
        out.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[3],result; uint64_t hash; } DAY1_m28_projectile_cases[]={')
    for kind,first,end,args,result,value in rows:
        params=','.join(f'0x{x&0xFFFFFFFF:X}u' for x in args)
        out.append(f' {{{kind},{first},{end},{{{params}}},0x{result:X}u,UINT64_C(0x{value:X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_m28_projectile_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:',len(rows),'complete original M0028I projectile cases',flush=True)


if __name__=='__main__':main()
