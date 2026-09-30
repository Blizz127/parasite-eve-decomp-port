#!/usr/bin/env python3
"""Original effect-pump histories: real VM, paired flashes, pause and resume."""
import hashlib,struct,sys
from pe_eve_charge_oracle import ROOT,execute,find_disc,read_form1
from pe_scripted_exit_oracle import words
from pe_m0023i_effect_oracle import fixture as particle_fixture
from pe_m0023i_flash_oracle import fixture as flash_fixture,RANGES as FLASH_RANGES
from pe_pistol_effect_oracle import fixture as pistol_fixture
ENTRIES=(0x80069594,)
RANGES=tuple(x for x in FLASH_RANGES if x!=(0x1FEFB8,6))+((0x9D1A0,4),
 (0xE2248,4),(0xE22F8,24),(0xF3300,0x200))
CASES=[dict(entry=0,bank=bank,pause=pause) for bank in (0,1) for pause in (0,4,0x100)]
for bank in (0,1):
 for age in (0,1,10,127,128,255):
  CASES.append(dict(entry=0,bank=bank,pause=4,weapon=(1,),age=age))
 for weapon in ((),(0,),(2,),(1,2),(2,1),(1,1)):
  CASES.append(dict(entry=0,bank=bank,pause=4,weapon=weapon,age=10))
 for weapon in ((1,),(2,),(1,2),(2,1)):
  CASES.append(dict(entry=0,bank=bank,pause=4,weapon=weapon,age=255,after=True))
  CASES.append(dict(entry=0,bank=bank,pause=4,weapon=weapon,age=10,no_particles=True))
for bank in (0,1):
 for after in (False,True):
  for program in ('delay','loop','stop'):
   for club in (False,True):
    CASES.append(dict(entry=0,bank=bank,pause=4,weapon=(),age=0,
                     after=after,active_updates=program,club=club))
FRAMES=14

def fixture(exe,overlay,c):
 r,s,_,ctrl=flash_fixture(exe,overlay,dict(c,mode=2))
 pr,ps,_,_=particle_fixture(exe,overlay,dict(entry=0,particles=3,state=1,interval=2))
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
 def sh(a,v):put(a,struct.pack('<H',v&65535))
 put(0x150100,ps[0x150100:0x150110]);put(0x150200,ps[0x150200:0x1502FC])
 put(0x150140,ps[0x150100:0x150110]);put(0x150400,ps[0x150200:0x1502FC]);sh(0x15014E,16)
 for i in range(1,11):put(0x150000+i*0xA0C,b'\0')
 put(0x150000,bytes((2,0x76,1,0)))
 sw(0x942E0,0x80151C00);sw(0x942E4,0x80150000)
 sw(0x151D54,0x80151D60);sw(0x151D6C,0x800D4704);sw(0x151D70,0x800D413C)
 sw(0x15000C,0x80151E00);sw(0x150010,0x80150800)
 put(0x150018,b'\2\0');sh(0x15001A,0);sh(0x15001C,0);sh(0x15001E,0);sh(0x150020,2)
 sh(0x15002C,0);sh(0x15002E,0);sw(0x150030,0x80150100);sw(0x150034,0x80150200)
 for i in range(1,8):sh(0x15002C+i*12,65535)
 sh(0x150038,0);sh(0x15003A,0);sw(0x15003C,0x80150140);sw(0x150040,0x80150400)
 sw(0x15008C,0x80190720)
 # Synthetic effect-VM input: delay, initialize both original flashes, wait.
 commands=((4,1,0),(1,0,0),(2,1,0),(1,0,1),(2,1,0),(4,100,0),(0,0,0))
 for i,command in enumerate(commands):put(0x151E00+i*6,struct.pack('<3H',*command))
 if 'weapon' in c:
  # Original draw descriptors, with the retail 0x100 gate holding weapon
  # updates while a room slot in its allowed 55..72 range advances.
  # All room codes >=55 select the same descriptor. Slot/record order vary.
  put(0x150001,b'\x55')
  wr,_,_=pistol_fixture(exe,dict(entry=4,age=c['age'],bank=c['bank']))
  for a,n in ((0x966EC,0x4000),(0xC21A4,0x20),(0xE0AD8,0x60),
              (0x142000,0x1000),(0x9D254,4),(0xE22F8,24),
              (0xF34B8,24),(0xF34E4,2),(0xF337A,1),(0xF33AC,1),
              (0xF3422,6),(0xE224C,1)):
   put(a,wr[a:a+n])
  for a,n in ((0xE0A84,0x30),(0xE0F18,0x30)):
   put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
  # Nonzero padding word proves the muzzle writer is data, not a Z=0 guess.
  sw(0xC21C0,0x8765FB2E)
  if c.get('no_particles'):
   # No F004 prologue: the flash consumes all six weapon-written bytes.
   sw(0x150204,0);sw(0x150404,0)
  pool=0x150000 if c.get('after') else 0x150000-2*0xA0C
  sw(0x942E4,0x80000000+pool)
  weapon=0x150A0C if c.get('after') else pool
  hit=weapon+0xA0C
  for slot,code,draw,update,entry in ((weapon,2,0x800C9B68,0x800C9B90,0x151DA0),
                                   (hit,4,0x800CD8C8,0x800CD8F0,0x151DC0)):
   put(slot,bytes(512));put(slot,bytes((2,code,0,0)))
   sw(0x151C00+code*4,0x80000000+entry);sw(entry+12,draw);sw(entry+16,update)
  # Empty hit callbacks must preserve earlier writes, including when active.
  put(hit+128,bytes((0,1,0,0,0,0)))
  for i,code in enumerate(c['weapon']):
   put(weapon+128+i*6,struct.pack('<BBHh',code,1,0,i*64))
   put(weapon+512+i*64,wr[0x143200:0x143230])
   put(weapon+513+i*64,bytes(((c['age']+i)&255,)))
  if c.get('active_updates'):
   # Empty weapon/impact lists still run C251C and C2758. Their shallow
   # frames preserve the preceding room-update writer at pump SP-0xA4.
   for slot,script in ((weapon,0x151E40),(hit,0x151E60)):
    put(slot+128,bytes(384));sw(slot+8,struct.unpack_from('<I',r,0x9D254)[0])
    sw(slot+120,0x80000000+script)
    program=(0xFFFFFFFF,) if c['active_updates']=='stop' else (
        0x00100012,0x00200003,0x00020001,0x30FC0000)
    for j,op in enumerate(program):sw(script+j*4,op)
    if c['active_updates']=='delay':sh(slot+12,5)
  if c.get('club'):
   # CE144/CE16C use the same shared loops and shallow frame depths.
   # Leave the second slot inactive and use the original club descriptor.
   put(hit,b'\0');put(weapon+1,b'\x06')
   sw(0x151C18,0x80151DA0);sw(0x151DAC,0x800CE144);sw(0x151DB0,0x800CE16C)
   put(0xE0FB4,exe[0xE0FB4-0x10000+0x800:0xE1044-0x10000+0x800])
  assert struct.unpack_from('<I',r,0x9CDDC)[0]==c['bank']
 return r,s,(),ctrl

