#!/usr/bin/env python3
"""Offline fixed-control-flow translation; no opcode decoder is linked at runtime.
Reads the locally extracted ovl_03C9; emits native C with literal gotos and direct
host calls. Caller differential proof is separate from production scheduling/callee/visual/audio
coverage. No src/ match claim; regeneration requires renewed differential review.
"""
import hashlib
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
BASE = 0x801ED7F8
REG = ['zero','at','v0','v1','a0','a1','a2','a3','t0','t1','t2','t3','t4','t5','t6','t7','s0','s1','s2','s3','s4','s5','s6','s7','t8','t9','k0','k1','gp','sp','fp','ra']
# Exact native signatures, not a generic dispatch ABI. Voids remain void.
# An optional third field names a host replacement called instead of func_<addr>.
PROTO = {
 0x8006DDCC: ('int', 'int a0, int a1, short a2, short a3, short a4'),
 0x80071A54: ('unsigned int', 'void'),
 0x80077A64: ('int','int tp, int abr, int x, int y'),
 0x80077AA4: ('int','int a0, int a1'),
 0x80077CF4: ('int','int a0'),
 0x80077DC4: ('int','int a0'),
 0x80078CC4: ('pe_addr_t','pe_addr_t matrix, pe_addr_t scale'),
 0x80079754: ('pe_addr_t','pe_addr_t r, pe_addr_t m','PE_RotMatrixYXZ_Guest'),
 0x800C6EC0: ('void','unsigned int first, unsigned int second'),
 0x800C6ED8: ('void','unsigned int value'),
 0x800C6EF8: ('void','pe_addr_t pe_record'),
 0x800C6F4C: ('void','pe_addr_t pe_record'),
 0x800C6FA0: ('void','pe_addr_t mesh, uint32_t level'),
 0x800C71E4: ('void','pe_addr_t mesh, pe_addr_t matrix'),
 0x800CE560: ('uint32_t','pe_addr_t pool, uint32_t size, int32_t count, pe_addr_t fn'),
 0x800CE610: ('pe_addr_t','pe_addr_t pool'),
 0x800CE870: ('void','pe_addr_t actor, int32_t mode, pe_addr_t out'),
 0x800CE8F0: ('void','pe_addr_t actor, uint32_t joint, pe_addr_t local, pe_addr_t out'),
 0x800CEDA8: ('void','int32_t texture'),
 0x800CEE20: ('void','pe_addr_t position, pe_addr_t angles, int32_t sx, int32_t sy, int32_t texture, uint32_t clut, int32_t abr, int32_t brightness, pe_addr_t color'),
 0x800CF3AC: ('void','pe_addr_t curve, pe_addr_t out, int32_t time'),
 0x800CFB7C: ('void','pe_addr_t angles, int32_t distance, pe_addr_t out'),
 0x800D004C: ('void','pe_addr_t position, int32_t radius0, int32_t radius1, int32_t segments, pe_addr_t angles, int32_t sx, int32_t sy, pe_addr_t center_color, pe_addr_t edge_color, int32_t brightness, int32_t abr'),
 0x800D0728: ('void','pe_addr_t position, int32_t inner, int32_t outer, int32_t segments, pe_addr_t angles, int32_t sx, int32_t sy, pe_addr_t inner_color, pe_addr_t outer_color, int32_t brightness, int32_t abr'),
 0x800D1384: ('void','pe_addr_t head0, pe_addr_t head1, uint32_t n, pe_addr_t col0, pe_addr_t col1, int32_t scale, pe_addr_t trail, int32_t abr'),
 0x800D1AE0: ('void','pe_addr_t a0, int a1, int a2, int a3'),
 0x800D2370: ('void','pe_addr_t position, pe_addr_t angles, int32_t length, int32_t width, int32_t u, int32_t v, int32_t uw, int32_t vh, uint32_t clut, pe_addr_t start_color, pe_addr_t end_color, int32_t brightness, int32_t abr'),
}

