#!/usr/bin/env python3
"""Original status poll + stream assembly. VSync is a provider; MMIO relocated.
No sector data provider: original word copies assemble the seeded RAM sectors.
"""
import hashlib,itertools,struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
RANGES=((0x9B294,3),(0x9B374,4),(0xA3460,0x44),(0xA5D54,4),(0xA8018,12),
        (0xB0CD0,4),(0xB6914,8),(0xB8620,4),(0xB89F4,4),(0xBCD7C,4),
        (0xBE998,4),(0xBE9E4,8),(0xC0DB8,20),(0xC20C4,4),(0x150000,0x5000))
SEED=(0xB89F4,0xA801C,0xA8020,0xC0DB8,0xBCD7C,0x9B296,0x9B295,
      0xA3470,0xA3468,0xBE998,0xBE9E4,0xC20C4,0xC0DC0,0xB6918,
      0xC0DBC,0xB8620,0xB6914,0xA8018,0xA5D54,0xB0CCC,0x945E6)
BASE=[0,1,1,0x80160000,0,1,0,0,0,1,1,8,0,0,0,3,4,0,0,0,0]

def sw(r,a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
def sh(r,a,v):struct.pack_into('<H',r,a,v&65535)

def main():
    ex,o=load();rows=[];poll=[];spec=[]
    def add(name,change=None,header=None,occupied=0,first=0,busy=0):
        v=BASE.copy()
        for a,value in (change or {}).items():v[SEED.index(a)]=value
        h=[0x160,3<<10,0,2,7]
        for i,value in (header or {}).items():h[i]=value
        spec.append((name,v,h,occupied,first,busy))
    add('continue');add('one-part',header={3:1})
    add('second-final',{0xA8018:1,0xA5D54:7},header={2:1})
    add('memory-offset',{0xBCD7C:1},header={3:1})
    add('format-zero',{0xA801C:0},header={3:1})
    add('busy-guard',{0xB89F4:1});add('guard-only-equals-one',{0xB89F4:2},header={3:1})
    add('mdec-busy',busy=1);add('mdec-busy-ignored',{0xA801C:0},busy=1)
    add('status-five',{0x9B296:5});add('status-error',{0xA3470:4})
    add('fallback-status',{0x9B296:0,0x9B295:1})
    add('no-result',{0x9B296:0})
    add('record-occupied',occupied=4)
    add('fifo-boundary',{0xA8020:0})
    add('dma-boundary',{0xC0DB8:0})
    add('start-reject',{0xC0DC0:1,0xB6918:8})
    add('start-match',{0xC0DC0:1,0xB6918:7})
    add('bad-magic',header={0:0x161});add('bad-channel',header={1:4<<10})
    add('part-mismatch',{0xBE998:3,0xBE9E4:1},header={2:1})
    add('signed-part-mismatch',{0xA8018:65535},header={2:65535})
    add('frame-mismatch',{0xA5D54:8,0xBE998:3})
    add('frame-end',{0xC0DBC:7,0xBE998:3})
    add('frame-over-end',{0xC0DBC:6})
    add('frame-end-callback',{0xC0DBC:7,0xB0CCC:0x80170000})
    add('wrap-without-limit',{0xBE998:6})
    add('wrap-callback',{0xBE998:6,0xB0CCC:0x80170000})
    add('wrap-first-busy',{0xBE998:6,0xC0DBC:99},first=2)
    add('wrap-first-free',{0xBE998:6,0xC0DBC:99})
    add('exact-fit',{0xBE998:5})
    add('negative-limit',{0xC0DBC:0xFFFFFFFF})
    add('interrupt-idle',{0x945E6:1})
    # Distinguishable payloads and two format options multiply the structural cases.
    for name,init,h,occupied,first,busy in spec:
      for seed in (37,165):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        for a,n in RANGES:r[a:a+n]=bytes(n)
        r[0x150000:0x155000]=bytes([0xCC])*0x5000
        for a,v in zip(SEED,init):sw(r,a,v)
        sw(r,0xC0DC8,0x80150000);sw(r,0xB0CC8,0)
        for i in range(8):sh(r,0x150000+i*32,4)
        sh(r,0x150000,first);sh(r,0x150000+init[9]*32,occupied)
        for i in range(6144):r[0x160000+i]=(seed+i*17)&255
        addr=0x160000+init[4]*2048
        for i,v in enumerate(h):sh(r,addr+i*2,v)
        for a,v in ((0x9B27C,0x80130000),(0x9B280,0x80130001),
          (0x9B284,0x80130002),(0x9B288,0x80130003),(0x9B32C,0x80130000),
          (0x9B338,0x80130003),(0x9B33C,0x80130010),(0x9B340,0x80130014),
          (0x9B35C,0x80130018),(0x9B34C,0x8013001C)):sw(r,a,v)
        r[0x130000:0x130020]=bytes(32);sw(r,0x13001C,0x01000000 if busy else 0)
        sw(r,0x1FEFE8,0x55667788);r[0x1FEFF0:0x1FEFF8]=bytes(8)
        # A physical FIFO and DMA submission cannot be executed by the RAM interpreter.
        stops=(0x80073A44,0x8007C70C,0x8007CEAC,0x80170000)
        regs=execute(r,0x8007C564,stop_at=stops,instruction_budget=500000)
        boundary=0
        while regs[31]:
            ret=regs[31]
            if ret in (0x8007B2CC,0x8007B318):
                regs[2]=0
                regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=stops,instruction_budget=500000)
                continue
            if name=='fifo-boundary':boundary=1
            elif name=='dma-boundary':boundary=2
            elif name=='interrupt-boundary':boundary=3
            elif 'callback' in name:boundary=4
            else:raise AssertionError((name,hex(ret)))
            break
        rows.append((name,init,h,occupied,first,busy,seed,boundary,digest(r,RANGES),*struct.unpack_from('<2I',r,0x130010),r[0x130003]))
    # Status priority, result copy, aliases and empty nonblocking result.
    for hi,lo,dst,mode in itertools.product((0,1,5,255),(0,2,5),(0x80140000,0),(1,256)):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        for a,n in ((0xA3460,0x44),(0x9B294,3),(0x140000,16)):r[a:a+n]=bytes(n)
        r[0x140000:0x140010]=bytes([0xCC])*16
        for i in range(16):r[0xA3468+i]=(37+i*17)&255
        r[0x9B296]=hi;r[0x9B295]=lo;sh(r,0x945E6,0)
        regs=execute(r,0x8007A488,(mode,dst),stop_at=(0x80073A44,))
        while regs[31]:
            ret=regs[31];assert ret in (0x8007B2CC,0x8007B318)
            regs[2]=0;regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=(0x80073A44,))
        poll.append((hi,lo,dst,mode,regs[2],digest(r,((0xA3460,0x44),(0x9B294,3),(0x140000,16)))))
    lines=['/* Original EXE execution. VSync provider, redirected MMIO, real RAM copies. */',
      'static const uint32_t CDSTREAM_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const uint32_t CDSTREAM_seed_addresses[]={'+','.join(f'0x{a:X}u' for a in SEED)+'};',
      'static const struct { uint32_t init['+str(len(SEED))+'],header[5],occupied,first,busy,seed,boundary; uint64_t hash; uint32_t bus,mailbox,request; } CDSTREAM_cases[]={']
    for name,v,h,*tail in rows:
        nums=tail[:5];hashv=tail[5];io=tail[6:]
        lines.append('/* '+name+' */ {{'+','.join(f'{x}u' for x in v)+'},{'+','.join(f'{x}u' for x in h)+'},'+','.join(f'{x}u' for x in nums)+f',UINT64_C(0x{hashv:016X}),'+','.join(f'{x}u' for x in io)+'},')
    lines+=['};','static const struct { uint32_t hi,lo,dst,mode,result; uint64_t hash; } CDSTREAM_poll[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in poll];lines+=['};']
    output='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_stream_cases.h'
    if '--check' in sys.argv:assert p.read_text()==output
    else:p.write_text(output)
    print(f'PASS {len(rows)} stream graphs/prefixes and {len(poll)} complete status-poll graphs')
if __name__=='__main__':main()
