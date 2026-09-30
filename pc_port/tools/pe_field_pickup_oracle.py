#!/usr/bin/env python3
"""Original field item awards, chest selection, pickup windows and draw graphs.

--compare-native checks new A9/window/filter cases against full native RAM
below 1FE000, excluding the original execution stack. It also checks executed
instruction bytes and that fixture ranges include every original write.
--capture-only --capture PATH compares both A9 phases from the observed
m0334i chest context without injecting a capture into the connected route.
"""
import hashlib
import struct
import sys
import subprocess
import tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT, execute
from pe_inventory_entry_oracle import fixture as menu_fixture, RANGES
from pe_scripted_exit_oracle import words

ENTRIES=(0x800532B4,0x800194B0,0x8004F490,0x80015BAC,0x8004F644,
         0x8004F730,0x8004F798,0x8004F7D8,0x80062FEC,0x8005E30C,
         0x80019540,0x8004C34C,0x80055E14)
RESULT_ENTRIES={0,1,3,5,10,11}
CASES=[]
for item in (0,1,200,255,256,383,384,511,512,520,521,0xFFFFFFFF):
    CASES.append(dict(entry=0,item=item))
for item in (1,6,200,256):
    for full in (0,1):CASES.append(dict(entry=1,item=item,full=full))
for kind in (1,9,10,15):
    for bank in (0,1):
        CASES.append(dict(entry=2,kind=kind,bank=bank))
        CASES.append(dict(entry=8,kind=kind,bank=bank))
for kind in (1,9):
    for bank in (0,1):CASES.append(dict(entry=8,kind=kind,bank=bank,slots=3))
for flags in (0,32):
    for overlay in (0,0x1000):
        for kind in (1,10):CASES.append(dict(entry=3,flags=flags,overlay=overlay,kind=kind))
for entry in (4,6,7):
    for bank in (0,1):CASES.append(dict(entry=entry,bank=bank))
for event in (0,2,64,0x10000,0x10040):
    for special in (0,1):
        CASES.append(dict(entry=5,event=event,special=special))
        CASES.append(dict(entry=9,event=event,special=special))


for flags in (0,32,0xFFE0):
    for result in (0,1,2,3,4,258,386,387,32767,32768,65534,65535):
        CASES.append(dict(entry=10,flags=flags,result=result,item=0))
for existing in (0,1):
    for saved in (0,1,0x305,0xFFFFFFFF):
        for item in (0,6,200):CASES.append(dict(entry=11,existing=existing,saved=saved,item=item))
for count in (0,1,8,32,50):
    for protected in (0,0x20,0x40,0x80,0xE0):
        for equipped in ((0,1),(-1,-1)):
            CASES.append(dict(entry=12,count=count,protected=protected,equipped=equipped))


