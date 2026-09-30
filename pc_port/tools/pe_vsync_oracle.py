#!/usr/bin/env python3
"""Original VSync + wait instructions; injected device reads and BIOS services.

No callee is replaced. Transcript includes all global/device operations but
excludes private stack traffic. Native code must consume the identical trace.
"""
import hashlib,struct,sys,json
from pe_m0000i_leaves_oracle import load_overlay
import pe_battle_hud_oracle as cpu
WATCH={0x80094574,0x80094578,0x8009457C,0x80094580,0x800956AC,0x1F801814,0x1F801110}
def main():
 scratchpad="--scratchpad-audit" in sys.argv
 stack_rows=[]
 ex,o,b=load_overlay();digest=hashlib.sha256(ex[0x73A44-0xF800:0x73C54-0xF800]).hexdigest();print('SHA256',digest,flush=True);assert digest=='e356692b0a159f0f9e07da321a2ea515c4789379094ff39d6d77f42858169af4'
 source=cpu.ROOT.joinpath('pc_port/tools/pe_battle_hud_oracle.py').read_text()
 marker='        jump = None\n';assert source.count(marker)==1
 source=source.replace(marker,marker+'''        if op in (40,41,43) and 0x390<=a<0x39A:
            for q in range(a,a+{40:1,41:2,43:4}[op]): stack_writers[q]=pc
        raw_address=(r[rs]+si)&0xFFFFFFFF
        if op==35 and raw_address in WATCH:
            value=device_read(raw_address)
            struct.pack_into('<I',ram,a,value&0xFFFFFFFF)
            record(0,raw_address,value)
        if op==43 and raw_address in WATCH: record(1,raw_address,r[rt])
''')
 marker='        if pc == 0xA0:\n';assert source.count(marker)==1
 source=source.replace(marker,'''        if pc in (0xB0,0xC0):
            assert (pc,r[9]) in ((0xB0,0x3F),(0xB0,0x5B),(0xC0,0xA))
            record(2,(pc<<8)|r[9],r[4],r[5] if pc==0xC0 else 0)
            pc=r[31]
            continue
'''+marker)
 ns=dict(__file__=cpu.__file__,__name__='vsync_device_observer',WATCH=WATCH,stack_writers={});exec(compile(source,cpu.__file__,'exec'),ns)
 all_events=[];rows=[]
 for n in range(264):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
  initial=(0,1,65535,0x7FFFFFFE,0xFFFFFFFF)[n%5]
  sw(0x80094574,0x1F801814);sw(0x80094578,0x1F801110);sw(0x8009457C,(0,65535,0x80000000,12345)[n%4]);sw(0x80094580,initial+(n%3)-1)
  counter_reads=timer_reads=gpu_reads=0;events=[]
  def record(kind,a,v,extra=0):
   event=(kind,a,v&0xffffffff,extra&0xffffffff)
   if events and events[-1][1:]==event:events[-1]=(events[-1][0]+1,*event)
   else:events.append((1,*event))
  def read(a):
   nonlocal counter_reads,timer_reads,gpu_reads
   if a==0x800956AC:
    counter_reads+=1
    return (initial+(0 if n>=256 else counter_reads//2))&0xffffffff
   if a==0x1F801110:
    sequence=(100,101,102,102,200,201,202,202) if n&1 else (65535,65535,1,1)
    value=sequence[min(timer_reads,len(sequence)-1)];timer_reads+=1;return value
   if a==0x1F801814:
    gpu_reads+=1;return (0x400000 if n&2 else 0)|(0x80000000 if gpu_reads>=4 else 0)
   return struct.unpack_from('<I',r,a&0x1fffff)[0]
  ns.update(device_read=read,record=record)
  ns["stack_writers"].clear()
  if scratchpad:r[:1024]=bytes((165,))*1024
  mode=(-1,-2,1,0,2,3,4,8)[n%8] if n<256 else 0
  regs=ns['execute'](r,0x80073A44,(mode,),instruction_budget=2000000,initial_regs={29:0x1F8003C8} if scratchpad else None)
  if scratchpad:
   stack_rows.append(dict(case=n,mode=mode,values=list(struct.unpack_from("<5h",r,0x390)),writers=[hex(ns["stack_writers"].get(0x390+j,0)) for j in range(10)]))
   assert r[0x394:0x398]==bytes((165,))*4
  first=len(all_events);all_events.extend(events);rows.append((mode,first,len(all_events),regs[2]))
 out=['/* Original VSync device/BIOS transcripts, run-length encoded. */','static const uint32_t DAY1_vsync_events[][5]={']
 out.extend(' {'+','.join(f'0x{x&0xffffffff:X}u' for x in e)+'},' for e in all_events);out.append('};')
 out.append('static const struct { int32_t mode; uint32_t first,end,result; } DAY1_vsync_cases[]={')
 out.extend(f' {{{m},{a},{z},0x{v:X}u}},' for m,a,z,v in rows);out.append('};')
 text='\n'.join(out)+'\n'
 if '--write-header' in sys.argv:(cpu.ROOT/'pc_port/tests/retail_vsync_cases.h').write_text(text)
 else:assert (cpu.ROOT/'pc_port/tests/retail_vsync_cases.h').read_text()==text
 if scratchpad:(cpu.ROOT/'local/live/vsync-stack-audit.json').write_text(json.dumps(stack_rows,indent=2)+'\n')
 print(f'PASS {len(rows)} original VSync cases; {len(all_events)} RLE events, including timeouts')
if __name__=='__main__':main()
