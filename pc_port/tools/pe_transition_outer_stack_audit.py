#!/usr/bin/env python3
"""Constructor last writers for the outer loop's borrowed menu-list tail.

Constructor provider calls remain explicit gaps in stack provenance. This
observation is deliberately not sufficient to bind the production menu tail.
"""
import json,struct,sys
import pe_battle_hud_oracle as cpu
import pe_transition_constructor_oracle as constructor
from pe_m0000i_leaves_oracle import load_overlay

def main():
 scratchpad='--scratchpad' in sys.argv
 stack=0x1F8003C8 if scratchpad else 0x801FEFD0
 menu=(stack-0x38)&0x1FFFFF
 if scratchpad:constructor.RANGES=constructor.RANGES+((0,1024),)
 source=cpu.ROOT.joinpath('pc_port/tools/pe_battle_hud_oracle.py').read_text()
 marker='        jump = None\n';assert source.count(marker)==1
 source=source.replace(marker,marker+'''        if op in (40,41,43) and (0x1FE000<=a<0x1FF000 or 0<=a<1024):
            for q in range(a,a+{40:1,41:2,43:4}[op]): writers[q]=pc
''')
 ns=dict(__file__=cpu.__file__,__name__='outer_stack_observer',writers={})
 exec(compile(source,cpu.__file__,'exec'),ns)
 def execute(r,entry,**kwargs):
  if entry==0x80196498:kwargs['initial_regs']={29:stack}
  return ns['execute'](r,entry,**kwargs)
 constructor.execute=execute
 ex,o,b=load_overlay();rows=[]
 for mode in (0,1,2,10):
  for fill in (0,165):
   r=constructor.fixture(ex,o,b,0,mode,0,0,0)
   r[0:1024] = bytes((fill,))*1024 if scratchpad else r[0:1024]
   r[0x1FE000:0x1FF000]=bytes((fill,))*0x1000
   ns['writers'].clear();constructor.original(r,0,0)
   p=menu
   assert [ns['writers'].get(p+j,0) for j in range(10)]==[0x80196F8C]*4+[0]*4+[0x80196EF8]*2
   assert r[p+4:p+8]==bytes((fill,))*4
   rows.append(dict(mode=mode,fill=fill,menu_address=hex(0x1F800000+p if scratchpad else 0x80000000+p),
    values=list(struct.unpack_from('<5h',r,p)),
    last_writers=[hex(ns['writers'].get(p+j,0)) for j in range(10)]))
 input_rows=[]
 for fill in (0,165):
  for state,flags,busy in ((0,0,255),(1,0x4000,255),(2,0x4000,255),(6,0x4000,255),(6,0xC000,255),(6,0xC000,1)):
   r=constructor.fixture(ex,o,b,0,0,0,0,0)
   r[0:1024] = bytes((fill,))*1024 if scratchpad else r[0:1024]
   r[0x1FE000:0x1FF000]=bytes((fill,))*0x1000
   ns['writers'].clear();constructor.original(r,0,0)
   def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
   r[0xA5B70:0xA5D50]=bytes(0x1E0)
   sw(0x9B738,0x80084B20);sw(0x9B740,0x80084F8C)
   sw(0xA5BA0,0x800BE9A0);sw(0x9D1A0,flags)
   r[0xA5BB9]=state;r[0xA5BB6]=busy
   struct.pack_into('<H',r,0xA5C56,1)
   struct.pack_into('<HH',r,0xBE9A0,0x4000,0xFFFF)
   execute(r,0x8003EB04,initial_regs={29:stack})
   p=menu
   assert r[p+4:p+8]==bytes((fill,))*4
   expected=0x80083BD0 if state==6 and flags==0xC000 else 0x80196F8C
   assert [ns['writers'].get(p+j,0) for j in range(4)]==[expected]*4
   if expected==0x80083BD0:assert struct.unpack_from('<I',r,p)[0]==0x8008291C
   input_rows.append(dict(fill=fill,pad_state=state,flags=hex(flags),busy=busy,
     values=list(struct.unpack_from('<5h',r,p)),
     last_writers=[hex(ns['writers'].get(p+j,0)) for j in range(10)]))
 print(json.dumps(dict(stack=hex(stack),scope='constructor provider gaps remain; subsequent original digital controller calls execute without providers',providers=[hex(x) for x in constructor.PROVIDERS],cases=rows,input_cases=input_rows),indent=2))
if __name__=='__main__':main()