def fixture(exe,c):
    r,s,_=menu_fixture(exe,dict(entry=3,bank=c.get('bank',0)))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    def word(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    item=c.get('item',200);kind=c.get('kind',10)
    sw(0x142000,0x80142020);sw(0x142004,0x80142024);sw(0x142020,item);sw(0x142024,0xDEADBEEF)
    sw(0x9D300,0x80142100);sw(0x142108,c.get('flags',0));sw(0x142110,77)
    sw(0x9CE00,0x80143000);sw(0xB0CD8,c.get('overlay',0));sw(0x9D1A0,0x4040)
    record=0x154000+199*32
    sb(record+6,kind);sb(record+20,c.get('slots',0))
    sb(record+21,0);sb(record+22,1);sb(record+23,31)
    for offset,value in ((7,32),(8,27),(9,14)):sb(record+offset,value)
    for offset,value in ((14,-3),(16,4),(18,13)):sh(record+offset,value)
    sw(0x9CF58,0x80000000+record);sw(0x9CF38,c.get('special',0))
    if c.get('full'):
        for i in range(word(0x9D050)):sh(word(0x9D048)+i*2,6)
    if c['entry']>=10:
        sh(0x9D2A4,c.get('result',0));sw(0x9CF98,c.get('saved',0))
        if c['entry']==11 and c.get('existing'):execute(r,0x8004C34C,(17,))
        if c['entry']==12:
            count=c.get('count',8);sw(0x9D050,count)
            sb(0xC0E20,c['equipped'][0]);sb(0xC0E22,c['equipped'][1])
            ids=(256,257,6,200,512,0,65535,383)
            for i in range(count):sh(word(0x9D048)+i*2,ids[i%len(ids)])
            for id in (256,257,6,200,512,383):
                p=execute(r,0x800532B4,(id,))[2]
                sb(p+5,c['protected'])
    window=0
    if c['entry'] in (5,8,9):
        execute(r,0x8004F490,(item,))
        window=word(0x9D154)
        while word(window+36)==19:window=word(window)
    if c['entry']==9:
        q=0x80140500;sw(q,0);sw(q+4,1);sw(q+8,c['event'])
        sw(0x9D0E0,q);sw(0x9D0E4,q);sw(0x9D0EC,1)
    args=((item,),(0x80142000,),(item,),(0x80142000,),(),
          (window,c.get('event',0)),(),(),(),(),(0x80142000,),(item,),())[c['entry']]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,args


def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


NATIVE = r'''#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc!=5)return 2;PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);
 D_8009D1A0=PE_LoadU32(0x8009D1A0u);D_8009D018=PE_LoadU32(0x8009D018u);
 D_8009D03C=PE_LoadU32(0x8009D03Cu);
 D_8009D048=PE_LoadU32(0x8009D048u);D_8009D04C=PE_LoadU32(0x8009D04Cu);
 D_8009D050=PE_LoadU32(0x8009D050u);D_8009D054=PE_LoadU32(0x8009D054u);
 D_8009D058=PE_LoadU32(0x8009D058u);D_8009D064=PE_LoadU32(0x8009D064u);
 unsigned result=0,a=strtoul(argv[4],0,0);
 if(atoi(argv[3])==10)result=func_80019540(a);
 else if(atoi(argv[3])==11)result=func_8004C34C(a);else func_80055E14();
 if(PE_Port_ShouldStop())return 4;
 PE_StoreU32(0x8009D1A0u,D_8009D1A0);
 PE_StoreU32(0x8009D048u,D_8009D048);PE_StoreU32(0x8009D050u,D_8009D050);
 PE_StoreU32(0x8009D058u,D_8009D058);PE_StoreU32(0x8009D064u,D_8009D064);
 f=fopen(argv[2],"wb");if(!f)return 5;
 fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);fclose(f);
 printf("%u\n",result);return 0;
}'''


def main():
    exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,s,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    temporary=tempfile.TemporaryDirectory(prefix='pe-chest-selection-')
    temp=Path(temporary.name);binary=temp/'native';inp=temp/'in.bin';out_ram=temp/'out.bin'
    compare_native='--compare-native' in sys.argv
    if compare_native:
        build=Path(sys.argv[sys.argv.index('--build-dir')+1]) if '--build-dir' in sys.argv else ROOT/'pc_port/build'
        source=temp/'native.c';source.write_text(NATIVE)
        subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],
                        str(source),str(build.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
    selected=[] if '--capture-only' in sys.argv else CASES
    for k,c in enumerate(selected):
        r,s,args=fixture(exe,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        initial=bytes(r);seen=set()
        regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=900000,visited_pcs=seen)
        if c['entry']>=10:
            for pc in seen|{p+4 for p in seen}:
                offset=pc-0x80010000+0x800
                assert 0x800<=offset<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[offset:offset+4],('instruction',hex(pc))
            covered=bytearray(0x1FE000)
            for a,n in RANGES:covered[a:a+n]=bytes([1])*n
            assert all(covered[i] or r[i]==initial[i] for i in range(0x1FE000)),('uncovered write',c)
            if compare_native:
                inp.write_bytes(initial)
                native_result=int(subprocess.check_output([str(binary),str(inp),str(out_ram),str(c['entry']),str(args[0] if args else 0)]))
                native=out_ram.read_bytes()
                expected=regs[2] if c['entry'] in RESULT_ENTRIES else 0
                assert native_result==expected,('result',c,hex(native_result),hex(expected))
                if native[:0x1FE000]!=r[:0x1FE000]:
                    raise AssertionError((c,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:30]))
        result=regs[2] if c['entry'] in RESULT_ENTRIES else 0;h=fingerprint(r)
        cases.append((c['entry'],first,len(patches),args,result,h))
        print(k,c,hex(result),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'local/live/pickup-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_field_pickup_oracle.py; original instruction execution. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t SEW17_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[2],result; uint64_t hash; } SEW17_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:
        assert selected, 'do not replace fixtures in capture-only mode'
        (ROOT/'pc_port/tests/retail_field_pickup_cases.h').write_text('\n'.join(out)+'\n')
    captured=0
    if '--capture' in sys.argv:
        assert compare_native, '--capture requires --compare-native'
        capture=Path(sys.argv[sys.argv.index('--capture')+1])
        r=bytearray(capture.read_bytes());assert len(r)==0x200000
        print('capture',capture,'SHA256',hashlib.sha256(r).hexdigest(),flush=True)
        # Reconstruct the two operands and call context of the observed A9.
        # This is an isolated comparison; captured RAM never advances a route.
        assert struct.unpack_from('<4I',r,0x19292C)==(0x1040A9,0,0,8)
        sw=lambda a,v:struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
        for step in range(2):
            sw(0x9D300,0x8009D3C0);sw(0x9D2F0,0x800BF490);sw(0x9CE00,0x8019293C)
            sw(0x120F80,0x80192934);sw(0x120F84,0x800BF55C)
            initial=bytes(r);inp.write_bytes(initial);seen=set()
            native_result=int(subprocess.check_output([str(binary),str(inp),str(out_ram),'10',str(0x80120F80)]))
            regs=execute(r,0x80019540,(0x80120F80,),instruction_budget=900000,visited_pcs=seen)
            for pc in seen|{p+4 for p in seen}:
                offset=pc-0x80010000+0x800
                assert 0x800<=offset<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[offset:offset+4],('captured instruction',hex(pc))
            native=out_ram.read_bytes()
            assert native_result==regs[2],('captured result',step,native_result,regs[2])
            if native[:0x1FE000]!=r[:0x1FE000]:
                raise AssertionError(('captured RAM',step,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:30]))
            captured+=1
            print('PASS: captured A9 phase',step,flush=True)
    temporary.cleanup()
    print('PASS:',len(cases),'complete original field pickup cases; native comparisons',sum(c['entry']>=10 for c in selected) if compare_native else 0, 'captured phases',captured)


if __name__=='__main__':main()
