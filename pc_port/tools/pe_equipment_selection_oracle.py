#!/usr/bin/env python3
"""Original equipment commits, cross-bank eligibility and list drawing."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_equipment_menu_oracle import fixture as menu_fixture,RANGES
from pe_scripted_exit_oracle import words
ENTRIES=(0x80059534,0x8005968C,0x80054520,0x800509E0,0x8004FA10,0x8004FB48,0x800430A0,0x8004F978,0x8004FFD0,0x80045D0C,0x8004620C,0x800466C0,0x80046574,0x80043DA4,0x80062FEC,0x8005E30C,0x80044E98)
RESULT_ENTRIES={1,2,9,10,11,13,16}
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for entry in (0,1):
 for battle in (0,1):
  for index in (-1,0,1,2,8):case(entry,battle=battle,index=index)
for old_extra in (0,1,2,4):
 for new_extra in (0,1,2,4):
  for filled in (0,1):case(1,old_extra=old_extra,new_extra=new_extra,filled=filled,index=1)
for old in (-1,0,1):
 for battle in (0,1):case(0,old=old,battle=battle);case(1,old=old,battle=battle)
for mask in (0,1,2,62,256,510,512,830,0xFFFFFFFF):
 for bank in (0,1):
  for alternate in (0,1):case(2,mask=mask,bank=bank,alternate=alternate)
for entry in (3,4,5,6,7,8):
 for weapon in (0,1):
  for upgrade in (0,1):case(entry,weapon=weapon,upgrade=upgrade)
for index in (-1,0,1,3,8,10):
 for enabled in (0,1):case(6,index=index,enabled=enabled)
for old in (-1,0,1):case(3,old=old);case(7,old=old)
for alternate in (0,1):
 for bank in (0,1):case(5,alternate=alternate,bank=bank,bank_window=1)
for tools in (0,1,2):case(6,upgrade=1,tools=tools)

for entry in (9,10,11):
 for event in (0,64,0x1000,0x4000,0x10000):
  for battle in (0,1):case(entry,event=event,battle=battle)
for entry in (11,12,13,14,15):
 for weapon in (0,1):
  for battle in (0,1):case(entry,weapon=weapon,battle=battle)
for confirmed in (0,1):
 for weapon in (0,1):case(12,confirmed=confirmed,weapon=weapon)
for entry in (9,10,11):
 for enabled in (0,1):case(entry,event=64,profile=1,bank_window=1,enabled=enabled)
for event in (0,64,0x10000):
 for selection in (0,1):case(16,event=event,selection=selection,battle=1)

def fixture(exe,c):
 r,s,opening=menu_fixture(exe,dict(c,entry=7,focused=1,focus=1,weapon=c.get('weapon',c['entry']!=1)))
 execute(r,0x80046378,opening)
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
 sw(0x9D010,0);sw(0x9D014,0x800A1AA0)
 sb(0xC0E20,c.get('old',0));sb(0xC0E22,c.get('old',1))
 def reserve(a,n):
  sb(a+20,1 if n else 0)
  if n:sb(a+21,{1:8,2:9,4:10}[n])
 if 'old_extra' in c:
  reserve(0xC0ECC,c['old_extra']);reserve(0xC0F4C,c['new_extra'])
  if c.get('filled'):
   for i in range(10):
    if not struct.unpack_from('<H',r,0xC0E48+i*2)[0]:sh(0xC0E48+i*2,6)
 if not c.get('alternate',1):sw(0x9D04C,0)
 if c.get('bank_window'):
  w=execute(r,0x80062D2C,(54,0,0,0))[2];n=execute(r,0x8006322C,(54,w,w))[2]
  sw(n+68,0);sw(n+72,c.get('bank',0))
 if c['entry']==4:
  w=execute(r,0x80062D2C,(53,0,0,0))[2];node=execute(r,0x8006322C,(53,w,w))[2]
  sw(0x9d094,1)
 elif c['entry'] in (3,7):node=find(2,5)
 elif c['entry']==8:node=find(2,27)
 else:node=find(2,7)
 sw(0x9CEF4,node);sw(0x9CF34,c.get('enabled',0));sw(0x9CF38,0)
 if 'tools' in c:
  sw(0xA1888,c['tools']);sw(0xA188C,0);sw(0xA1890,0);sw(0xA1894,0)
 window=0
 if c['entry']>=9:
  node=find(2,7);columns=word(node+52);index=c.get('index',1)
  sw(node+68,index%columns);sw(node+72,index//columns)
  if c['entry'] in (9,10,11,12,15):
   which=5 if c['entry']==9 else 6 if c['entry']==10 else 7
   node=find(2,which);window=find(1,which);execute(r,0x80062CB8,(node,))
  if c['entry']==13:
   execute(r,0x80062F9C);execute(r,0x800438EC)
   node=find(2,0);window=find(1,0);sw(node+68,0);sw(node+72,2 if c.get('weapon',1) else 3)
  if c['entry']==14:
   execute(r,0x80062CB8,(find(2,5),))
  if c['entry']==15:
   q=0x80140500;sw(q,0);sw(q+4,1);sw(q+8,c.get('event',64))
   sw(0x9D0E0,q);sw(0x9D0E4,q);sw(0x9D0EC,1);sw(0x9D0E8,0)
  if c['entry']==16:
   execute(r,0x800466C0,(find(1,7),0x10000))
   window=find(1,41);node=find(2,41);sw(node+68,c.get('selection',0));sw(node+72,0)
 else:window=0
 args=((c.get('index',1),),(c.get('index',1),),(c.get('mask',510),),(0,),(node,),(node,),(c.get('index',0),),(node,),(node,),(window,c.get('event',0x10000)),(window,c.get('event',0x10000)),
       (window,c.get('event',0x10000)),(window,c.get('confirmed',1)),(window,c.get('event',0x10000)),(),(),(window,c.get('event',0x10000)))[c['entry']]
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
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/equipment-selection-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_equipment_selection_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV22_equipment_selection_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } INV22_equipment_selection_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_equipment_selection_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original equipment menu cases')

if __name__=='__main__':main()
