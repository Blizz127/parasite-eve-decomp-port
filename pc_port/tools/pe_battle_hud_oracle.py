#!/usr/bin/env python3
"""Execute the complete retail HUD routines; produce native-test byte hashes.

This test-only MIPS runner reads instructions from the original executable.
It executes 33A40, 34104, 334AC, 438E0 and 77AC4, including AddPrim's
unaligned accesses, rather than reconstructing HUD formulas in Python.
"""
from pathlib import Path
import hashlib
import struct

ROOT = Path(__file__).resolve().parents[2]
CASES = [
    # bank, battle, PE visible, AT, PE, max PE, HP, max HP, status, corner, tick
    (0, 1, 0, 4500, 80, 80, 45, 45, 0, 3, 0),
    (1, 1, 1, 12000, 121, 80, 50, 45, 0x1FFF, 2, 23),
    (0, 0, 1, 0, -2, 80, 9, 123, 0x413, 0, 17),
    (1, 0, 0, 8999, 12, 80, 0, 999, 0x800, 1, 31),
    (0, 1, 1, 9000, -1, 80, 127, 9999, 0x1555, 3, 16),
    (1, 1, 1, 8999, 1, 200, 2, 1, 0xAAA, 1, 7),
    (0, 0, 0, 9001, 4, 80, -1, 45, 0, 2, 0),
    (1, 1, 0, 1, 79, 80, 420, 9999, 0x200, 0, 0),
]
RANGES = ((0x9E068, 0xBD0), (0xB00E8, 0xD8), (0xB6920, 0x38),
          (0x141000, 0x80), (0x142000, 0x40))


def signed(n):
    n &= 0xFFFFFFFF
    return n - 0x100000000 if n & 0x80000000 else n


