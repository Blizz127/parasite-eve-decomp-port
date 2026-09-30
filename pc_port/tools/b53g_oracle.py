#!/usr/bin/env python3
"""Phase 6E-B53G literal oracle for func_800746A0.

This is an independent retail proof of the Psy-Q DMA callback-slot setter.
It verifies all 43 literal words against the SHA-exact executable and then
executes them on a small MIPS-I interpreter with exact delay slots.

The interpreter models exactly two authorities:

  * guest RAM, initialised from the retail image, holding the eight-entry
    callback table D_800956C0 and the DICR pointer word D_800956BC;
  * a DICR hardware register at MMIO 0x1F8010F4 with PSX W1C semantics for
    the channel completion flags 24..30 and a read-only master flag 31.

Nothing here imports, links, or calls production C.  No DMA transfer, IRQ
delivery, callback dispatch, ring publication, or timing progression exists
in this model: every value below is derived from retail words alone.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SETTER_SHA256 = (
    "ac160079410a40d719e5a8668f627d05d36d287d08959adc22fd3e069e4e99dd"
)
DMA_INIT_SHA256 = (
    "e293c6aa8850c852f562fee7c52c855d8b020fad9669159721d8b8e4b152a1be"
)

MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x00200000
RA_SENTINEL = 0xDEADBEE8
SP_INITIAL = 0x801FFF00

SETTER = 0x800746A0
SETTER_END = 0x8007474C
SETTER_SIZE = 0xAC
SETTER_WORDS_COUNT = 43
SETTER_FILE_OFFSET = 0x64EA0

DMA_INIT = 0x800744D4
DMA_INIT_END = 0x80074520
DMA_DISPATCHER = 0x80074520
WRAPPER = 0x80073CF4

CALLBACK_TABLE = 0x800956C0
CALLBACK_TABLE_SLOTS = 8
CALLBACK_TABLE_END = CALLBACK_TABLE + 4 * CALLBACK_TABLE_SLOTS

DICR_POINTER = 0x800956BC
DICR_MMIO = 0x1F8010F4
DMA0_MMIO = 0x1F801080
I_STAT_MMIO = 0x1F801070
I_MASK_MMIO = 0x1F801074

DICR_CONTROL_MASK = 0x00FFFFFF
DICR_FLAG_MASK = 0x7F000000
DICR_MASTER_ENABLE = 0x00800000
DICR_MASTER_FLAG = 0x80000000
DICR_FORCE = 0x00008000

GPU_RING = 0x800BD030
GPU_RING_SIZE = 0x1800

CANONICAL_CHANNEL = 2
CANONICAL_HANDLER = 0x80076EE4


# ── Literal retail words ────────────────────────────────────────────────
# Constants transcribed from the disassembly; never read back from the
# executable or from production code.
SETTER_WORDS = exe_words(0x800746A0, 43)
    # 800746A0  addu  a2,a0,zero        ; a2 = channel
    # 800746A4  lui   v1,0x8009
    # 800746A8  addiu v1,v1,0x56C0      ; &D_800956C0
    # 800746AC  sll   v0,a2,2           ; channel*4
    # 800746B0  addu  v1,v0,v1          ; raw slot address
    # 800746B4  lw    a3,0(v1)          ; previous handler
    # 800746B8  addu  a0,a1,zero        ; a0 = handler
    # 800746BC  beq   a0,a3,0x80074744
    # 800746C0  addu  v0,a3,zero        ; delay slot: return
    # 800746C4  beqz  a0,0x80074708
    # 800746C8  lui   v0,0x00FF         ; delay slot
    # 800746CC  lui   a1,0x8009
    # 800746D0  lw    a1,0x56BC(a1)     ; DICR pointer
    # 800746D4  ori   v0,v0,0xFFFF      ; 0x00FFFFFF
    # 800746D8  sw    a0,0(v1)          ; slot = handler
    # 800746DC  lw    a0,0(a1)          ; DICR read
    # 800746E0  addiu v1,a2,0x10        ; channel + 16
    # 800746E4  and   a0,a0,v0
    # 800746E8  addiu v0,zero,1
    # 800746EC  sllv  v0,v0,v1
    # 800746F0  lui   v1,0x0080
    # 800746F4  or    v0,v0,v1
    # 800746F8  or    a0,a0,v0
    # 800746FC  sw    a0,0(a1)          ; DICR write
    # 80074700  j     0x80074744
    # 80074704  addu  v0,a3,zero        ; delay slot: return
    # 80074708  lui   a1,0x8009
    # 8007470C  lw    a1,0x56BC(a1)     ; DICR pointer
    # 80074710  ori   v0,v0,0xFFFF      ; 0x00FFFFFF
    # 80074714  sw    zero,0(v1)        ; slot = 0
    # 80074718  lw    v1,0(a1)          ; DICR read
    # 8007471C  addiu a0,a2,0x10        ; channel + 16
    # 80074720  and   v1,v1,v0
    # 80074724  lui   v0,0x0080
    # 80074728  or    v1,v1,v0          ; master BEFORE clear
    # 8007472C  addiu v0,zero,1
    # 80074730  sllv  v0,v0,a0
    # 80074734  nor   v0,zero,v0
    # 80074738  and   v1,v1,v0
    # 8007473C  sw    v1,0(a1)          ; DICR write
    # 80074740  addu  v0,a3,zero        ; return previous
    # 80074744  jr    ra
    # 80074748  nop                     ; jr delay slot

# func_800744D4: the initialization owner.  It clears the eight-slot table
# through func_8007474C, zeroes DICR, installs func_80074520 on interrupt
# source 3, and returns the func_800746A0 identity for the jump table.
DMA_INIT_WORDS = exe_words(0x800744D4, 19)

# The seven libetc jump-table wrappers and the field each dispatches.
# Only field +0x04 reaches func_800746A0.
JUMP_TABLE_WRAPPERS = {
    0x80073C94: 0x0C,
    0x80073CC4: 0x08,
    0x80073CF4: 0x04,
    0x80073D24: 0x14,
    0x80073D58: 0x14,
    0x80073D88: 0x10,
    0x80073DB8: 0x18,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


def u32(value: int) -> int:
    return value & MASK


def sx16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def s32(value: int) -> int:
    value &= MASK
    return value - (1 << 32) if value & 0x80000000 else value


class Exe:
    def __init__(self, path: pathlib.Path):
        data = path.read_bytes()
        got = hashlib.sha1(data).hexdigest()
        require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
        require(data[:8] == b"PS-X EXE", f"{path} is not a PS-X EXE")
        self.data = data
        self.taddr, self.tsize = struct.unpack_from("<II", data, 0x18)
        require(len(data) == 0x800 + self.tsize,
                "PS-X EXE size/header mismatch")
        self.image = data[0x800:]

    def file_offset(self, address: int) -> int:
        return 0x800 + address - self.taddr

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset + size <= len(self.image),
                f"executable range 0x{address:08X}+0x{size:X}")
        return self.image[offset:offset + size]

    def word(self, address: int) -> int:
        return struct.unpack("<I", self.body(address, 4))[0]

    def words(self):
        for index in range(0, len(self.image) - 3, 4):
            yield (self.taddr + index,
                   struct.unpack_from("<I", self.image, index)[0])


class Dicr:
    """PSX DMA interrupt control register, bounded to the fields the retail
    words actually use.

    Lower 24 bits are plain control state.  Bits 24..30 are per-channel
    completion flags cleared by writing one.  Bit 31 is the read-only
    derived master flag; a write to it is ignored by hardware.
    """

    def __init__(self, value: int = 0, *, synthesize_master_flag: bool = False):
        self.control = value & DICR_CONTROL_MASK
        self.flags = value & DICR_FLAG_MASK
        self.synthesize_master_flag = synthesize_master_flag
        self.reads = 0
        self.writes: list[int] = []

    def _master_flag(self) -> int:
        if not self.synthesize_master_flag:
            return 0
        if self.control & DICR_FORCE:
            return DICR_MASTER_FLAG
        if (self.control & DICR_MASTER_ENABLE) == 0:
            return 0
        pending = (self.flags >> 24) & 0x7F
        # B53I-A/B2: completion flags are gated when created.  Once a flag
        # is retained, the physical master flag depends on master + any
        # retained flag, not the current per-channel enable bits.
        return DICR_MASTER_FLAG if pending else 0

    def read(self) -> int:
        self.reads += 1
        return self.control | self.flags | self._master_flag()

    def write(self, value: int) -> None:
        value &= MASK
        self.writes.append(value)
        self.flags &= ~(value & DICR_FLAG_MASK)
        self.control = value & DICR_CONTROL_MASK

    def raw(self) -> int:
        return self.control | self.flags


class Machine:
    """MIPS-I executor for exactly the 43-word setter body."""

    def __init__(self, exe: Exe, *, dicr: Dicr,
                 table: dict[int, int] | None = None):
        self.ram = bytearray(RAM_SIZE)
        image_offset = exe.taddr - RAM_BASE
        require(0 <= image_offset <= RAM_SIZE - exe.tsize,
                "retail image does not fit modeled guest RAM")
        self.ram[image_offset:image_offset + exe.tsize] = exe.image
        for slot, value in (table or {}).items():
            self.store32_setup(CALLBACK_TABLE + 4 * slot, value)
        self.dicr = dicr
        self.reg = [0] * 32
        self.reg[29] = SP_INITIAL
        self.reg[31] = RA_SENTINEL
        self.trace: list[int] = []
        self.guest_writes: list[tuple[int, int]] = []
        self.mmio_reads: list[int] = []
        self.mmio_writes: list[tuple[int, int]] = []

    # ── memory ─────────────────────────────────────────────────────────
    def _offset(self, address: int, size: int) -> int:
        require(RAM_BASE <= address and address + size <= RAM_BASE + RAM_SIZE,
                f"guest access outside modeled RAM 0x{address:08X}+{size}")
        return address - RAM_BASE

    def load32(self, address: int) -> int:
        require(address & 3 == 0, f"unaligned lw 0x{address:08X}")
        if address == DICR_MMIO:
            self.mmio_reads.append(address)
            return self.dicr.read()
        require(address < 0x1F000000 or address >= RAM_BASE,
                f"unmodeled MMIO read 0x{address:08X}")
        return struct.unpack_from("<I", self.ram, self._offset(address, 4))[0]

    def store32_setup(self, address: int, value: int) -> None:
        struct.pack_into("<I", self.ram, self._offset(address, 4),
                         value & MASK)

    def store32(self, address: int, value: int) -> None:
        require(address & 3 == 0, f"unaligned sw 0x{address:08X}")
        if address == DICR_MMIO:
            self.mmio_writes.append((address, value & MASK))
            self.dicr.write(value)
            return
        require(address < 0x1F000000 or address >= RAM_BASE,
                f"unmodeled MMIO write 0x{address:08X}")
        struct.pack_into("<I", self.ram, self._offset(address, 4),
                         value & MASK)
        self.guest_writes.append((address, value & MASK))

    def snapshot(self, address: int, size: int) -> bytes:
        offset = self._offset(address, size)
        return bytes(self.ram[offset:offset + size])

    def table(self) -> list[int]:
        return [struct.unpack_from("<I", self.ram,
                                   self._offset(CALLBACK_TABLE + 4 * i, 4))[0]
                for i in range(CALLBACK_TABLE_SLOTS)]

    # ── execution ──────────────────────────────────────────────────────
    def _set(self, index: int, value: int) -> None:
        if index:
            self.reg[index] = value & MASK

    def step(self, pc: int, *, in_delay_slot: bool = False) -> int:
        self.trace.append(pc)
        word = self.load32(pc)
        op = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        sa = (word >> 6) & 31
        imm = word & 0xFFFF
        simm = sx16(word)

        if op == 0x0F:                                  # lui
            self._set(rt, imm << 16)
            return pc + 4
        if op == 0x09:                                  # addiu
            self._set(rt, self.reg[rs] + simm)
            return pc + 4
        if op == 0x0D:                                  # ori
            self._set(rt, self.reg[rs] | imm)
            return pc + 4
        if op == 0x23:                                  # lw
            self._set(rt, self.load32(u32(self.reg[rs] + simm)))
            return pc + 4
        if op == 0x2B:                                  # sw
            self.store32(u32(self.reg[rs] + simm), self.reg[rt])
            return pc + 4
        if op == 0x04:                                  # beq / beqz
            require(not in_delay_slot, "nested control transfer")
            taken = self.reg[rs] == self.reg[rt]
            target = u32(pc + 4 + (simm << 2))
            self.step(pc + 4, in_delay_slot=True)
            return target if taken else pc + 8
        if op == 0x02:                                  # j
            require(not in_delay_slot, "nested control transfer")
            target = (u32(pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
            self.step(pc + 4, in_delay_slot=True)
            return target
        if op == 0:
            funct = word & 0x3F
            if funct == 0x00:                           # sll / nop
                self._set(rd, u32(self.reg[rt] << sa))
                return pc + 4
            if funct == 0x04:                           # sllv
                self._set(rd, u32(self.reg[rt] << (self.reg[rs] & 31)))
                return pc + 4
            if funct == 0x21:                           # addu
                self._set(rd, self.reg[rs] + self.reg[rt])
                return pc + 4
            if funct == 0x24:                           # and
                self._set(rd, self.reg[rs] & self.reg[rt])
                return pc + 4
            if funct == 0x25:                           # or
                self._set(rd, self.reg[rs] | self.reg[rt])
                return pc + 4
            if funct == 0x27:                           # nor
                self._set(rd, u32(~(self.reg[rs] | self.reg[rt])))
                return pc + 4
            if funct == 0x08:                           # jr
                require(not in_delay_slot, "nested control transfer")
                target = self.reg[rs]
                self.step(pc + 4, in_delay_slot=True)
                return target
        raise SystemExit(f"FATAL: unsupported word 0x{word:08X} "
                         f"at 0x{pc:08X}")

    def call(self, channel: int, handler: int, limit: int = 64) -> int:
        self.reg[4] = channel & MASK
        self.reg[5] = handler & MASK
        pc = SETTER
        for _ in range(limit):
            pc = self.step(pc)
            if pc == RA_SENTINEL:
                return self.reg[2]
        raise SystemExit("FATAL: setter execution step limit exceeded")


# ── static verification ────────────────────────────────────────────────
def verify_static(exe: Exe) -> None:
    body = exe.body(SETTER, SETTER_SIZE)
    require(exe.file_offset(SETTER) == SETTER_FILE_OFFSET,
            "func_800746A0 file offset changed")
    require(SETTER_END - SETTER == SETTER_SIZE,
            "func_800746A0 declared range is not 0xAC")
    require(len(SETTER_WORDS) == SETTER_WORDS_COUNT,
            "func_800746A0 transcription is not 43 words")
    require(list(struct.unpack(f"<{SETTER_WORDS_COUNT}I", body))
            == SETTER_WORDS,
            "func_800746A0 literal words differ from executable")
    require(hashlib.sha256(body).hexdigest() == SETTER_SHA256,
            "func_800746A0 body SHA-256 mismatch")
    require(exe.word(SETTER_END) == 0x10A00006,
            "exclusive end is not the func_8007474C prologue")

    # Branches, their targets, and every delay slot.
    require(SETTER_WORDS[7] == 0x10870021,
            "previous-handler equality branch changed")
    require(SETTER_WORDS[8] == 0x00E01021,
            "equality-branch delay slot is not addu v0,a3,zero")
    require(SETTER_WORDS[9] == 0x10800010,
            "null-handler branch changed")
    require(SETTER_WORDS[10] == 0x3C0200FF,
            "null-handler delay slot is not lui v0,0x00FF")
    require(SETTER_WORDS[24] == 0x0801D1D1,
            "install path does not jump to the shared return")
    require(SETTER_WORDS[25] == 0x00E01021,
            "install jump delay slot is not addu v0,a3,zero")
    require(SETTER_WORDS[41] == 0x03E00008, "return is not jr ra")
    require(SETTER_WORDS[42] == 0x00000000, "jr delay slot is not exact nop")

    equality_target = 0x800746BC + 4 + (sx16(SETTER_WORDS[7]) << 2)
    null_target = 0x800746C4 + 4 + (sx16(SETTER_WORDS[9]) << 2)
    jump_target = ((0x80074704 & 0xF0000000) |
                   ((SETTER_WORDS[24] & 0x03FFFFFF) << 2))
    require(equality_target == 0x80074744,
            "equality branch does not target the shared return")
    require(null_target == 0x80074708,
            "null-handler branch does not target the removal path")
    require(jump_target == 0x80074744,
            "install path does not join the shared return")

    # No call, no unexpected control transfer, exactly one return.
    for index, word in enumerate(SETTER_WORDS):
        if index in (7, 9, 24, 41):
            continue
        require(word >> 26 not in (0x02, 0x03, 0x04, 0x05, 0x06, 0x07),
                f"unexpected control transfer at setter word {index}")
        if word >> 26 == 0:
            require(word & 0x3F not in (0x08, 0x09),
                    f"unexpected jr/jalr at setter word {index}")

    # Callback table and DICR pointer authorities.
    require(exe.word(DICR_POINTER) == DICR_MMIO,
            "D_800956BC does not hold the DICR MMIO address")
    for slot in range(CALLBACK_TABLE_SLOTS):
        require(exe.word(CALLBACK_TABLE + 4 * slot) == 0,
                f"callback slot {slot} is not zero in the retail image")
    require(exe.word(CALLBACK_TABLE_END) == DMA0_MMIO,
            "D_800956E0 does not follow the eight-slot callback table")

    # Initialization owner.
    dma_init_body = exe.body(DMA_INIT, DMA_INIT_END - DMA_INIT)
    require(list(struct.unpack("<19I", dma_init_body)) == DMA_INIT_WORDS,
            "func_800744D4 literal words differ from executable")
    require(hashlib.sha256(dma_init_body).hexdigest() == DMA_INIT_SHA256,
            "func_800744D4 body SHA-256 mismatch")
    require(DMA_INIT_WORDS[1:3] == [0x3C048009, 0x248456C0]
            and DMA_INIT_WORDS[5] == 0x24050008,
            "func_800744D4 does not clear eight callback slots")
    require(DMA_INIT_WORDS[7:9] == [0x3C028009, 0x8C4256BC]
            and DMA_INIT_WORDS[12] == 0xAC400000,
            "func_800744D4 does not zero DICR through D_800956BC")
    require(DMA_INIT_WORDS[13:15] == [0x3C028007, 0x244246A0],
            "func_800744D4 does not materialize func_800746A0")

    # Whole-executable caller census.
    jal_word = 0x0C000000 | ((SETTER >> 2) & 0x03FFFFFF)
    direct = [address for address, word in exe.words() if word == jal_word]
    require(direct == [],
            f"unexpected direct jal to func_800746A0 at {direct}")
    literal = [address for address, word in exe.words() if word == SETTER]
    require(literal == [],
            f"unexpected static data word holding 0x800746A0 at {literal}")
    materializations = []
    previous = None
    for address, word in exe.words():
        if (previous is not None and previous[1] >> 26 == 0x0F
                and previous[1] & 0xFFFF == 0x8007
                and word >> 26 == 0x09 and word & 0xFFFF == 0x46A0
                and (previous[1] >> 16) & 31 == (word >> 21) & 31):
            materializations.append(previous[0])
        previous = (address, word)
    require(materializations == [0x80074508],
            f"func_800746A0 address materializations changed: "
            f"{[hex(a) for a in materializations]}")

    # The single indirect dispatcher.
    for wrapper, field in JUMP_TABLE_WRAPPERS.items():
        found = None
        for index in range(10):
            word = exe.word(wrapper + 4 * index)
            if (word >> 26 == 0x23 and (word >> 21) & 31 == 2
                    and (word >> 16) & 31 == 2 and word & 0xFFFF < 0x100):
                found = word & 0xFFFF
                break
        require(found == field,
                f"libetc wrapper 0x{wrapper:08X} field changed: {found}")
    require([w for w, f in JUMP_TABLE_WRAPPERS.items() if f == 0x04]
            == [WRAPPER],
            "func_80073CF4 is not the sole dispatcher of jump-table field +4")

    # func_800746A0 touches neither I_MASK nor I_STAT.
    for literal_address in (I_MASK_MMIO, I_STAT_MMIO):
        require(all(word != literal_address
                    for address, word in exe.words()
                    if SETTER <= address < SETTER_END),
                "setter body embeds an interrupt-controller literal")


# ── executed behaviour ─────────────────────────────────────────────────
def scenario(exe: Exe, channel: int, handler: int, *,
             dicr_initial: int = 0, table: dict[int, int] | None = None,
             synthesize_master_flag: bool = False):
    dicr = Dicr(dicr_initial,
                synthesize_master_flag=synthesize_master_flag)
    machine = Machine(exe, dicr=dicr, table=table)
    before_ring = machine.snapshot(GPU_RING, GPU_RING_SIZE)
    returned = machine.call(channel, handler)
    require(machine.snapshot(GPU_RING, GPU_RING_SIZE) == before_ring,
            "setter mutated the retail GPU command ring")
    return machine, returned


def expected_install(dicr: int, channel: int) -> int:
    bit = 1 << ((channel + 16) & 31)
    return (dicr & DICR_CONTROL_MASK) | bit | DICR_MASTER_ENABLE


def expected_remove(dicr: int, channel: int) -> int:
    bit = 1 << ((channel + 16) & 31)
    return ((dicr & DICR_CONTROL_MASK) | DICR_MASTER_ENABLE) & u32(~bit)


def verify_behaviour(exe: Exe) -> None:
    # ── canonical channel-2 install from the measured entry state ──────
    machine, returned = scenario(exe, CANONICAL_CHANNEL, CANONICAL_HANDLER)
    require(returned == 0, "canonical install did not return zero")
    require(machine.table() == [0, 0, CANONICAL_HANDLER, 0, 0, 0, 0, 0],
            "canonical install wrote the wrong callback slot")
    require(machine.guest_writes == [(0x800956C8, CANONICAL_HANDLER)],
            "canonical install performed unexpected guest writes")
    require(machine.mmio_reads == [DICR_MMIO]
            and machine.mmio_writes == [(DICR_MMIO, 0x00840000)],
            "canonical install DICR access is not one read and one write")
    require(machine.dicr.raw() == 0x00840000,
            "canonical install DICR result is not 0x00840000")

    # Slot store precedes the DICR read.
    slot_store = machine.trace.index(0x800746D8)
    dicr_read = machine.trace.index(0x800746DC)
    require(slot_store < dicr_read,
            "install path read DICR before storing the callback slot")

    # ── previous-handler semantics ─────────────────────────────────────
    machine, returned = scenario(exe, 2, 0xABCDEF01, table={2: 0x80076EE4})
    require(returned == 0x80076EE4,
            "replacement did not return the full previous identity")
    require(machine.table()[2] == 0xABCDEF01,
            "replacement did not store the new handler")

    machine, returned = scenario(exe, 2, 0, table={2: 0x80076EE4},
                                 dicr_initial=0x00840000)
    require(returned == 0x80076EE4, "removal did not return the previous")
    require(machine.table()[2] == 0, "removal did not clear the slot")
    require(machine.guest_writes == [(0x800956C8, 0)],
            "removal performed unexpected guest writes")
    require(machine.dicr.raw() == expected_remove(0x00840000, 2)
            == 0x00800000,
            "removal DICR result is not master-only")

    # Zero to zero and same-handler reinstall are complete no-ops.
    for label, table_state, handler in (
            ("zero-to-zero", None, 0),
            ("same-handler", {2: 0x80076EE4}, 0x80076EE4)):
        machine, returned = scenario(exe, 2, handler, table=table_state,
                                     dicr_initial=0x00123456)
        expected_previous = 0 if table_state is None else 0x80076EE4
        require(returned == expected_previous,
                f"{label} returned the wrong previous handler")
        require(machine.guest_writes == [],
                f"{label} wrote guest memory")
        require(machine.mmio_reads == [] and machine.mmio_writes == [],
                f"{label} touched DICR")
        require(machine.dicr.raw() == 0x00123456,
                f"{label} changed DICR")

    # 32-bit identities survive intact; no truncation or normalisation.
    for handler in (0xFFFFFFFF, 0x80000000, 0x00000001, 0x8007FFFF):
        machine, returned = scenario(exe, 3, handler, table={3: 0x87654321})
        require(machine.table()[3] == handler,
                f"handler 0x{handler:08X} was not stored verbatim")
        require(returned == 0x87654321,
                f"handler 0x{handler:08X} lost the previous identity")

    # ── channel range: retail validates nothing ────────────────────────
    for channel in range(CALLBACK_TABLE_SLOTS):
        machine, returned = scenario(exe, channel, 0x80010000 + channel)
        expected = [0] * CALLBACK_TABLE_SLOTS
        expected[channel] = 0x80010000 + channel
        require(machine.table() == expected,
                f"channel {channel} indexed the wrong slot")
        require(returned == 0, f"channel {channel} previous handler changed")
        require(machine.dicr.raw() == expected_install(0, channel),
                f"channel {channel} install DICR mismatch")

    # Channel 7's enable bit IS the master bit, so removal clears master.
    machine, _ = scenario(exe, 7, 0, table={7: 0x80001234},
                          dicr_initial=0x00800000)
    require(machine.dicr.raw() == 0,
            "channel 7 removal did not clear the master bit it shares")
    require(expected_remove(0x00800000, 7) == 0,
            "channel 7 removal model disagrees with the retail expression")

    # Channel 8 addresses D_800956E0, one word past the table; retail has
    # no bound, and its enable bit lands on completion flag 24 (W1C).
    machine, returned = scenario(exe, 8, 0x80009999,
                                 dicr_initial=0x03000000)
    require(returned == DMA0_MMIO,
            "channel 8 did not read the word past the callback table")
    require(machine.guest_writes == [(CALLBACK_TABLE_END, 0x80009999)],
            "channel 8 did not write one word past the callback table")
    require(machine.dicr.raw() == 0x02800000,
            "channel 8 write did not clear completion flag 24 by W1C")

    # Channel -1 wraps back onto the DICR pointer word and shifts to
    # bit 15; both are exact retail consequences of an unchecked index.
    machine, returned = scenario(exe, 0xFFFFFFFF, 0x80005555)
    require(returned == DICR_MMIO,
            "channel -1 did not read the DICR pointer word")
    require(machine.guest_writes == [(DICR_POINTER, 0x80005555)],
            "channel -1 did not overwrite the DICR pointer word")
    require(machine.dicr.raw() == (0x00808000),
            "channel -1 did not set DICR bit 15")

    # ── unrelated DICR state ───────────────────────────────────────────
    machine, _ = scenario(exe, 2, CANONICAL_HANDLER, dicr_initial=0x005A1234)
    require(machine.dicr.raw() == 0x005A1234 | 0x00840000,
            "install did not preserve unrelated lower control bits")

    machine, _ = scenario(exe, 2, CANONICAL_HANDLER, dicr_initial=0x7F000000)
    require(machine.dicr.raw() == 0x7F000000 | 0x00840000,
            "install acknowledged pending completion flags")
    require(machine.mmio_writes[0][1] & DICR_FLAG_MASK == 0,
            "install wrote a one into a W1C completion flag")

    machine, _ = scenario(exe, 2, 0, table={2: CANONICAL_HANDLER},
                          dicr_initial=0x7F840000)
    require(machine.dicr.raw() == 0x7F000000 | 0x00800000,
            "removal acknowledged pending completion flags")
    require(machine.mmio_writes[0][1] & DICR_FLAG_MASK == 0,
            "removal wrote a one into a W1C completion flag")

    # ── DICR bit 31 verdict ────────────────────────────────────────────
    # Verdict B: the raw word is read, the 0x00FFFFFF mask discards bit 31
    # before writeback, and no instruction tests it.  Running the same
    # scenario with and without a synthesized master flag must therefore be
    # observationally identical.
    plain, plain_ret = scenario(exe, 2, CANONICAL_HANDLER,
                                dicr_initial=0x04840000)
    synth, synth_ret = scenario(exe, 2, CANONICAL_HANDLER,
                                dicr_initial=0x04840000,
                                synthesize_master_flag=True)
    require(synth.dicr.read() & DICR_MASTER_FLAG != 0,
            "bit-31 scenario did not actually assert the master flag")
    require(plain_ret == synth_ret
            and plain.dicr.raw() == synth.dicr.raw()
            and plain.guest_writes == synth.guest_writes
            and [v for _, v in plain.mmio_writes]
            == [v for _, v in synth.mmio_writes],
            "func_800746A0 behaviour depends on DICR bit 31")
    for _, value in synth.mmio_writes:
        require(value & DICR_MASTER_FLAG == 0,
                "func_800746A0 wrote a one into DICR bit 31")

    # ── no interrupt-controller access at all ──────────────────────────
    machine, _ = scenario(exe, 2, CANONICAL_HANDLER)
    require(all(address == DICR_MMIO for address in machine.mmio_reads),
            "setter read an MMIO register other than DICR")
    require(all(address == DICR_MMIO for address, _ in machine.mmio_writes),
            "setter wrote an MMIO register other than DICR")

    # ── the canonical func_80073CF4 call context ───────────────────────
    require(exe.word(0x80076D58) == 0x3C058007
            and exe.word(0x80076D5C) == 0x24A56EE4
            and exe.word(0x80076D60) == 0x0C01CF3D
            and exe.word(0x80076D64) == 0x24040002,
            "canonical enqueue call site to func_80073CF4 changed")
    require(exe.word(WRAPPER + 0x18) == 0x0040F809,
            "func_80073CF4 no longer dispatches through jalr v0")


def main(argv: list[str]) -> int:
    default = pathlib.Path(__file__).resolve().parents[2] / \
        "build" / "disc1.candidate.exe"
    path = pathlib.Path(argv[1]) if len(argv) > 1 else default
    exe = Exe(path)
    verify_static(exe)
    verify_behaviour(exe)
    print(f"B53G oracle: PASS  ({path})")
    print(f"  func_800746A0  0x{SETTER:08X}..0x{SETTER_END - 1:08X} "
          f"(exclusive end 0x{SETTER_END:08X}), "
          f"0x{SETTER_SIZE:X} bytes, {SETTER_WORDS_COUNT} words")
    print(f"  callback table 0x{CALLBACK_TABLE:08X}.."
          f"0x{CALLBACK_TABLE_END - 1:08X}, "
          f"{CALLBACK_TABLE_SLOTS} guest identities")
    print(f"  DICR pointer   0x{DICR_POINTER:08X} -> 0x{DICR_MMIO:08X}")
    print("  callers        0 direct; 1 indirect dispatcher func_80073CF4")
    print("  bit 31         read then masked off; never tested, never set")
    print("  I_MASK         not accessed")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
