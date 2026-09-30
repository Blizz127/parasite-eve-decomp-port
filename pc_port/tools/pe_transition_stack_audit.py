#!/usr/bin/env python3
"""Observe original message-list loads and last stack writers, without providers."""
import json,struct
import pe_battle_hud_oracle as cpu
from pe_transition_update_oracle import fixture
from pe_m0000i_leaves_oracle import load_overlay

def main():
 source=cpu.ROOT.joinpath('pc_port/tools/pe_battle_hud_oracle.py').read_text()
 marker='        jump = None\n'
 assert source.count(marker)==1
 source=source.replace(marker,marker+'''        if op in (40,41,43) and 0x1FE800<=a<0x1FF000:
            for q in range(a,a+{40:1,41:2,43:4}[op]): writers[q]=pc
        if pc==0x800375E0:
            p=r[6]&0x1FFFFF
            observed.append(dict(return_pc=f'{r[31]:08X}',list_pc=f'{r[6]:08X}',
                values=list(struct.unpack_from('<5h',ram,p)),
                last_writers=[f'{writers.get(p+j,0):08X}' for j in range(10)]))
''')
 ns=dict(__file__=cpu.__file__,__name__='stack_observer',writers={},observed=[])
 exec(compile(source,cpu.__file__,'exec'),ns)
 ex,o,b=load_overlay();rows=[]
 for mode in (0,10):
  for fill in (0,165):
   for s2 in (0,1):
    r,_=fixture(ex,o,b,(0,mode,0,0,0,74 if mode else 0))
    r[0x1FE800:0x1FF000]=bytes((fill,))*0x800
    ns['writers'].clear();ns['observed'].clear()
    ns['execute'](r,0x801942FC,initial_regs={18:s2,19:0xFFFFFF})
    assert len(ns['observed'])==1
    row=ns['observed'][0]
    if mode:
     assert row['values'][:3]==[0,0,-1]
     assert row['last_writers'][2:6]==['8018F578']*2+['8018F574']*2
    else:
     assert row['values'][0]==0 and row['last_writers'][2:]==['00000000']*8
    rows.append(dict(mode=mode,stack_fill=fill,caller_s2=s2,message=row))
 print(json.dumps(dict(scope='original update last-writer audit; outer initialization/menu-tail provenance remains unresolved',cases=rows),indent=2))
if __name__=='__main__':main()
