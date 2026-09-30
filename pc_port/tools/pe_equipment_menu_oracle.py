#!/usr/bin/env python3
"""Original equipment filtering, cursor restoration, construction and properties."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_pe_cost_oracle import fixture as cost_fixture,RANGES
from pe_scripted_exit_oracle import words
ENTRIES=(0x800542A0,0x800543CC,0x80059EC8,0x80064B74,0x80064C20,0x80045EE4,0x8004542C,0x80046378,0x8004F9A0,0x80050AD8,0x8005E988,0x8004551C,0x80045670,0x80045A98)
RESULT_ENTRIES=set()
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for mask in (0,1,2,62,510,512,1023,0x80000000,0xFFFFFFFF):
 for bank in (0,1):
  for entry in (0,1):case(entry,mask=mask,bank=bank)
for kind in (0,1,5,8,9,10,19,31,32,33,255):
 for entry in (0,1):case(entry,kind=kind,mask=0x80000203)
for excluded in (-1,0,1,2,9,10):case(1,excluded=excluded)
for slot in (0,1,2,0xFFFFFFFF):
 for bank in (0,1):case(2,slot=slot,bank=bank,index=-1)
for row in (-1,0,1,8,127):
 for restore in (0,1):
  for columns in (0,1):case(3,row=row,restore=restore,columns=columns)
for index in (-1,0,18,20):case(3,index=index)
case(4)
case(3,null_list=1);case(4,null_list=1)
for entry in (5,6,7,8):
 for weapon in (0,1):
  for upgrade in (0,1):
   for profile in (0,1):case(entry,weapon=weapon,upgrade=upgrade,profile=profile)
for focus in (0,1):
 for mask_empty in (0,1):case(7,focus=focus,mask_empty=mask_empty)
for index in (-1,0,1,2,4,20):
 for weapon in (0,1):case(9,index=index,weapon=weapon)
for property in (0,1,31,32,255):case(9,property=property)

for old in (-2147483648,-1,0,1,2147483647):
 for new in (-2147483648,-1,0,1,2147483647):case(10,old=old,new=new)
for entry in (11,12,13):
 for weapon in (0,1):
  for upgrade in (0,1):
   for focused in (0,1):case(entry,weapon=weapon,upgrade=upgrade,focused=focused)
for bonus in (-32768,-256,-1,0,1,998,999,1000,32767):
 for missing in (0,1,2):case(12,bonus=bonus,missing=missing)
for bonus in (-32768,0,1000,32767):case(11,bonus=bonus)

def fixture(exe,c):
 r,s,_=cost_fixture(exe,dict(entry=2,battle=c.get('battle',0)))
 def put(a,b):a&=0x1fffff;r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,v):put(a,struct.pack('<I',v&0xffffffff))
 def sh(a,v):put(a,struct.pack('<H',v&65535))
 def sb(a,v):put(a,bytes((v&255,)))
 def word(a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
 def find(kind,id):
  p=word(0x9d154)
  while p:
   if (word(p+32),word(p+36))==(kind,id):return p
   p=word(p)
  raise AssertionError((kind,id))
 bank=c.get('bank',0);execute(r,0x80052E30,(bank,))
 selected=word(0x9d048)
 for i,value in enumerate((256,257,6,512,0,258,259,260,261,65535)):sh(selected+i*2,value)
 sw(0x9d050,10)
 for i,kind in enumerate((c.get('kind',1),9,0,5,8,9)):
  p=0xC0EAC+i*32;sb(p+6,kind);sb(p+20,2 if i!=4 else 0);sb(p+21,c.get('property',3));sb(p+22,0)
 if c.get('mask_empty'):
  for i in range(10):sh(selected+i*2,0)
 sw(0x9d090,0 if c.get('weapon',1) else 1);sw(0x9d094,1)
 sw(0x9d098,bank);sw(0x9d09c,bank)
 execute(r,0x80062F9C);sw(0x9cef0,0x3f);execute(r,0x800438EC)
 main=find(2,0);sw(0x9cf18,c.get('weapon',1));sw(0x9cf1c,c.get('upgrade',0));sw(0x9cf0c,c.get('profile',0))
 node=main
 if c['entry'] in (3,4):
  sw(node+88,3);sw(node+104,c.get('columns',0));sw(node+100,0);sw(0x9d16c,c.get('restore',0))
  sb(0xa3060+c.get('index',18)*4,1);sb(0xa3061+c.get('index',18)*4,c.get('row',1));sb(0xa3062+c.get('index',18)*4,2)
 if c['entry'] in (7,8,11,12,13):
  execute(r,0x80045EE4,(main,));execute(r,0x8004542C,(main,))
  if c['entry']==7:
   node=find(2,5)
   if c.get('upgrade'):execute(r,0x80062D2C,(47,0,0,0))
  else:node=find(2,6)
  if c['entry']>=11:
   node=find(1,5)
   if c.get('focused'):
    window=execute(r,0x80062D2C,(7,0,0,0))[2]
    target=execute(r,0x8006322C,(7,window,window))[2]
    execute(r,0x80062CB8,(target,))
 for i in range(3):
  sb(0xc0eb3+i,120+i);sh(0xc0eba+i*2,c.get('bonus',50)+i)
  sb(0xc0ed3+i,100+i);sh(0xc0eda+i*2,c.get('bonus',25)-i)

 if c['entry']==9:sw(0x9cf20,0x800c0eac)
 sw(0x9d100,0x80160000);sw(0x9d104,0x80160000)
 if c.get('null_list'):node=0
 args=((c.get('mask',510),),(c.get('mask',510),c.get('excluded',0)),(c.get('slot',0),c.get('index',2)),(node,c.get('index',18)),(node,),(main,),(main,),(node,c.get('focus',0)),(node,),(c.get('index',0),),(c.get('old',0),c.get('new',0)),
       (0x800c0eac,), (0 if c.get('missing')==1 else 0x800c0eac,0 if c.get('missing')==2 else 0x800c0ecc),(node,))[c['entry']]
 for a,n in RANGES:s[a:a+n]=r[a:a+n]
 return r,s,tuple(v&0xffffffff for v in args)


def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,s,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args=fixture(exe,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],args)
        result=regs[2] if c['entry'] in RESULT_ENTRIES else 0;h=fingerprint(r)
        cases.append((c['entry'],first,len(patches),args,result,h))
        print(k,c,hex(result),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/equipment-menu-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_equipment_menu_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV21_equipment_menu_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } INV21_equipment_menu_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_equipment_menu_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original equipment menu cases')

if __name__=='__main__':main()
