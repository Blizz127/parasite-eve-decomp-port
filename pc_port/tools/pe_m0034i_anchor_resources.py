#!/usr/bin/env python3
"""Real M0034I model publication, initialization, pose and attachment slices.

Original pose stops before lighting; VM stops at its first following ALU call.
No substituted data for the model, clips, joint points or attachment command.
"""
import hashlib,json,struct,sys
from pe_day2_route_audit import ROOT,parse_field_table,extract_script_from_package,decode_script,find_disc,read_form1
from pe_battle_hud_oracle import execute
BASE=0x8018EFE8
NATIVE_RANGES=((0x150000,0x300),(0x160000,0x11000),(BASE&0x1FFFFF,204800),(0xB1638,32),(0x9CDDC,4))
def native_hash(r):
 h=14695981039346656037
 for a,n in NATIVE_RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rec=next(r for r in parse_field_table(ex) if r['map_id']==34);meta=rec['meta'];prefix=((meta&255)+(meta>>8&4095))*2048
 pkg=read_form1(find_disc(ROOT),1013+rec['start'],prefix//2048+(meta>>20));chunk=pkg[prefix:]
 assert hashlib.sha256(chunk).hexdigest()=='0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
 script_offset,raw=extract_script_from_package(pkg,meta);script=decode_script(raw)
 assert script['sha256']=='dc224a516c3d0eaaf16d87b03bc1dc622ebca19ef419a6a2d51c8dfe8daa3603'
 script_base=BASE+script_offset-prefix;start=script_base+0x40CC
 assert script['modules'][4]['start']==0x40CC
 native_header="--native-header" in sys.argv or "--check-native-header" in sys.argv
 lighting="--lighting" in sys.argv or native_header
 native_rows=[]
 cases=[];seen=set();clips=None
 for packets in (0,1):
  for command in (2,4,5,6,7,8,9,11,12,13,15,16,17,18):
   for fraction in (0,1,2):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(chunk)]=chunk;scratch=bytearray(0x400)
    def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    def lh(a):return struct.unpack_from('<H',r,a&0x1FFFFF)[0]
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
    def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
    def call(entry,args=(),**kw):return execute(r,entry,args,scratchpad=scratch,visited_pcs=seen,instruction_budget=1000000,**kw)
    header=BASE+lw(BASE+4)
    call(0x8006B7B0,initial_regs={18:0,20:BASE,21:header,22:0x800B0CD8},stop_at=(0x8006B898,))
    obj=lw(0x800B0E7C);assert obj==BASE+0x8268 and r[(obj&0x1FFFFF)+2]==52
    published=[i for i in range(48) if lw(0x800B0E98+576+i*4)]
    assert published==[2,4,5,6,7,8,9,11,12,13,15,16,17,18];clips=published
    actor=0x80150000;model=actor+0x1B4;child=0x80151000;task=0x80152000
    call(0x8003D050,(model,obj,0x80160000,0,0,0,0,0,0,packets))
    init_hash=native_hash(r) if native_header else 0
    if '--dump-model' in sys.argv and command==2 and fraction==0:(ROOT/f'local/live/m34-original-init-{packets}.bin').write_bytes(r)
    points=lw(model+0x18);matrices=lw(model+0x84)
    assert points==BASE+0xC224 and matrices==(0x8016FC24 if packets else 0x80160084)
    r[0x15000C]=3;call(0x8001A680,(actor,command));clip=lw(actor+0x1B0)
    cap=r[0x15000F];frame=(0,cap//2,cap)[fraction]
    # Pose production is complete before this lighting callee; packet-mode
    # lighting later reaches NCCT, unsupported by this original runner.
    if lighting:
     for i,v in enumerate((4096,0,0,0,4096,0,0,0,4096)):sh(0x800BEA40+i*2,v)
     sw(0x8009CDA0,0x00808080)
     call(0x8003D834,(model,clip,frame,0x800BEA40),initial_cop_control={16:4096,18:4096,20:4096})
    else:call(0x8003D834,(model,clip,frame,0x800BEA40),stop_at=(0x8003B97C,))
    if native_header:native_rows.append((packets,command,frame,init_hash,native_hash(r)))
    if '--dump-model' in sys.argv and command==2 and fraction==0:(ROOT/f'local/live/m34-original-pose-{packets}.bin').write_bytes(r)
    packet_hash=hashlib.sha256(r[0x160000:lw(model+0x80)&0x1FFFFF]).hexdigest()
    matrix=matrices+49*32;point=points+49*16
    matrix_bytes=bytes(r[matrix&0x1FFFFF:(matrix&0x1FFFFF)+32])
    point_values=list(struct.unpack_from('<4h',r,point&0x1FFFFF))
    # Supply actor/task publication, but retain original model/pose data.
    sw(0x8009D20C,actor);sw(actor+4,child);sw(child+4,0);sw(child+0x9C,start)
    r[0x15100C]=4;sw(0x8009D2F0,child);sw(0x8009D300,task);sw(0x8009D1A0,0)
    sw(task,start);sw(task+16,1);sw(0x800A7930,0x12345678)
    for i,v in enumerate((4096,0,0,0,4096,0,0,0,4096)):sh(0x800B89F8+i*2,v)
    assert r[start&0x1FFFFF:(start&0x1FFFFF)+20]==raw[0x40CC:0x40E0]
    call(0x80017018,stop_at=(0x80012850,),initial_cop_control={26:256,24:160<<16,25:112<<16})
    assert lw(child+0x18C)==actor and lw(child+0x1B4)==0 and lw(child+0x1D8)==model and lh(child+0x1E6)==49
    assert lw(child+0x98)&0x2000 and lw(0x800B6AA8)==0x12345678
    assert lw(0x8009CE00)==script_base+0x4108 and lw(task)==start and lw(task+16)==0
    assert [lw(child+0x28+i*4) for i in range(3)]==[lh(child+0x254+i*2)<<16 for i in range(3)]
    cases.append(dict(packets=packets,command=command,frame=frame,cap=cap,clip=f'{clip:08X}',matrix_sha256=hashlib.sha256(matrix_bytes).hexdigest(),point=point_values,world=[lh(child+0x254+i*2) for i in range(3)],projected=[lw(child+0x210),lw(child+0x218)]))
    if lighting:cases[-1]['packet_sha256']=packet_hash
 for first,second in zip(cases[:42],cases[42:]):
  assert all(first[k]==second[k] for k in ('command','frame','cap','matrix_sha256','point','world','projected'))
 assert {0x8006B804,0x8006B87C,0x8003D050,0x80039B74,0x8003A088,0x80015108,0x8003DF50,0x8003E188,0x800173F4}<=seen
 if lighting:assert {0x8003BBDC,0x8003BCE0}<=seen
 out=dict(script_sha256=script['sha256'],chunk_sha256=hashlib.sha256(chunk).hexdigest(),script_base=f'{script_base:08X}',model=f'{BASE+0x8268:08X}',joint_count=52,clips=clips,scope='original resource publication window, complete model init with texture adjustment disabled, pose prefix before lighting, real module4 VM prefix before first ALU; supplied memory placement/actor list/task/view; no live constructor/render/scene claim',cases=cases)
 if lighting:out['scope']='original complete model init with texture adjustment disabled, complete3D834 including lighting and both packet buffers, real VM prefix before ALU; supplied lighting/view/actor/task/allocation, no GPU submission/live claim'
 if native_header:
  lines=['/* Original M0034I model init and complete3D834 hashes. Requires original disc. */',f'#define M34_CHUNK_LBA {1013+rec["start"]+prefix//2048}u','static const uint32_t M34_ranges[][2]={']
  lines.extend(f'{{0x{a:X}u,0x{n:X}u}},' for a,n in NATIVE_RANGES);lines.append('};')
  lines.append('static const struct { unsigned packets,command,frame; uint64_t init,pose; } M34_cases[]={')
  lines.extend(f'{{{p},{c},{f},UINT64_C(0x{i:016X}),UINT64_C(0x{h:016X})}},' for p,c,f,i,h in native_rows);lines.append('};')
  header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0034i_model_cases.h'
  if '--check-native-header' in sys.argv:assert target.read_text()==header
  else:target.write_text(header)
 path=ROOT/('local/live/m0034i-anchor-lighting.json' if lighting else 'local/live/m0034i-anchor-resources.json');path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS {len(cases)} real M0034I model/pose/attachment slices; {path.relative_to(ROOT)}')
if __name__=='__main__':main()
