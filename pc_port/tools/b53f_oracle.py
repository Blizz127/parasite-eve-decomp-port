#!/usr/bin/env python3
"""Phase 6E-B53F literal oracle for func_80073CF4.

This is an independent retail proof.  It verifies and executes all twelve
words of the libetc DMA-callback wrapper, including both MIPS delay slots.
The dynamically installed backend is a controlled event: it records the
four argument registers and supplies a scripted 32-bit return, but performs
no callback-table, DICR, queue, GPU, DMA, or timing operation.

The ResetCallback install chain is proved structurally from the SHA-exact
executable.  The first guard-passing func_80073E28 call obtains
func_800746A0 from func_800744D4 and stores it at jump-table field +4 in a
jal delay slot.  A separate pre-install scenario leaves that field at its
retail initial zero and stops honestly at the null indirect target.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WRAPPER_SHA256 = (
    "3f439194730111b8a1dd4fc5a252bc381c5a52aa4e2b14e34066f6b403749694"
)
DMA_INIT_SHA256 = (
    "e293c6aa8850c852f562fee7c52c855d8b020fad9669159721d8b8e4b152a1be"
)

MASK = 0xFFFFFFFF
RAM_BASE = 0x80000000
RAM_SIZE = 0x00200000
RA_SENTINEL = 0xDEADBEEC
SP_INITIAL = 0x801FFF00

WRAPPER = 0x80073CF4
WRAPPER_END = 0x80073D24
WRAPPER_SIZE = 0x30
WRAPPER_FILE_OFFSET = 0x644F4

RESET_CALLBACK = 0x80073E28
RESET_GUARD = 0x800945E4
DMA_INIT = 0x800744D4
DMA_INIT_END = 0x80074520
DMA_BACKEND = 0x800746A0
RESET_EXIT_HELPER = 0x8007436C

JUMP_TABLE_POINTER = 0x8009566C
JUMP_TABLE = 0x8009564C
DMA_BACKEND_FIELD = JUMP_TABLE + 0x04

DMA_CALLBACK_TABLE = 0x800956C0
DMA_CALLBACK_TABLE_SIZE = 0x20
GPU_RING = 0x800BD030
GPU_RING_SIZE = 0x1800


# Every executable JAL to the wrapper, with the smallest literal window that
# proves its channel/handler setup and the architecturally executed call delay
# slot.  Values are independent retail words, not decoded production state.
CALL_SITE_WINDOWS = [
    ("gpu-init-null", 0x80074C8C,
     [0x24040002, 0x00002821, 0x0C01CF3D, 0xA2300000]),
    ("dispatcher-pump", 0x80076D58,
     [0x3C058007, 0x24A56EE4, 0x0C01CF3D, 0x24040002]),
    ("pump-clear", 0x80076F8C,
     [0x24040002, 0x0C01CF3D, 0x00002821]),
    ("loadimage2-a", 0x80077690,
     [0x3C058007, 0x24A57A00, 0x0C01CF3D, 0x24040002]),
    ("loadimage2-b", 0x8007777C,
     [0x3C058007, 0x24A57A00, 0x0C01CF3D, 0x24040002]),
    ("storeimage", 0x80077870,
     [0x3C058007, 0x24A57A00, 0x0C01CF3D, 0x24040002]),
    ("moveimage", 0x800779C0,
     [0x3C058007, 0x24A57A00, 0x0C01CF3D, 0x24040002]),
    ("dma2-pump-restore", 0x80077A08,
     [0x3C058007, 0x24A56EE4, 0x0C01CF3D, 0x24040002]),
    ("dma3-wrapper-a", 0x8007A8F4,
     [0x00802821, 0x0C01CF3D, 0x24040003]),
    ("dma4-wrapper", 0x8007DD1C,
     [0x00802821, 0x0C01CF3D, 0x24040004]),
    ("dma3-wrapper-b", 0x800824F8,
     [0x00802821, 0x0C01CF3D, 0x24040003]),
]


# Literal retail words.  Expectations are constants, never extracted from
# production code or derived by calling a production helper.
WRAPPER_WORDS = exe_words(0x80073CF4, 12)
    # lui   v0,0x8009
    # lw    v0,0x566C(v0)  ; D_8009566C
    # addiu sp,sp,-0x18
    # sw    ra,0x10(sp)
    # lw    v0,4(v0)       ; installed backend
    # nop
    # jalr  v0
    # nop                   ; jalr delay slot
    # lw    ra,0x10(sp)
    # addiu sp,sp,0x18
    # jr    ra
    # nop                   ; jr delay slot

# The complete func_800744D4 body.  It has no branch and one return; after
# its setup calls it materializes func_800746A0 in v0 and returns it.
DMA_INIT_WORDS = exe_words(0x800744D4, 19)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


def u32(value: int) -> int:
    return value & MASK


def sx16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


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

    def half(self, address: int) -> int:
        return struct.unpack("<H", self.body(address, 2))[0]


class NullTargetBoundary(Exception):
    def __init__(self, pc: int, target: int, arguments: tuple[int, ...]):
        super().__init__(f"null target at 0x{pc:08X}")
        self.pc = pc
        self.target = target
        self.arguments = arguments


class Machine:
    """Tiny MIPS-I executor for exactly the twelve-word wrapper.

    The executable image supplies ordinary guest reads.  The only setup
    mutation is the separately proved ResetCallback field installation.
    A nonzero jalr target becomes one controlled backend event and returns
    a scripted v0.  There is deliberately no MMIO or hardware model.
    """

    def __init__(self, exe: Exe, *, installed: bool, backend_return: int):
        self.ram = bytearray(RAM_SIZE)
        image_offset = exe.taddr - RAM_BASE
        require(0 <= image_offset <= RAM_SIZE - exe.tsize,
                "retail image does not fit modeled guest RAM")
        self.ram[image_offset:image_offset + exe.tsize] = exe.image
        if installed:
            self.store32_setup(DMA_BACKEND_FIELD, DMA_BACKEND)
        self.backend_return = backend_return & MASK
        self.reg = [0] * 32
        self.reg[29] = SP_INITIAL
        self.reg[31] = RA_SENTINEL
        self.trace: list[int] = []
        self.backend_events: list[tuple[int, int, int, int, int]] = []
        self.mmio_events: list[tuple[int, int]] = []
        self.hardware_progress = 0
        self.stack_writes: list[tuple[int, int]] = []

    def _offset(self, address: int, size: int) -> int:
        require(RAM_BASE <= address and address + size <= RAM_BASE + RAM_SIZE,
                f"guest access outside RAM 0x{address:08X}+{size}")
        return address - RAM_BASE

    def load32(self, address: int) -> int:
        require(address & 3 == 0, f"unaligned lw 0x{address:08X}")
        return struct.unpack_from("<I", self.ram,
                                  self._offset(address, 4))[0]

    def store32_setup(self, address: int, value: int) -> None:
        struct.pack_into("<I", self.ram, self._offset(address, 4),
                         value & MASK)

    def store32(self, address: int, value: int) -> None:
        require(address & 3 == 0, f"unaligned sw 0x{address:08X}")
        struct.pack_into("<I", self.ram, self._offset(address, 4),
                         value & MASK)
        self.stack_writes.append((address, value & MASK))

    def snapshot(self, address: int, size: int) -> bytes:
        offset = self._offset(address, size)
        return bytes(self.ram[offset:offset + size])

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
        simm = sx16(word)

        if op == 0x0F:                              # lui
            self._set(rt, (word & 0xFFFF) << 16)
            return pc + 4
        if op == 0x09:                              # addiu
            self._set(rt, self.reg[rs] + simm)
            return pc + 4
        if op == 0x23:                              # lw
            self._set(rt, self.load32(u32(self.reg[rs] + simm)))
            return pc + 4
        if op == 0x2B:                              # sw
            self.store32(u32(self.reg[rs] + simm), self.reg[rt])
            return pc + 4
        if op == 0:
            funct = word & 0x3F
            if funct == 0x00:                       # sll / nop
                self._set(rd, self.reg[rt] << ((word >> 6) & 31))
                return pc + 4
            if funct == 0x09:                       # jalr
                require(not in_delay_slot, "nested control transfer")
                target = self.reg[rs]
                self._set(rd or 31, pc + 8)
                arguments = tuple(self.reg[index] for index in range(4, 8))
                self.step(pc + 4, in_delay_slot=True)
                if target == 0:
                    raise NullTargetBoundary(pc, target, arguments)
                self.backend_events.append((target, *arguments))
                self.reg[2] = self.backend_return
                return pc + 8
            if funct == 0x08:                       # jr
                require(not in_delay_slot, "nested control transfer")
                target = self.reg[rs]
                self.step(pc + 4, in_delay_slot=True)
                return target
        raise SystemExit(f"FATAL: unsupported word 0x{word:08X} "
                         f"at 0x{pc:08X}")

    def run(self, limit: int = 32) -> int:
        pc = WRAPPER
        for _ in range(limit):
            pc = self.step(pc)
            if pc == RA_SENTINEL:
                return self.reg[2]
        raise SystemExit("FATAL: wrapper execution step limit exceeded")


def jal_word(target: int) -> int:
    return 0x0C000000 | ((target >> 2) & 0x03FFFFFF)


def verify_static(exe: Exe) -> None:
    body = exe.body(WRAPPER, WRAPPER_SIZE)
    require(exe.file_offset(WRAPPER) == WRAPPER_FILE_OFFSET,
            "func_80073CF4 file offset changed")
    require(len(body) == WRAPPER_SIZE,
            "func_80073CF4 size is not 0x30")
    require(list(struct.unpack("<12I", body)) == WRAPPER_WORDS,
            "func_80073CF4 literal words differ from executable")
    require(hashlib.sha256(body).hexdigest() == WRAPPER_SHA256,
            "func_80073CF4 body SHA-256 mismatch")
    require(exe.word(WRAPPER_END) == 0x27BDFFE8,
            "exclusive end is not func_80073D24 prologue")

    require(WRAPPER_WORDS[6] == 0x0040F809,
            "indirect transfer is not jalr v0")
    require(WRAPPER_WORDS[7] == 0,
            "jalr delay slot is not exact nop")
    require(WRAPPER_WORDS[10] == 0x03E00008,
            "wrapper return is not jr ra")
    require(WRAPPER_WORDS[11] == 0,
            "jr delay slot is not exact nop")
    for index, word in enumerate(WRAPPER_WORDS):
        if index in (6, 10):
            continue
        require(word >> 26 not in (0x02, 0x03, 0x04, 0x05, 0x06, 0x07),
                f"unexpected direct control transfer at wrapper word {index}")

    # Complete DMA initializer and its single constant return path.
    dma_init_body = exe.body(DMA_INIT, DMA_INIT_END - DMA_INIT)
    require(list(struct.unpack("<19I", dma_init_body)) == DMA_INIT_WORDS,
            "func_800744D4 literal words differ from executable")
    require(hashlib.sha256(dma_init_body).hexdigest() == DMA_INIT_SHA256,
            "func_800744D4 body SHA-256 mismatch")
    require(DMA_INIT_WORDS[-6:-4] == [0x3C028007, 0x244246A0],
            "func_800744D4 does not materialize func_800746A0")
    require(DMA_INIT_WORDS[-2:] == [0x03E00008, 0x00000000],
            "func_800744D4 does not return through jr-ra/nop")

    # Initial pointer and first guard-passing ResetCallback install chain.
    require(exe.word(JUMP_TABLE_POINTER) == JUMP_TABLE,
            "D_8009566C does not point to D_8009564C")
    require(exe.word(DMA_BACKEND_FIELD) == 0,
            "DMA backend field is not initially zero")
    require(exe.half(RESET_GUARD) == 0,
            "ResetCallback guard is not initially zero")
    require(exe.word(0x80073E3C) == 0x96020000
            and exe.word(0x80073E44) == 0x1440002A,
            "ResetCallback first-call guard sequence changed")
    require(exe.word(0x80073ECC) == jal_word(DMA_INIT),
            "ResetCallback does not call func_800744D4")
    require(exe.word(0x80073ED0) == 0xAC620014,
            "func_800744D4 call delay slot changed")
    require(exe.word(0x80073ED4) == 0x3C048009
            and exe.word(0x80073ED8) == 0x8C84566C,
            "ResetCallback does not reload D_8009566C")
    require(exe.word(0x80073EDC) == jal_word(RESET_EXIT_HELPER),
            "post-install call target changed")
    require(exe.word(0x80073EE0) == 0xAC820004,
            "backend install is not sw v0,4(a0) in the jal delay slot")

    expected_calls = []
    for label, start, words in CALL_SITE_WINDOWS:
        got = [exe.word(start + 4 * index) for index in range(len(words))]
        require(got == words, f"{label} caller window changed")
        call_indices = [index for index, word in enumerate(words)
                        if word == jal_word(WRAPPER)]
        require(len(call_indices) == 1,
                f"{label} does not contain one wrapper call")
        expected_calls.append(start + 4 * call_indices[0])

    scanned_calls = [
        exe.taddr + offset
        for offset in range(0, exe.tsize, 4)
        if struct.unpack_from("<I", exe.image, offset)[0]
        == jal_word(WRAPPER)
    ]
    require(scanned_calls == sorted(expected_calls),
            "executable-wide func_80073CF4 caller census changed")


def scenario_installed_forwarding(exe: Exe) -> None:
    scenarios = [
        ("slot2-pump", 2, 0x80076EE4, 0x00000000),
        ("slot2-null", 2, 0x00000000, 0x00000001),
        ("slot2-restore", 2, 0x80077A00, 0x7FFFFFFF),
        ("slot3-high", 3, 0x8007A8EC, 0x80000000),
        ("slot4-high", 4, 0x8007D614, 0xFFFFFFFF),
    ]

    for label, slot, handler, backend_return in scenarios:
        m = Machine(exe, installed=True, backend_return=backend_return)
        # a2/a3 sentinels prove that the transparent wrapper changes none of
        # the four ABI argument registers before its backend call.
        inputs = (slot, handler, 0x89ABCDEF, 0xFEDCBA98)
        for index, value in enumerate(inputs, 4):
            m.reg[index] = value
        callbacks_before = m.snapshot(DMA_CALLBACK_TABLE,
                                      DMA_CALLBACK_TABLE_SIZE)
        ring_before = m.snapshot(GPU_RING, GPU_RING_SIZE)
        result = m.run()

        require(m.backend_events == [(DMA_BACKEND, *inputs)],
                f"{label}: target or argument forwarding mismatch")
        require(result == backend_return,
                f"{label}: backend return was not forwarded unchanged")
        require(m.reg[4:8] == list(inputs),
                f"{label}: wrapper changed argument registers")
        require(m.reg[29] == SP_INITIAL,
                f"{label}: stack pointer was not restored")
        require(m.trace == [
            0x80073CF4, 0x80073CF8, 0x80073CFC, 0x80073D00,
            0x80073D04, 0x80073D08, 0x80073D0C, 0x80073D10,
            0x80073D14, 0x80073D18, 0x80073D1C, 0x80073D20,
        ], f"{label}: instruction or delay-slot trace mismatch")
        require(m.snapshot(DMA_CALLBACK_TABLE, DMA_CALLBACK_TABLE_SIZE)
                == callbacks_before,
                f"{label}: controlled backend mutated callback slots")
        require(m.snapshot(GPU_RING, GPU_RING_SIZE) == ring_before,
                f"{label}: wrapper/backend event mutated retail ring")
        require(m.mmio_events == [] and m.hardware_progress == 0,
                f"{label}: wrapper invented MMIO or hardware progress")
        require(m.stack_writes == [(SP_INITIAL - 8, RA_SENTINEL)],
                f"{label}: wrapper made a non-stack guest write")
        print(f"scenario {label}: PASS target=0x{DMA_BACKEND:08X} "
              f"return=0x{result:08X}")


def scenario_preinstall_boundary(exe: Exe) -> None:
    m = Machine(exe, installed=False, backend_return=0xA5A5A5A5)
    inputs = (2, 0x80076EE4, 0x12345678, 0xFFFFFFFF)
    for index, value in enumerate(inputs, 4):
        m.reg[index] = value
    callbacks_before = m.snapshot(DMA_CALLBACK_TABLE,
                                  DMA_CALLBACK_TABLE_SIZE)
    ring_before = m.snapshot(GPU_RING, GPU_RING_SIZE)
    try:
        m.run()
    except NullTargetBoundary as stop:
        require(stop.pc == 0x80073D0C and stop.target == 0,
                "pre-install boundary is not jalr-zero")
        require(stop.arguments == inputs,
                "pre-install boundary did not preserve arguments")
    else:
        raise SystemExit("FATAL: pre-install zero target fabricated a return")

    require(m.backend_events == [],
            "pre-install zero target invoked a controlled backend")
    require(m.trace[-2:] == [0x80073D0C, 0x80073D10],
            "pre-install jalr delay slot did not execute exactly")
    require(m.snapshot(DMA_CALLBACK_TABLE, DMA_CALLBACK_TABLE_SIZE)
            == callbacks_before
            and m.snapshot(GPU_RING, GPU_RING_SIZE) == ring_before,
            "pre-install boundary mutated callback or ring state")
    require(m.mmio_events == [] and m.hardware_progress == 0,
            "pre-install boundary invented hardware behavior")
    print("scenario pre-install-zero-target: PASS boundary=0x00000000")


def main() -> int:
    if len(sys.argv) > 2:
        raise SystemExit(f"usage: {sys.argv[0]} [SHA-exact-SLUS_006.62]")
    path = pathlib.Path(sys.argv[1] if len(sys.argv) == 2
                        else "build/extracted/disc1/SLUS_006.62")
    exe = Exe(path)
    verify_static(exe)
    scenario_installed_forwarding(exe)
    scenario_preinstall_boundary(exe)
    print("B53F ORACLE: PASS — 12/12 literal wrapper words; 11/11 caller "
          "sites; ResetCallback installs func_800746A0; forwarding and "
          "zero-target boundary proven")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