def execute(ram, entry, args=(), *, stop_at=(), initial_regs=None, initial_cop_control=None, initial_cop_data=None, bios_seed=1, instruction_budget=100000, strict_gte_flags=False, final_gte=None, visited_pcs=None, scratchpad=None):
    r = [0] * 32
    r[28], r[29] = 0x8009CD70, 0x801FF000
    r[4:4+min(len(args),4)] = args[:4]
    for i,value in enumerate(args[4:]):
        struct.pack_into('<I',ram,0x1FF010+i*4,value&0xFFFFFFFF)
    if initial_regs:
        for index, value in initial_regs.items(): r[index] = value
    hi = lo = 0
    cop_data, cop_control = [0]*32, [0]*32
    flags_valid=True
    if initial_cop_data:
        for index,value in initial_cop_data.items(): cop_data[index]=value&0xFFFFFFFF
    if initial_cop_control:
        for index,value in initial_cop_control.items(): cop_control[index]=value&0xFFFFFFFF

    def geometry_command(w):
        nonlocal flags_valid
        # Test-only hardware arithmetic for complete original call graphs.
        # GPF/GPL and RTPS/RTPT follow the documented psx-spx GTE commands.
        command=w&0x1FFFFFF
        flags_valid=False
        if command in (0xA00428,0xA80428):
            # SQR: https://psx-spx.consoledev.net/geometrytransformationenginegte/
            # Signed 16-bit inputs, nonnegative MAC results; only IR saturation
            # can set FLAG. IR3 saturation alone does not set the summary bit.
            flags=0;shift=12 if command&0x80000 else 0
            for i in range(3):
                value=(cop_data[9+i]&32767)-(cop_data[9+i]&32768)
                square=(value*value)>>shift
                cop_data[25+i]=square
                cop_data[9+i]=min(32767,square)
                if square>32767:flags|=1<<(24-i)
            cop_control[31]=flags|(0x80000000 if flags&0x7F87E000 else 0)
            flags_valid=True
            return
        assert command in (0x118043F,0x480012,0x49E012,0x486012,0x180001,0x280030,
                           0x1400006,0x158002D,0x168002E,0x170000C,0x178000C,0x190003D,0x198003D,0x1A0003E,0x1A8003E), hex(command)
        def half(v): return (v&32767)-(v&32768)
        def wrap44(v): return ((v+(1<<43))&((1<<44)-1))-(1<<43)
        flags=0
        def accumulator(value,row):
            nonlocal flags
            if value>=(1<<43):flags|=1<<(30-row)
            if value<-(1<<43):flags|=1<<(27-row)
            return wrap44(value)
        def mac0_flags(value):
            nonlocal flags
            if value>0x7FFFFFFF:flags|=1<<16
            if value<-0x80000000:flags|=1<<15
        def finish_flags():
            nonlocal flags_valid
            cop_control[31]=flags|(0x80000000 if flags&0x7F87E000 else 0)
            flags_valid=True
        if command==0x118043F:
            # psx-spx NCCT: sf=1, lm=1, LLM then BK/LCM then RGBC.
            matrices=[struct.unpack('<10h',struct.pack('<5I',*cop_control[a:a+5]))[:9] for a in (8,16)]
            for vertex in range(3):
                vector=list(struct.unpack('<4h',struct.pack('<2I',*cop_data[vertex*2:vertex*2+2]))[:3])
                for stage,matrix in enumerate(matrices):
                    result=[]
                    for row in range(3):
                        total=signed(cop_control[13+row])*4096 if stage else 0
                        for col in range(3):total=accumulator(total+matrix[row*3+col]*vector[col],row)
                        mac=signed(total>>12);cop_data[25+row]=mac&0xFFFFFFFF
                        if not 0<=mac<=32767:flags|=1<<(24-row)
                        result.append(max(0,min(32767,mac)))
                    vector=result
                color=cop_data[6]&0xFF000000
                for row in range(3):
                    mac=(((cop_data[6]>>(row*8))&255)*vector[row]*16)>>12
                    cop_data[25+row]=mac;cop_data[9+row]=min(32767,mac)
                    if mac>32767:flags|=1<<(24-row)
                    if mac>>4>255:flags|=1<<(21-row)
                    color|=min(255,mac>>4)<<(row*8)
                cop_data[20:23]=cop_data[21:23]+[color]
            finish_flags()
            return
        if command in (0x170000C,0x178000C):
            diagonal=[half(cop_control[i]) for i in (0,2,4)]
            vector=[half(v) for v in cop_data[9:12]]
            flags=0;shift=12 if command&0x80000 else 0
            for i in range(3):
                a,b=(i+1)%3,(i+2)%3
                mac=(vector[b]*diagonal[a]-vector[a]*diagonal[b])>>shift
                cop_data[25+i]=mac&0xFFFFFFFF;cop_data[9+i]=max(-32768,min(32767,mac))&0xFFFFFFFF
                if not -32768<=mac<=32767:flags|=1<<(24-i)
            cop_control[31]=flags|(0x80000000 if flags&0x7F87E000 else 0)
            flags_valid=True
            return
        if command in (0x158002D,0x168002E):
            quad=command==0x168002E
            value=sum(v&65535 for v in cop_data[16 if quad else 17:20])*half(cop_control[30 if quad else 29])
            cop_data[24]=value&0xFFFFFFFF;cop_data[7]=max(0,min(65535,value>>12))
            mac0_flags(value)
            if not 0<=value>>12<=65535:flags|=1<<18
            finish_flags()
            return
        if command==0x1400006:
            points=[struct.unpack('<hh',struct.pack('<I',v)) for v in cop_data[12:15]]
            value=sum(points[i][0]*(points[(i+1)%3][1]-points[(i+2)%3][1]) for i in range(3))
            cop_data[24]=value&0xFFFFFFFF;mac0_flags(value);finish_flags()
            return
        if command&63 in (0x3D,0x3E):
            shift=12 if command&0x80000 else 0
            color=cop_data[6]&0xFF000000
            for i in range(3):
                base=signed(cop_data[25+i])*(1<<shift) if command&63==0x3E else 0
                mac=signed(accumulator(base+half(cop_data[8])*half(cop_data[9+i]),i)>>shift)
                cop_data[25+i]=mac&0xFFFFFFFF
                cop_data[9+i]=max(-32768,min(32767,mac))&0xFFFFFFFF
                if not -32768<=mac<=32767:flags|=1<<(24-i)
                if not 0<=mac>>4<=255:flags|=1<<(21-i)
                color|=max(0,min(255,mac>>4))<<(i*8)
            cop_data[20:23]=cop_data[21:23]+[color]
            finish_flags()
            return
        halves=struct.unpack('<10h',struct.pack('<5I',*cop_control[:5]))
        for vertex_index in range(3 if command==0x280030 else 1):
            if command==0x49E012: vector=[half(v) for v in cop_data[9:12]]
            else: vector=struct.unpack('<4h',struct.pack('<2I',*cop_data[vertex_index*2:vertex_index*2+2]))[:3]
            mac=[]
            for row in range(3):
                total=0 if command in (0x49E012,0x486012) else signed(cop_control[5+row])*4096
                for j in range(3):total=accumulator(total+halves[row*3+j]*vector[j],row)
                mac.append(signed(total//4096))
                cop_data[25+row]=mac[row]&0xFFFFFFFF
                cop_data[9+row]=max(-32768,min(32767,mac[row]))&0xFFFFFFFF
                if not -32768<=mac[row]<=32767:flags|=1<<(24-row)
            if command in (0x180001,0x280030):
                depth=max(0,min(65535,mac[2])); h=cop_control[26]&65535
                if not 0<=mac[2]<=65535:flags|=1<<18
                if h>=depth*2: ratio=0x1FFFF;flags|=1<<17
                else:
                    # PSX UNR reciprocal refinement, integer arithmetic only.
                    shift=16-depth.bit_length(); divisor=depth<<shift
                    index=(divisor-0x7FC0)//128
                    table=max(0,((0x40000//(index+256)+1)//2)-257)
                    reciprocal=table+257
                    correction=(0x2000080-divisor*reciprocal)//256
                    reciprocal=(128+correction*reciprocal)//256
                    ratio=min(0x1FFFF,((h<<shift)*reciprocal+32768)//65536)
                screen=[]
                for i in range(2):
                    raw=signed(cop_data[9+i])*ratio+signed(cop_control[24+i]);mac0_flags(raw)
                    xy=raw>>16
                    if not -1024<=xy<=1023:flags|=1<<(14-i)
                    screen.append(max(-1024,min(1023,xy)))
                cop_data[12:15]=cop_data[13:15]+[(screen[0]&65535)|((screen[1]&65535)<<16)]
                cop_data[16:20]=cop_data[17:20]+[depth]
                cue=ratio*half(cop_control[27])+signed(cop_control[28])
                cop_data[24]=cue&0xFFFFFFFF
                cop_data[8]=max(0,min(4096,cue//4096))
                if command!=0x280030 or vertex_index==2:
                    mac0_flags(cue)
                    if not 0<=cue>>12<=4096:flags|=1<<12
        finish_flags()

    def step(pc):
        nonlocal hi, lo, flags_valid
        w = struct.unpack_from('<I', ram, pc & 0x1FFFFF)[0]
        op, rs, rt, rd, sh = w >> 26, w >> 21 & 31, w >> 16 & 31, w >> 11 & 31, w >> 6 & 31
        imm = w & 0xFFFF
        si = imm if imm < 32768 else imm - 65536
        address = (r[rs] + si) & 0xFFFFFFFF
        a = address & 0x1FFFFF
        memory = ram
        if scratchpad is not None and 0x1F800000 <= address < 0x1F800400:
            assert len(scratchpad) == 0x400
            memory, a = scratchpad, address - 0x1F800000
        jump = None
        if op == 0:
            fn = w & 63
            if fn == 0: r[rd] = r[rt] << sh
            elif fn == 2: r[rd] = r[rt] >> sh
            elif fn == 3: r[rd] = signed(r[rt]) >> sh
            elif fn == 4: r[rd] = r[rt] << (r[rs] & 31)
            elif fn == 6: r[rd] = r[rt] >> (r[rs] & 31)
            elif fn == 7: r[rd] = signed(r[rt]) >> (r[rs] & 31)
            elif fn == 8: jump = r[rs]
            elif fn == 9:
                jump = r[rs]
                r[rd] = pc + 8
            elif fn == 16: r[rd] = hi
            elif fn == 18: r[rd] = lo
            elif fn == 24:
                prod = signed(r[rs]) * signed(r[rt])
                hi, lo = prod >> 32 & 0xFFFFFFFF, prod & 0xFFFFFFFF
            elif fn == 25:
                prod = r[rs] * r[rt]
                hi, lo = prod >> 32 & 0xFFFFFFFF, prod & 0xFFFFFFFF
            elif fn == 26:
                n, d = signed(r[rs]), signed(r[rt])
                assert d, ('division by zero in original fixture',hex(pc),n)
                q = (abs(n) // abs(d)) * (-1 if (n < 0) != (d < 0) else 1)
                lo, hi = q & 0xFFFFFFFF, (n-q*d) & 0xFFFFFFFF
            elif fn in (32,34):
                result=signed(r[rs])+signed(r[rt])*(1 if fn==32 else -1)
                assert -(1<<31)<=result<(1<<31),'retail signed arithmetic overflow'
                r[rd]=result
            elif fn == 33: r[rd] = r[rs] + r[rt]
            elif fn == 35: r[rd] = r[rs] - r[rt]
            elif fn == 36: r[rd] = r[rs] & r[rt]
            elif fn == 37: r[rd] = r[rs] | r[rt]
            elif fn == 38: r[rd] = r[rs] ^ r[rt]
            elif fn == 39: r[rd] = ~(r[rs] | r[rt])
            elif fn == 42: r[rd] = int(signed(r[rs]) < signed(r[rt]))
            elif fn == 43: r[rd] = int(r[rs] < r[rt])
            else: raise AssertionError((hex(pc), hex(w)))
        elif op in (2, 3):
            jump = (pc & 0xF0000000) | (w & 0x3FFFFFF) << 2
            if op == 3: r[31] = pc + 8
        elif op in (1, 4, 5, 6, 7):
            if op == 1:
                assert rt in (0, 1)
                take = signed(r[rs]) >= 0 if rt == 1 else signed(r[rs]) < 0
            elif op == 4: take = r[rs] == r[rt]
            elif op == 5: take = r[rs] != r[rt]
            elif op == 6: take = signed(r[rs]) <= 0
            else: take = signed(r[rs]) > 0
            jump = pc + 4 + si * 4 if take else pc + 8
        elif op == 8:
            result=signed(r[rs])+si
            assert -(1<<31)<=result<(1<<31),'retail ADDI overflow'
            r[rt]=result
        elif op == 9: r[rt] = r[rs] + si
        elif op == 10: r[rt] = int(signed(r[rs]) < si)
        elif op == 11: r[rt] = int(r[rs] < (si & 0xFFFFFFFF))
        elif op == 12: r[rt] = r[rs] & imm
        elif op == 13: r[rt] = r[rs] | imm
        elif op == 14: r[rt] = r[rs] ^ imm
        elif op == 15: r[rt] = imm << 16
        elif op == 18:
            if rs == 0: r[rt] = cop_data[rd]
            elif rs == 2:
                assert not(strict_gte_flags and rd==31 and not flags_valid), ('unimplemented GTE FLAG result',hex(pc))
                # Matrix final elements are signed 16-bit on CFC2 readback.
                # https://psx-spx.consoledev.net/geometrytransformationenginegte/
                r[rt] = (((cop_control[rd]&32767)-(cop_control[rd]&32768)) & 0xFFFFFFFF) if rd in (4,12,20) else cop_control[rd]
            elif rs == 4:
                cop_data[rd] = r[rt]
                if rd==30:
                    bits=r[rt]^0xFFFFFFFF if r[rt]&0x80000000 else r[rt]
                    cop_data[31]=32-bits.bit_length()
            elif rs == 6:
                cop_control[rd] = r[rt]
                if rd==31:flags_valid=True
            else:
                geometry_command(w)
        elif op == 50: cop_data[rt] = struct.unpack_from('<I',memory,a)[0]
        elif op == 58: struct.pack_into('<I',memory,a,cop_data[rt]&0xFFFFFFFF)
        elif op in (32, 33, 35, 36, 37):
            r[rt] = struct.unpack_from({32:'<b', 33:'<h', 35:'<I', 36:'<B', 37:'<H'}[op], memory, a)[0]
        elif op in (40, 41, 43):
            fmt, mask = {40:('<B',255), 41:('<H',65535), 43:('<I',0xFFFFFFFF)}[op]
            struct.pack_into(fmt, memory, a, r[rt] & mask)
        elif op in (34, 38, 42, 46):
            val = struct.unpack_from('<I', memory, a & ~3)[0]
            shift = (3-(a&3))*8 if op in (34, 42) else (a&3)*8
            if op == 34: r[rt] = (r[rt] & ((1 << shift)-1)) | (val << shift)
            elif op == 38: r[rt] = (r[rt] & (0xFFFFFFFF << (32-shift))) | (val >> shift)
            else:
                if op == 42: val = (val & ~(0xFFFFFFFF >> shift)) | (r[rt] >> shift)
                else: val = (val & ((1 << shift)-1)) | (r[rt] << shift)
                struct.pack_into('<I', memory, a & ~3, val & 0xFFFFFFFF)
        else: raise AssertionError((hex(pc), hex(w)))
        r[:] = [v & 0xFFFFFFFF for v in r]
        r[0] = 0
        return jump

    pc = entry
    for _ in range(instruction_budget):
        if pc == 0 or pc in stop_at:
            if final_gte is not None:
                final_gte.update(data=cop_data[:], control=cop_control[:])
            return r
        if pc == 0xA0:
            if r[9] == 0x2B:
                start=r[4]&0x1FFFFF
                ram[start:start+r[6]]=bytes((r[5]&255,))*r[6]
                r[2]=r[4]
            elif r[9] == 0x28:
                # Existing b21_bzero_oracle BIOS A(28h) memory contract.
                start=r[4]&0x1FFFFF;size=r[5]
                assert start+size<=len(ram)
                ram[start:start+size]=bytes(size)
                r[2]=r[4]
            elif r[9] == 0x30: bios_seed=r[4]
            elif r[9] == 0x2F:
                bios_seed=(bios_seed*0x41C64E6D+0x3039)&0xFFFFFFFF
                r[2]=(bios_seed>>16)&0x7FFF
            elif r[9] == 0x3F:
                # BIOS printf diagnostic used by missing effect resources.
                # These messages do not modify guest RAM. Model the signed
                # integer arguments used by the original diagnostic only.
                start=r[4]&0x1FFFFF
                end=ram.index(0,start)
                fmt=bytes(ram[start:end])
                count=fmt.count(b'%d')
                assert count<=3 and b'%' not in fmt.replace(b'%d',b''), 'unsupported BIOS printf format'
                values=tuple(v if v<0x80000000 else v-0x100000000 for v in r[5:5+count])
                r[2]=len(fmt % values)
            else: raise AssertionError(f'unsupported BIOS service {r[9]:X} from {r[31]:08X} in test oracle')
            pc=r[31]
            continue
        if visited_pcs is not None: visited_pcs.add(pc)
        jump = step(pc)
        if jump is None: pc += 4
        else:
            assert step(pc+4) is None, 'branch in delay slot'
            pc = jump
    raise AssertionError('instruction budget')


def fixture(exe, case):
    ram = bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800] = exe[0x800:]
    for start, length in RANGES:
        ram[start:start+length] = bytes((i*37+19) & 255 for i in range(length))
    def sw(a, v): struct.pack_into('<I', ram, a, v & 0xFFFFFFFF)
    def sh(a, v): struct.pack_into('<H', ram, a, v & 0xFFFF)
    bank, battle, pe, at, energy, maximum, hp, maxhp, flags, corner, tick = case
    sw(0x9CDDC, bank); sw(0x9D1A0, battle*2); sw(0x9CEF0, pe*2)
    sw(0xB0E38+bank*4, 0x80142000)
    sw(0x9D254, 0x80140000); sw(0x140000, 0x80141000); sw(0x9D278, 0x80141000)
    sw(0x141008, energy*65536); sw(0x141028, maximum*65536)
    sh(0x141010, at); sh(0x14100C, hp); sh(0x14101C, maxhp)
    sw(0x14104C, flags); sw(0x9D1E8, tick); ram[0x9CE80] = corner
    sh(0x9CE84, (15, 221, 221, 15)[corner]); sh(0x9CE86, (177, 177, 15, 15)[corner])
    return ram


def fingerprint(ram):
    h = 14695981039346656037
    for start, length in RANGES:
        for b in ram[start:start+length]: h = ((h ^ b)*1099511628211) & 0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe = (ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for i, case in enumerate(CASES):
        ram = fixture(exe, case)
        execute(ram, 0x80033A40)
        print(i, f'0x{fingerprint(ram):016X}ull')
    print('PASS: retail HUD, HP digits, status icons and AddPrim executed for both banks and all corners')


if __name__ == '__main__':
    main()
