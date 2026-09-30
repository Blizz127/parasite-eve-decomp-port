#!/usr/bin/env python3
"""Trace original M34 projectile stack dependencies without altering a route.

This diagnostic stops the original update at its projectile initializer,
then varies its retained stack translations in isolated MIPS executions.
It does not assert that any supplied stack is a retail gameplay stack.
"""
import argparse,hashlib,inspect,json,struct
from collections import Counter,deque
from pathlib import Path
import pe_battle_hud_oracle as m
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1
ROOT=m.ROOT

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('capture',type=Path);p.add_argument('--frame',action='store_true',help='execute full original 35558 from a pre-effect capture');p.add_argument('--frames',type=int,default=1,help='maximum isolated original frames to search');p.add_argument('--prefix-input-frame',type=int,help='include original 3F404..3F4F8 prefix and supply the route digital pad replies, starting at this present-hook frame');p.add_argument('--counter-prefix',action='store_true',help='include original counter/mailbox prefix 3F4D0..3F4F8; excludes input, graphics and frame tail');p.add_argument('--watch-entry-sp',type=lambda x:int(x,0),help='report six words relative to this prospective F434 entry SP even if initializer is not reached');p.add_argument('--draw-prefix',action='store_true',help='execute original outer drawing fragment 3F4F8..3F590, excluding presentation');args=p.parse_args()
 assert sum((args.counter_prefix,args.prefix_input_frame is not None,args.draw_prefix))<=1
 assert not args.draw_prefix or args.frames==1
 assert args.frame or not(args.counter_prefix or args.prefix_input_frame is not None or args.draw_prefix)
 raw=args.capture.read_bytes();assert len(raw)==0x200000
 ex=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 ov=read_form1(find_disc(ROOT),16597,100);assert hashlib.sha256(ov).hexdigest()=='0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
 source=bytearray(0x200000);source[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];source[0x18efe8:0x18efe8+len(ov)]=ov
 r=bytearray(raw);writes={};phase='';seen=set();counts=Counter();tail=deque(maxlen=12)
 def watch(pc,regs,ram):
  counts[pc]+=1;tail.append(hex(pc))
  word=struct.unpack_from('<I',ram,pc&0x1fffff)[0];op=word>>26
  if op not in (40,41,42,43,46,58):return
  rs=word>>21&31;im=word&65535;im=im if im<32768 else im-65536;a=(regs[rs]+im)&0x1fffff
  if op in (42,46):a&=~3
  for i in range({40:1,41:2}.get(op,4)):writes[a+i]=(phase,hex(pc))
 # Observe stores in the existing interpreter; no instruction is replaced.
 code=inspect.getsource(m.execute).replace('        w = struct.unpack_from','        watch(pc,r,ram)\n        w = struct.unpack_from',1)
 namespace=dict(m.__dict__);namespace['watch']=watch;exec(code,namespace);run=namespace['execute']
 if args.frame:
  assert args.frames>0
  gte={};scratch=bytearray(1024)
  for frame in range(args.frames):
   phase='frame35558:'+str(frame);counts.clear();tail.clear()
   entry=0x80035558;stops=(0x8018f434,);initial_regs=None
   if args.counter_prefix:entry=0x8003F4D0;stops=(0x8018f434,0x8003F4F8)
   if args.draw_prefix:
    entry=0x8003F4F8;stops=(0x8003F590,0x8003F5F4,0x8003F678,0x8003F68C)
    initial_regs={16:0x800B0CD8,17:0x800BCFE8,18:0x800B0CEA}
   if args.prefix_input_frame is not None:
    # External digital-pad reply only, matching the route's M34 input pulses.
    struct.pack_into('<HH',r,0xBE9A0,0x4100,0xBFFF if (args.prefix_input_frame+frame)%8==3 else 0xFFFF)
    entry=0x8003F404;stops=(0x8018f434,0x8003F4F8)
    initial_regs={16:0x800B0CD8,17:0x800BCFE8,18:0x800B0CEA}
   try:
    regs=run(r,entry,stop_at=stops,scratchpad=scratch,visited_pcs=seen,instruction_budget=3000000,initial_regs=initial_regs,
             initial_cop_data=dict(enumerate(gte.get('data',[]))),initial_cop_control=dict(enumerate(gte.get('control',[]))),final_gte=gte)
   except AssertionError as error:
    raise AssertionError(dict(phase=phase,last_pcs=list(tail),frequent_pcs=[(hex(pc),n) for pc,n in counts.most_common(8)])) from error
   if regs[31]==0x800C2CE4:break
 else:
  phase='constructor';run(r,0x8018f00c,(0x801861a0,),visited_pcs=seen)
  phase='update';regs=run(r,0x8018f12c,(0x801861a0,),stop_at=(0x8018f434,),scratchpad=bytearray(1024),visited_pcs=seen,instruction_budget=500000)
 prefix_pcs=len(seen)
 if args.watch_entry_sp is not None:
  for pc in seen|{pc+4 for pc in seen}:
   a=pc&0x1FFFFF;assert raw[a:a+4]==source[a:a+4],('instruction',hex(pc))
  dependencies=[]
  for off in (-76,-72,-68,-36,-32,-28):
   a=(args.watch_entry_sp+off)&0x1FFFFF
   dependencies.append(dict(offset=off,value=struct.unpack_from('<I',r,a)[0],writers=[writes.get(a+i) for i in range(4)]))
  print(json.dumps(dict(capture_sha256=hashlib.sha256(raw).hexdigest(),initializer_reached=regs[31]==0x800C2CE4,prefix=('3F4F8..3F590' if args.draw_prefix else '3F4D0..3F4F8' if args.counter_prefix else '3F404..3F4F8' if args.prefix_input_frame is not None else '35558' if args.frame else 'constructor/update'),watched_entry_sp=hex(args.watch_entry_sp),searched_frames=frame+1 if args.frame else None,prefix_pcs=prefix_pcs,dependencies=dependencies),indent=2))
  return
 assert regs[31]==0x800c2ce4,dict(reason='initializer not reached',searched_frames=frame+1 if args.frame else None,return_address=hex(regs[31]),prefix_pcs=prefix_pcs,last_pcs=list(tail))
 sp=regs[29];data=regs[6];offsets=(-76,-72,-68,-36,-32,-28)
 dependencies=[]
 for off in offsets:
  a=(sp+off)&0x1fffff
  dependencies.append(dict(offset=off,value=struct.unpack_from('<I',r,a)[0],writers=[writes.get(a+i) for i in range(4)]))
 cases=[]
 for i in range(7):
  ram=bytearray(r)
  if i:struct.pack_into('<I',ram,(sp+offsets[i-1])&0x1fffff,1000)
  original=bytes(ram)
  m.execute(ram,0x8018f434,initial_regs=dict(enumerate(regs)),stop_at=(regs[31],),scratchpad=bytearray(1024),visited_pcs=seen,instruction_budget=500000)
  for pc in seen|{pc+4 for pc in seen}:
   a=pc&0x1fffff;assert original[a:a+4]==source[a:a+4],('instruction',hex(pc))
  cases.append(dict(changed_offset=None if not i else offsets[i-1],translation=struct.unpack_from('<3i',ram,(data+0x38)&0x1fffff)))
 print(json.dumps(dict(capture_sha256=hashlib.sha256(raw).hexdigest(),prefix=('3F404..3F4F8' if args.prefix_input_frame is not None else '3F4D0..3F4F8' if args.counter_prefix else '35558') if args.frame else 'constructor/update',searched_frames=frame+1 if args.frame else None,prefix_pcs=prefix_pcs,initializer='8018F434',original_entry_sp=hex(sp),data=hex(data),dependencies=dependencies,cases=cases,pcs=len(seen)),indent=2))
if __name__=='__main__':main()
