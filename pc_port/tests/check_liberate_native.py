#!/usr/bin/env python3
"""Directed test-only differential for native Liberate callbacks.
Uses the existing e21 raw-retail oracle, unchanged. Additional leaf metadata is
in-memory only: no matched-src/config claim. Calls are mocked on both sides,
so success proves caller control/memory/ordered ABI, not actual rendering/callees.
"""
from pathlib import Path
import argparse
import json
import os
import sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/analysis'))
if os.environ.get('LIBERATE_PYLIB'):
 sys.path.insert(0,os.environ['LIBERATE_PYLIB'])
import e21_diff as e

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--seeds',type=int,default=64);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--native-frame',action='store_true');ns=ap.parse_args()
 # Use identical local addresses for this raw-layout draft (not the E21
 # compacted-local layout). Makes caller stack aliases compare exactly.
 if not ns.native_frame:
  e.HARNESS=e.HARNESS.replace('static pe_addr_t fsp = 0x801FEC00u;',f'static pe_addr_t fsp = 0x{e.SP0:08X}u;').replace('a[i] < 0x801FF000u','a[i] < 0x80200000u')
 os.environ.pop('E21_STACK_COINCIDE',None)
 e.OUT_EXTENTS[(0x800CE870,2)]=6 # real CE870(mode=0): writes three position halfwords
 old_run=e.run_mips
 metrics={'attempts':0,'faults':[]}
 def judged(*args,**kwargs):
  metrics['attempts']+=1
  prior=set(e.COVER) if e.COVER is not None else None
  try:return old_run(*args,**kwargs)
  except e.Fault as fault:
   metrics['faults'].append({'seed':args[3],'reason':str(fault)})
   if prior is not None:
    e.COVER.clear();e.COVER.update(prior)
   raise
 e.run_mips=judged
 old_parse=e.o.parse_overlay
 def parse(ovl):
  ov=old_parse(ovl)
  if ovl=='ovl_03C9':
   for addr,off,size in [(0x801EDC44,0x44c,0x950),(0x801EEBC8,0x13d0,0x998)]:
    name=f'func_{addr:08X}';ov['leaves'][name]={'name':name,'file_offset':off,'file_size':size,'vram':addr,'words':size//4,'nonmatching':True}
  return ov
 e.o.parse_overlay=parse
 specs=[
  'a0=0; a1=0x80140000; r(0x80071A54)=0x1234',
  'a0=1; a1=0x80140000; h(a1+0x10)=0,1,2,3,65535; h(a1+0x12)=0,1,15,16,17,23,24,47,48,32767,65535; r(0x800CE610)=0',
  'a0=1; a1=0x80140000; h(a1+0x10)=0,1,2; h(a1+0x12)=15,16,17,23,24,47,48; r(0x800CE610)=0x80150000; r(0x80071A54)=0x1234',
  'a0=2; a1=0x80140000; h(a1+0x10)=0,1,2,3,65535; h(a1+0x12)=0,1,15,16,17,23,24,47,48,32767,65535',
  'a0=3,4294967295; a1=0x80140000',
 ]
 common='; h(0x800E11E6)=0; w(0x800F33E0)=0x80162000; w(0x80162008)=0x80163000; w(0x800F32D0)=0x80160000; w(0x80160008)=0x80161000; w(0x800BCFA4)=0x80164000; w(0x8009D254)=0x80165000'
 results=[]
 for name in ('func_801EDC44','func_801EEBC8'):
  extra = (['a0=2; a1=0x80140000; w(0x800E27EC)=1; h(a1+0x10)=0,1; h(a1+0x12)=0,23,24,47,48; r(0x80077CF4)=4294967232'] if name=='func_801EDC44' else ['a0=1; a1=0x80140000; h(0x801F1F38)=0; h(a1+6)=0,1,2,3,6,7,8,15; r(0x800CE610)=0,0x80150000; r(0x80071A54)=0x1234,4294967295'])
  for i,spec in enumerate(specs+extra):
   spec += common
   p=ROOT/'pc_port/game/boot'/f'{name}_liberate_native.c'
   metrics['attempts']=0;metrics['faults']=[]
   code,detail=e.one(p,name,'ovl_03C9',ns.seeds,'cc',True,directed={('ovl_03C9',name):[e.parse_directed(spec)]},check_gte=True)
   result={'function':name,'spec_index':i,'spec':spec,'status':code,'detail':detail,'covered_pcs':sorted(e.COVER or []),'coverage_scope':'completed oracle cases only; equivalence contingent on status ok','frame_layout':'production arena' if ns.native_frame else 'same raw frame address','attempts':metrics['attempts'],'oracle_faults':list(metrics['faults'])};results.append(result)
   print(name,i,code,detail,flush=True)
   ns.output.write_text(json.dumps(results,indent=2)+'\n')
   if code!='ok':return 1
 return 0
if __name__=='__main__':raise SystemExit(main())
