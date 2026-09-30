#!/usr/bin/env python3
"""Closed M0351I script regions executed through original MIPS handlers.

Argument binding is an explicit pointer-bank fixture. No unknown opcode is
skipped. Regions stop before a transfer/yield or the next story block; this
is not execution of the preceding asynchronous scene or transfer handler.
"""
import hashlib,json,struct
from pe_day2_route_audit import ROOT,SCRIPTS,parse_field_table,extract_script_from_package,decode_script,decode_packed_name,find_disc,read_form1
from pe_battle_hud_oracle import execute
HANDLERS={0:0x80017294,5:0x8001731C,9:0x80012850,10:0x800173F4,0x11:0x800130B4,0xD2:0x80019DB8,0xD3:0x80019DF4}
SPANS=((0x17294,0x172BC),(0x1731C,0x1735C),(0x12850,0x12C20),(0x173F4,0x17410))
def load():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rec=next(r for r in parse_field_table(ex) if r['map_id']==351)
 c0=rec['meta']&255;c1=rec['meta']>>8&4095;c2=rec['meta']>>20
 package=read_form1(find_disc(ROOT),1013+rec['start'],c0+c1+c2)
 off,raw=extract_script_from_package(package,rec['meta']);script=decode_script(raw)
 assert script['sha256']==SCRIPTS[351]
 base=0x8018EFE8+off-(c0+c1)*2048
 return ex,raw,script,base

def run(ex,raw,script,base,start,stop,story,component13=0,component14=0,timer=None,pressed=0,persist1=0x3E6,persist8=0):
 ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
 ram[base&0x1FFFFF:(base&0x1FFFFF)+len(raw)]=raw
 def sw(a,v):struct.pack_into('<I',ram,a&0x1FFFFF,v&0xFFFFFFFF)
 def lw(a):return struct.unpack_from('<I',ram,a&0x1FFFFF)[0]
 module=script['modules'][1];module_base=base+module['start']
 sw(0x8009D2F0,0x80152000);sw(0x8015209C,module_base)
 sw(0x800A7918,story);sw(0x800A77F4,persist1)
 sw(0x800A7810,persist8);sw(0x8009D1F4,pressed)
 if timer is not None:sw(0x800A76BC,timer)
 sw(0x80153000+13*4,component13);sw(0x80153000+14*4,component14)
 commands={base+c['offset']:c for c in module['commands']};pc=start;trace=[];writes=[]
 for _ in range(200):
  if pc in stop:return dict(stop=f'{pc:08X}',story=lw(0x800A7918),persist1=lw(0x800A77F4),value311=lw(0x800A7CCC),components=[lw(0x80153000+i*4) for i in (13,14,15)],trace=trace,writes=writes)
  c=commands[pc];op=c['opcode'];assert op in HANDLERS, f'unresolved opcode {op:X} at {pc:X}'
  next_pc=pc+8+4*c['argc'];sw(0x8009CE00,next_pc)
  for i,(mode,value) in enumerate(zip(c['modes'],c['args'])):
   if mode==0:addr=pc+8+i*4
   elif mode==1:addr=0x80153000+value*4
   elif mode==2:addr=0x800A77F0+value*4
   elif mode==3:addr=0x8009DF70+value*4
   else:raise AssertionError(f'unresolved binder mode {mode}')
   sw(0x80150000+i*4,addr)
  regs=execute(ram,HANDLERS[op],(0x80150000,));assert regs[2]==1
  trace.append(f'{pc:08X}')
  if op==10 and c['modes'][0]==2:writes.append(dict(pc=f'{pc:08X}',index=c['args'][0],value=lw(0x800A77F0+c['args'][0]*4)))
  pc=lw(0x8009CE00)
 raise AssertionError('closed region did not terminate')