def frame_input(r,c,frame):
 flags=0x80|(0x100 if 'weapon' in c and not c.get('active_updates') else 0)|(c['pause'] if 4<=frame<=6 else 0)
 struct.pack_into('<I',r,0x9D1A0,flags);struct.pack_into('<I',r,0x9CDD8,0)
 # Fresh GPU output storage per fixture frame; effect data and borrowed CPU
 # stack bytes persist. This avoids comparing old undefined packet padding
 # after the next frame allocates fewer primitives.
 r[0x160000:0x164000]=bytes(0x4000)
 for bank in range(2):
  for i in range(4096):struct.pack_into('<I',r,0x164000+bank*0x4000+i*4,0xABFFFFFF)
 return flags

def fingerprint(r):
    r=bytearray(r);bank=struct.unpack_from('<I',r,0x9CDDC)[0]
    base=0x160000+bank*0x2000;size=struct.unpack_from('<I',r,0x9CDD8)[0];linked=set()
    for i in range(4096):
        p=struct.unpack_from('<I',r,0x164000+bank*0x4000+i*4)[0]&0xFFFFFF
        while base<=p<base+size and p not in linked:
            linked.add(p);p=struct.unpack_from('<I',r,p)[0]&0xFFFFFF
    p=base
    while p<base+size:
        length=r[p+3]
        assert length in (0,1,6,7,8,9,12),(hex(p),length)
        if length==0:p+=4;continue
        if length==6:
            r[p+15]=0;r[p+23]=0
        if length==8:
            for j in (15,23,31):r[p+j]=0
            if p not in linked:r[p+32:p+36]=bytes(4)
        if length in (1,6,8) and p not in linked:r[p:p+3]=bytes(3)
        if length==9 and (r[p+7]&0xFC)==0x2C:
            r[p+30:p+32]=bytes(2);r[p+38:p+40]=bytes(2)
            if p not in linked:r[p:p+3]=bytes(3);r[p+32:p+36]=bytes(4)
        p+=(length+1)*4
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 overlay=read_form1(find_disc(ROOT),13904,172)
 assert hashlib.sha256(overlay[0x8018F710-0x8018EFE8:0x8018FC14-0x8018EFE8]).hexdigest()=='6bee6882d0a259e2d326e4bd70941ff28d6622266e24d4f098dcb6cf5cc228aa'
 _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
 patches=[];cases=[]
 for k,c in enumerate(CASES):
  r,s,args,ctrl=fixture(exe,overlay,c);first=len(patches)
  patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  for frame in range(FRAMES):
   flags=frame_input(r,c,frame)
   regs=execute(r,0x80069594,initial_cop_control=ctrl,strict_gte_flags=True)
   if frame==2:
    assert sum(struct.unpack_from('<H',r,0x15002C+i*12)[0]==1 for i in range(8))==2, 'fixture must reach both original flashes'
   h=fingerprint(r);cases.append((k,frame,first,len(patches),flags,regs[2],h))
   print(k,frame,c,hex(regs[2]),hex(h),flush=True)
   if '--dump' in sys.argv:(ROOT/f'local/live/m0023i-pump-oracle-{k}-{frame}.bin').write_bytes(r)
 out=['/* Generated original M0023I effect-pump histories. */']
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_pump_{name}[][2]={{')
  out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
 out.append('static const struct { unsigned history,frame,first,end; uint32_t flags,result; uint64_t hash; } DAY1_pump_cases[]={')
 for k,f,a,b,flags,v,h in cases:
  out.append(f'    {{{k},{f},{a},{b},0x{flags:08X}u,0x{v:08X}u,UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_m0023i_pump_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(cases)} complete original effect-pump frames')
if __name__=='__main__':main()
