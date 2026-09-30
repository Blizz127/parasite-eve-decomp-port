#!/usr/bin/env python3
"""Complete opening STR frames through original table builder and VLC instructions.
COP0 status read/write are explicit providers; no original instructions patched.
Outputs hashes only: no copyrighted frame payloads embedded in fixtures.
"""
import hashlib, struct, sys
from pe_movie_init_oracle import load, ROOT, execute
from pe_m0000i_leaves_oracle import find_disc, read_form1
from pe_mv1d_c89c_oracle import MOV_SHA256
from pe_cd_ack_oracle import Stops


def fnv(data):
    h=14695981039346656037
    for b in data:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def frames(path, count=3):
    result=[];chunks=[];first=0;metadata=None
    with open(path,'rb') as f:
        for lba in range(189742,190742):
            f.seek(lba*2352);raw=f.read(2352)
            assert len(raw)==2352
            if not raw[18]&8:continue  # Video only; exclude interleaved XA audio.
            magic,index,total,frame,size,width,height=struct.unpack_from('<IHHIIHH',raw,24)
            assert magic==0x80010160 and raw[16:20]==raw[20:24]
            if index==0:
                assert not chunks
                first=lba;metadata=(total,frame,size,width,height)
            assert index==len(chunks) and metadata==(total,frame,size,width,height)
            chunks.append(raw[56:2072])
            if len(chunks)==total:
                data=b''.join(chunks);assert 12<=size<=len(data)
                result.append((first,lba,total,frame,size,width,height,data))
                chunks=[]
                if len(result)==count:return result
    raise AssertionError('incomplete frame scan')


def main():
    ex,_=load();path=find_disc(ROOT);overlay=read_form1(path,1940,38)
    assert hashlib.sha256(overlay).hexdigest()==MOV_SHA256
    ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-2048]=ex[2048:]
    ram[0x10BCF8:0x10BCF8+len(overlay)]=overlay
    execute(ram,0x8010BD4C,(0x80130000,0),instruction_budget=2000000)
    tablehash=fnv(ram[0x130000:0x141000]);rows=[]
    for first,last,total,frame,size,width,height,data in frames(path):
        r=bytearray(ram);r[0x150000:0x150000+len(data)]=data
        command=struct.unpack_from('<I',data)[0];length=4+(command&65535)*4
        r[0x160000:0x160000+length+16]=bytes([0xCD])*(length+16)
        stops=Stops((0x8010CBAC,0x8010CBBC));stops.last=0
        regs=execute(r,0x8010C89C,(0x80150000,0x80160000,0x80130000,0),stop_at=stops,instruction_budget=3000000)
        assert stops.last==0x8010CBAC
        consumed=regs[4]-0x80150000
        assert consumed<=size and regs[5]==0x80160000+length
        regs[9]=0;stops.last=0
        regs=execute(r,0x8010CBB0,initial_regs=dict(enumerate(regs)),stop_at=stops)
        assert stops.last==0x8010CBBC and regs[9]==0x20000
        regs=execute(r,0x8010CBC0,initial_regs=dict(enumerate(regs)))
        assert regs[2]==0 and r[0x160000+length:0x160000+length+16]==bytes([0xCD])*16
        rows.append((first,last,total,frame,size,width,height,command,length,consumed,fnv(data),fnv(r[0x160000:0x160000+length])))
        print(f'frame {frame}: {width}x{height}, {size} compressed bytes, {length} RLE bytes, consumed {consumed}',flush=True)
    out='/* Original complete STR-frame VLC output; COP0 status providers, no pixel oracle. */\n'
    out+=f'static const uint64_t MOVCOMPLETE_table_hash=UINT64_C(0x{tablehash:016X});\n'
    out+='static const struct { uint32_t first,last,chunks,frame,size,width,height,command,length,consumed; uint64_t input_hash,output_hash; } MOVCOMPLETE_cases[]={\n'
    out+=''.join('{'+','.join(f'{v}u' for v in row[:-2])+f',UINT64_C(0x{row[-2]:016X}),UINT64_C(0x{row[-1]:016X})'+'},\n' for row in rows)+'};\n'
    p=ROOT/'pc_port/tests/retail_movie_complete_frame_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original complete frame graphs and full VLC table builder')
if __name__=='__main__':main()