def main():
 ex,raw,script,base=load();assert base+script['modules'][1]['start']==0x8018F4C8
 handler_hash=hashlib.sha256(b''.join(ex[a-0xF800:b-0xF800] for a,b in SPANS)).hexdigest()
 assert handler_hash=='d77f3345b511d05f01bc227f60d3ad61af0e2e6c28361447459716ff26868b9a'
 commands={base+c['offset']:c for c in script['modules'][1]['commands']}
 for pc,destination in ((0x80191124,'m0042i'),(0x80191168,'m0092i')):
  command=commands[pc]
  assert command['opcode']==0x31 and command['modes']==[0]
  assert decode_packed_name(command['args'][0]).lower()==destination
 # Both sides of every calculation branch are retained. No feasibility
 # assumption is needed to prove the final constant write dominates exit.
 start,end,final=0x8018F7C8,0x8018FB88,0x8018FB78
 edges={}
 for pc,c in commands.items():
  if not start<=pc<end:continue
  assert c['opcode'] in HANDLERS
  following=pc+8+c['argc']*4
  if c['opcode']==0:
   assert c['modes']==[0];successors=[base+script['modules'][1]['start']+(c['args'][0]<<1)]
  elif c['opcode']==5:
   assert c['modes'][1]==0;successors=[following,base+script['modules'][1]['start']+(c['args'][1]<<1)]
  else:successors=[following]
  assert all(pc<q<=end and (q in commands or q==end) for q in successors)
  edges[pc]=successors
 def reaches_exit(omit=None):
  todo=[start];seen=set()
  while todo:
   pc=todo.pop()
   if pc==omit or pc in seen:continue
   if pc==end:return True
   seen.add(pc);todo.extend(edges[pc])
  return False
 assert reaches_exit() and not reaches_exit(final)
 assert commands[final]['opcode']==10 and commands[final]['modes']==[2,0] and commands[final]['args']==[311,800]
 dominance=dict(entry=f'{start:08X}',exit=f'{end:08X}',dominating_write=f'{final:08X}',commands=len(edges),forward_only=True)
 selectors=[]
 stories=list(range(0x301))+[0x7FFFFFFF,0x80000000,0xFFFFFFFF]
 for story in stories:
  result=run(ex,raw,script,base,0x801910DC,{0x80191124,0x80191168,0x80191174},story)
  signed=story if story<0x80000000 else story-0x100000000
  expected=('80191124',0x88,0) if signed<=0x80 else ('80191168',0x140,0x3E6) if story==0x138 else ('80191174',story,0x3E6)
  assert (result['stop'],result['story'],result['persist1'])==expected
  selectors.append(dict(input_story=f'{story:08X}',**result))
 calculations=[]
 for first in (-32768,-1,0,1,10,32767):
  for second in (-32768,-1,0,39,40,41,49,50,59,60,69,70,99,100,101,32767):
   result=run(ex,raw,script,base,0x8018F7C8,{0x8018FB88},0x80,first,second)
   assert result['value311']==800 and result['writes'][-1]==dict(pc='8018FB78',index=311,value=800)
   calculations.append(dict(local13=first,local14=second,**result))
 timer_handler_hash=hashlib.sha256(ex[0x19DB8-0xF800:0x19F04-0xF800]).hexdigest()
 assert timer_handler_hash=='f684e1f70afe44e8a2195316e020111fecf4c6ad3f99e2e1e3b0ad297662effe'
 timer_cases=[];gated_timer_cases=[]
 timer_values=(0,1,59,60,3600,216000,219660,0x7FFFFFFF,0x80000000,0xFFFFFFFF)
 for n in range(256):
  timer=timer_values[n] if n<len(timer_values) else (n*2654435761)&0xFFFFFFFF
  result=run(ex,raw,script,base,0x8018F770,{0x8018FB88},0x80,timer=timer)
  assert result['value311']==800
  assert result['trace'][:6]==['8018F770','8018F780','8018F790','8018F7A0','8018F7B0','8018F7C8']
  timer_cases.append(dict(timer_count=timer,**result))
  gated=run(ex,raw,script,base,0x8018F694,{0x8018FB88},0x80,timer=timer,pressed=0x100,persist1=0,persist8=1)
  assert gated['trace'][:11]==['8018F694','8018F6A8','8018F6C0','8018F6D0','8018F6E8','8018F6F8','8018F710','8018F720','8018F738','8018F748','8018F760']
  assert gated['trace'][11:]==result['trace']
  assert gated['components']==result['components'] and gated['writes']==result['writes'] and gated['value311']==800
  gated_timer_cases.append(dict(timer_count=timer,**gated))
 input_handler_hash=hashlib.sha256(ex[0x130B4-0xF800:0x131E8-0xF800]).hexdigest()
 assert input_handler_hash=='41ee2a1659cb0f8d1905ad71fb9f8b99408d6647c1a55127e789122ae7e25bbd'
 gates=[]
 gate_stops={0x8018F770,0x8018FB88,0x80190BC8,0x80191054,0x801910C8,0x80191174}
 for pressed in (0,1,0xFF,0x100,0x101,0x80000100,0xFFFFFFFF):
  for persist1 in (0,0x3E5,0x3E6,0x3E7,0xFFFFFFFF):
   for persist8 in (0,1,2,0x7FFFFFFF,0x80000000,0xFFFFFFFF):
    for story in (0x7F,0x80,0x81,0x138,0xFFFFFFFF):
     result=run(ex,raw,script,base,0x8018F694,gate_stops,story,pressed=pressed,persist1=persist1,persist8=persist8)
     expected=(0x80191174 if not pressed&0x100 else
               0x801910C8 if persist1==0x3E6 else
               0x80191054 if persist8==0 else
               0x80190BC8 if persist8&0x80000000 else
               0x8018F770 if story==0x80 else 0x8018FB88)
     assert result['stop']==f'{expected:08X}',(pressed,persist1,persist8,story,result)
     assert result['story']==story and result['persist1']==persist1 and not result['writes']
     gates.append(dict(pressed=pressed,input_persist1=persist1,input_persist8=persist8,input_story=story,**result))
 out=dict(scope='closed Day1-to-Day2 selector and Day1 calculation regions; original handlers with pointer-bank binding contract; no asynchronous scene/transfer execution',script_sha256=script['sha256'],handler_spans=[[f'{a:08X}',f'{b:08X}'] for a,b in SPANS],handler_sha256=handler_hash,selector_cases=selectors,calculation_cases=calculations,calculation_dominance=dominance,timer_chain_cases=timer_cases,timer_handler_sha256=timer_handler_hash,input_gate_cases=gates,input_handler_sha256=input_handler_hash,gated_timer_chain_cases=gated_timer_cases)
 path=ROOT/'local/live/day2-entry-paths.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print('handler SHA256',handler_hash)
 print(f'PASS {len(selectors)} original-handler selector cases; {len(calculations)} calculation cases')
 print(f'PASS {len(timer_cases)} original D2/D3-to-calculation script chains')
 print(f'PASS {len(gates)} original input/state gate paths, all six exits')
 print(f'PASS {len(gated_timer_cases)} input-gated timer-to-calculation chains')
 print(path)
if __name__=='__main__':main()