def callee(addr):
 ent=PROTO[addr]
 return (ent[2] if len(ent)>2 else f'func_{addr:08X}'),ent[0],ent[1]

def lift(blob, off, size, frame):
 words = {BASE+i:struct.unpack_from('<I',blob,i)[0] for i in range(off,off+size,4)}
 calls=sorted({((w&0x3ffffff)<<2)|0x80000000 for w in words.values() if w>>26==3})
 header=['/* Native callback: ovl_03C9 native fixed-control-flow translation.',' * Generated offline by pc_port/tests/lift_liberate_native.py from verified local',' * retail bytes. This is compiled C: no instruction fetch, CPU loop, CPU state',' * structure, runtime opcode decoding, interpreter or JIT. Callee calls are direct.',' * Source is behavior-equivalent native C, not matched decompilation.',' * Full scheduling, callee, visual/audio and playable-slice proof remain separate.',' */','#include "pe_guest_decomp.h"','', '/* host prototypes */']
 for addr in calls:
  name,ret,params=callee(addr);header += [f'{ret} {name}({params});']
 header+=['','static uint32_t liberate_lwl(uint32_t p,uint32_t old)', '{', '    unsigned shift=(3u-(p&3u))*8u;', '    uint32_t mask=0xFFFFFFFFu<<shift;', '    return (old&~mask)|(PE_LoadU32(p&~3u)<<shift);','}', 'static uint32_t liberate_lwr(uint32_t p,uint32_t old)','{','    unsigned shift=(p&3u)*8u;','    uint32_t mask=0xFFFFFFFFu>>shift;','    return (old&~mask)|(PE_LoadU32(p&~3u)>>shift);','}', 'static void liberate_swl(uint32_t p,uint32_t v)','{','    unsigned shift=(3u-(p&3u))*8u;','    uint32_t mask=0xFFFFFFFFu>>shift;','    PE_StoreU32(p&~3u,(PE_LoadU32(p&~3u)&~mask)|(v>>shift));','}', 'static void liberate_swr(uint32_t p,uint32_t v)','{','    unsigned shift=(p&3u)*8u;','    uint32_t mask=0xFFFFFFFFu<<shift;','    PE_StoreU32(p&~3u,(PE_LoadU32(p&~3u)&~mask)|(v<<shift));','}','',f'int func_{BASE+off:08X}(int32_t mode, pe_addr_t data)','{', f'    pe_addr_t frame = PE_GuestFrame_Enter({frame}u);']
 used=set()
 def r(i):
  if not i:return '0u'
  used.add(i);return REG[i]
 def assign(i,val):return f'{r(i)} = (uint32_t)({val});' if i else '(void)0;'
 def simple(pc):
  w=words[pc];op=w>>26;rs=w>>21&31;rt=w>>16&31;rd=w>>11&31;sh=w>>6&31;fn=w&63;imm=w&65535;si=imm if imm<32768 else imm-65536
  A=r(rs);B=r(rt);addr=f'({A} + (uint32_t)({si}))'
  if w==0:return '(void)0;'
  if op==0:
   if fn in (0,2,3):return assign(rd, [f'{B} << {sh}u', f'{B} >> {sh}u',f'(int32_t){B} >> {sh}u'][[0,2,3].index(fn)])
   if fn==33:return assign(rd,f'{A} + {B}')
   if fn==37:return assign(rd,f'{A} | {B}')
   if fn==35:return assign(rd,f'{A} - {B}')
   if fn==24:return f'product = (uint64_t)((int64_t)(int32_t){A} * (int32_t){B}); hi = (uint32_t)(product >> 32);'
   if fn==16:return assign(rd,'hi')
  if op==9:return assign(rt,f'{A} + (uint32_t)({si})')
  if op in (10,11):return assign(rt,f'{"(int32_t)" if op==10 else ""}{A} < {si if op==10 else "(uint32_t)(%d)"%si}')
  if op in (12,13):return assign(rt,f'{A} {"&" if op==12 else "|"} 0x{imm:X}u')
  if op==15:return assign(rt,f'0x{imm<<16:08X}u')
  if op in (33,35,37):return assign(rt, (f'(int32_t)(int16_t)PE_LoadU16({addr})' if op==33 else f'PE_LoadU{32 if op==35 else 16}({addr})'))
  if op in (41,43):return f'PE_StoreU{16 if op==41 else 32}({addr}, {B});'
  if op in (34,38):return assign(rt,f'liberate_{"lwl" if op==34 else "lwr"}({addr}, {B})')
  if op in (42,46):return f'liberate_{"swl" if op==42 else "swr"}({addr}, {B});'
  if op==18:
   if w&(1<<25):return f'PE_GTE_Command(0x{w&0x1ffffff:X}u);'
   if rs in (0,2):return assign(rt,f'PE_GTE_Read{"Data" if rs==0 else "Ctrl"}({rd}u)')
   if rs in (4,6):return f'PE_GTE_Write{"Data" if rs==4 else "Ctrl"}({rd}u, {B});'
  raise ValueError(f'unsupported {pc:08X} {w:08X}')
 lines=[];delay=set()
 for pc,w in words.items():
  op=w>>26;rs=w>>21&31;rt=w>>16&31;fn=w&63;si=w&65535;si=si if si<32768 else si-65536
  control=op in (1,2,3,4,5) or (op==0 and fn==8)
  if pc in delay:continue
  lines += [f'L{pc:08X}:;']
  if not control:
   lines += ['    '+simple(pc)];continue
  delay.add(pc+4)
  if op in (1,4,5):
   cond=f'(int32_t){r(rs)} {">=" if rt&1 else "<"} 0' if op==1 else f'{r(rs)} {"==" if op==4 else "!="} {r(rt)}'
   lines += [f'    take = ({cond});','    '+simple(pc+4), f'    if (take) goto L{pc+4+si*4:08X};']
  elif op==2:lines += ['    '+simple(pc+4),f'    goto L{((w&0x3ffffff)<<2)|0x80000000:08X};']
  elif op==3:
   target=((w&0x3ffffff)<<2)|0x80000000;name,ret,params=callee(target)
   args=[] if params=='void' else params.split(',')
   argexpr=[r(4+i) if i<4 else f'PE_LoadU32(sp+{16+4*(i-4)}u)' for i in range(len(args))]
   lines += [f'    ra = 0x{pc+8:08X}u;','    '+simple(pc+4),'    '+('v0 = (uint32_t)' if ret!='void' else '')+f'{name}('+', '.join(argexpr)+');']
  else:
   assert rs==31
   lines += ['    '+simple(pc+4),f'    PE_GuestFrame_Leave(frame, {frame}u);','    return (int32_t)v0;']
 # Fixed register allocation mirrors symbolic assembly variables; no runtime register lookup.
 used.update((2,4,5,28,29,31))
 header += ['    uint32_t '+', '.join(REG[i]+' = 0u' for i in sorted(used))+';', '    uint32_t hi = 0u; uint64_t product = 0u; int take = 0;',f'    a0=(uint32_t)mode; a1=data; gp=0x8009CD70u; sp=frame+{frame}u;',*lines,'}']
 return '\n'.join(header)+'\n'

if __name__=='__main__':
 blob=Path(sys.argv[1]).read_bytes()
 assert hashlib.sha1(blob).hexdigest()=='1d0e972cf9446dc700c82a8597292ced45958ef4'
 # The two Liberate record callbacks, and effect 0x6E-0x71's pool child 801F0068
 # (spawned by ovl_03C9 func_801F0690 mode 0 through func_800CE560).
 for off,size,frame,kind in [(0x44c,0x950,0x88,'liberate'),(0x13d0,0x998,0xb0,'liberate'),(0x2870,0x628,0xb0,'effect')]:
  out=ROOT/'pc_port/game/boot'/f'func_{BASE+off:08X}_{kind}_native.c'
  out.write_text(lift(blob,off,size,frame));print(out)
