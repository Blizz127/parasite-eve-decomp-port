#!/usr/bin/env python3
"""Phase 6E-B53I-B1 retail oracle: CPU IRQ reset and registration.

This is an independent executable proof.  It verifies every literal word in
the ResetCallback/source-registration windows used by B1 against the
SHA-exact retail executable, then runs an explicit value-only model of the
B1-relevant recovered ordering.  The model has one 16-bit I_STAT, one 16-bit
I_MASK, and guest 32-bit callback identities.  It has no DMA completion,
DICR edge, interrupt service, callback invocation, queue pump, or wall-clock
progress.

Nothing here imports, links, or calls production C.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
import pathlib
import struct
import sys
from pe_exe_words import exe_words


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
MASK32 = 0xFFFFFFFF

RESET_WRAPPER = 0x80073C94
SOURCE_WRAPPER = 0x80073CC4
RESET_BODY = 0x80073E28
SOURCE_SETTER = 0x800740D0
WORD_CLEAR = 0x80074330
SOURCE0_INIT = 0x800743B4
SOURCE3_INIT = 0x800744D4

I_STAT = 0x1F801070
I_MASK = 0x1F801074
IRQ_BLOCK = 0x800945E4
IRQ_BLOCK_WORDS = 0x41A
IRQ_BLOCK_BYTES = IRQ_BLOCK_WORDS * 4
IRQ_BLOCK_END = IRQ_BLOCK + IRQ_BLOCK_BYTES
IRQ_GUARD = 0x800945E4
CPU_TABLE = 0x800945E8
REGISTERED_MASK = 0x80094614
SDK_JUMP_TABLE = 0x8009564C

SOURCE0 = 0
SOURCE3 = 3
SOURCE0_SLOT = CPU_TABLE + SOURCE0 * 4
SOURCE3_SLOT = CPU_TABLE + SOURCE3 * 4
SOURCE0_HANDLER = 0x8007440C
SOURCE3_HANDLER = 0x80074520


# Complete literal windows.  They are transcribed expectations, not values
# generated from the executable being checked.
WINDOWS = {
    "func_80073C94": (
        0x80073C94, 0x64494,
        "497aefd54b6255dc9291f46bded9c3310f737aa4fbb132ded3aae4a4204d9f93",
        exe_words(0x80073C94, 12),
    ),
    "func_80073CC4": (
        0x80073CC4, 0x644C4,
        "ff71c9ce2b4ad8e8d5afb17da0e186ee953dabddd306f06499c40fc17b3c879e",
        exe_words(0x80073CC4, 12),
    ),
    "func_80073E28": (
        0x80073E28, 0x64628,
        "2fef9e18b8a89258afceb2e3131dd34738ed0585120282beb8ee269a842b6c61",
        exe_words(0x80073E28, 54),
    ),
    "func_800740D0": (
        0x800740D0, 0x648D0,
        "a6991559f3a0292a07423fdaf10d16d6da15eba33543f02dd1e1041aa5715332",
        exe_words(0x800740D0, 82),
    ),
    "func_80074330": (
        0x80074330, 0x64B30,
        "7205982a9f3764f91f1bab001959ef54d1d12176ea93ee300af2b342db7a47df",
        exe_words(0x80074330, 9),
    ),
    "func_800743B4": (
        0x800743B4, 0x64BB4,
        "a8d76ebfddf20244c3904501babf8e4409502f625eefcd21ea740d9fac6c76f2",
        exe_words(0x800743B4, 22),
    ),
    "func_800744D4": (
        0x800744D4, 0x64CD4,
        "e293c6aa8850c852f562fee7c52c855d8b020fad9669159721d8b8e4b152a1be",
        exe_words(0x800744D4, 19),
    ),
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


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

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset + size <= len(self.image),
                f"executable range 0x{address:08X}+0x{size:X}")
        return self.image[offset:offset + size]

    def word(self, address: int) -> int:
        return struct.unpack("<I", self.body(address, 4))[0]


@dataclass
class CpuIrqModel:
    """B1-only state.  There is intentionally no service operation."""

    status: int = 0
    mask: int = 0
    generation: int = 1
    guard: int = 0
    registered: int = 0
    slots: list[int] = field(default_factory=lambda: [0] * 11)
    pad_mode: int = 0
    vblank_mode: int = 0
    events: list[tuple] = field(default_factory=list)
    callback_invocations: int = 0

    def write_status(self, retain_bits: int) -> None:
        self.status &= retain_bits & 0xFFFF
        self.events.append(("I_STAT-W0C", retain_bits & 0xFFFF,
                            self.status))

    def assert_sources(self, sources: int) -> None:
        self.status |= sources & 0xFFFF
        self.events.append(("assert", sources & 0xFFFF, self.status))

    def hardware_reset(self) -> None:
        self.generation += 1
        self.status = 0
        self.mask = 0
        self.guard = 0
        self.registered = 0
        self.slots = [0] * 11
        self.events.append(("hardware-reset", self.generation))

    def set_source(self, source: int, handler: int) -> int:
        previous = self.slots[source]
        handler &= MASK32
        self.events.append(("setter", source, handler, previous))
        if handler == previous:
            self.events.append(("same-handler-return", source))
            return previous
        if self.guard == 0:
            self.events.append(("guard-zero-return", source))
            return previous

        restored = self.mask
        self.mask = 0
        self.events.append(("I_MASK", 0))
        bit = 1 << source
        self.slots[source] = handler
        if handler:
            restored |= bit
            self.registered |= bit
        else:
            restored &= ~bit
            self.registered &= ~bit
        self.mask &= 0xFFFF
        restored &= 0xFFFF
        self.registered &= 0xFFFF
        self.events.append(("slot", source, handler))
        self.events.append(("registered", self.registered))

        if source == SOURCE0:
            removing = int(handler == 0)
            self.pad_mode = removing
            self.events.append(("B(5B)", removing, self.mask))
            self.vblank_mode = removing
            self.events.append(("C(0A)", 3, removing, self.mask))

        self.mask = restored
        self.events.append(("I_MASK", self.mask))
        return previous

    def reset_callback(self) -> None:
        if self.guard:
            self.events.append(("guarded-return",))
            return

        self.mask = 0
        self.events.append(("I_MASK", 0))
        self.write_status(self.mask)
        self.events.append(("DPCR", 0x33333333))

        # Literal func_80074330(D_800945E4, 0x41A): the B1-relevant
        # authority inside this range is guard/table/registered mask.
        self.guard = 0
        self.registered = 0
        self.slots = [0] * 11
        self.events.append(("word-clear", IRQ_BLOCK, IRQ_BLOCK_WORDS,
                            IRQ_BLOCK_END))
        self.guard = 1
        self.events.append(("guard", 1))
        self.set_source(SOURCE0, SOURCE0_HANDLER)
        self.set_source(SOURCE3, SOURCE3_HANDLER)


def verify_static(exe: Exe) -> None:
    for name, (address, file_offset, digest, words) in WINDOWS.items():
        body = exe.body(address, len(words) * 4)
        require(hashlib.sha256(body).hexdigest() == digest,
                f"{name} SHA-256 changed")
        got = list(struct.unpack(f"<{len(words)}I", body))
        require(got == words, f"{name} literal words changed")
        require(0x800 + address - exe.taddr == file_offset,
                f"{name} file offset changed")

    # Retail data authority and wrapper target.
    require(exe.word(0x8009566C) == SDK_JUMP_TABLE,
            "D_8009566C no longer points to the SDK jump table")
    require(exe.word(SDK_JUMP_TABLE + 8) == SOURCE_SETTER,
            "source wrapper field no longer points to func_800740D0")
    require(exe.word(0x80095670) == I_STAT,
            "D_80095670 no longer points to I_STAT")
    require(exe.word(0x80095674) == I_MASK,
            "D_80095674 no longer points to I_MASK")

    # Exact calls and delay slots: source 0 first, then source 3.
    require(exe.word(0x800743E4) == 0x3C058007
            and exe.word(0x800743E8) == 0x24A5440C
            and exe.word(0x800743EC) == 0x0C01CF31
            and exe.word(0x800743F0) == 0x00002021,
            "source-0 func_80073CC4(0, func_8007440C) call changed")
    require(exe.word(0x800744EC) == 0x24040003
            and exe.word(0x800744F8) == 0x3C058007
            and exe.word(0x800744FC) == 0x24A54520
            and exe.word(0x80074500) == 0x0C01CF31
            and exe.word(0x80074504) == 0xAC400000,
            "source-3 func_80073CC4(3, func_80074520) call changed")

    # The reset bulk-clear call and its count-bearing delay slot.
    require(exe.word(0x80073E80) == 0x0C01D0CC
            and exe.word(0x80073E84) == 0x2405041A,
            "ResetCallback 0x41A-word clear changed")
    require(IRQ_BLOCK_END == SDK_JUMP_TABLE,
            "ResetCallback clear no longer ends at the SDK jump table")

    # Source-0 BIOS calls occur before the only final I_MASK restore.
    require(exe.word(0x8007418C) == 0x0C01CF1D
            and exe.word(0x80074190) == 0x02002021,
            "source-0 B(5B) call/delay slot changed")
    require(exe.word(0x80074198) == 0x0C01CF21
            and exe.word(0x8007419C) == 0x02002821,
            "source-0 C(0A) call/delay slot changed")
    require(exe.word(0x800741F0) == 0xA4530000,
            "source setter final I_MASK restore changed")


def verify_model() -> None:
    model = CpuIrqModel(status=0xFFFF, mask=0xA55A, guard=0,
                        registered=0xFFFF,
                        slots=[0xDEADBEEF] * 11)
    model.reset_callback()

    require(model.status == 0, "ResetCallback did not W0C-clear I_STAT")
    require(model.mask == 0x0009, "final I_MASK is not 0x0009")
    require(model.registered == 0x0009,
            "final registered-source mask is not 0x0009")
    require(model.slots[SOURCE0] == SOURCE0_HANDLER,
            "source-0 guest identity changed")
    require(model.slots[SOURCE3] == SOURCE3_HANDLER,
            "source-3 guest identity changed")
    require(all(model.slots[index] == 0
                for index in range(11) if index not in (SOURCE0, SOURCE3)),
            "ResetCallback retained an unrelated CPU callback identity")
    require(model.callback_invocations == 0,
            "registration or assertion delivered a callback")

    source0_setter = model.events.index(
        ("setter", SOURCE0, SOURCE0_HANDLER, 0))
    source0_slot = model.events.index(
        ("slot", SOURCE0, SOURCE0_HANDLER), source0_setter)
    source0_registered = model.events.index(
        ("registered", 0x0001), source0_slot)
    b5b = model.events.index(("B(5B)", 0, 0))
    c0a = model.events.index(("C(0A)", 3, 0, 0))
    source0_restore = model.events.index(("I_MASK", 1), b5b)
    source3_setter = model.events.index(
        ("setter", SOURCE3, SOURCE3_HANDLER, 0))
    source3_slot = model.events.index(
        ("slot", SOURCE3, SOURCE3_HANDLER), source3_setter)
    source3_registered = model.events.index(
        ("registered", 0x0009), source3_slot)
    source3_restore = model.events.index(
        ("I_MASK", 0x0009), source3_registered)
    require(source0_setter < source0_slot < source0_registered < b5b
            < c0a < source0_restore < source3_setter < source3_slot
            < source3_registered < source3_restore,
            "source-0 BIOS/final-mask/source-3 ordering changed")
    require(model.events.count(("B(5B)", 0, 0)) == 1
            and model.events.count(("C(0A)", 3, 0, 0)) == 1,
            "source-0 installation side-call count changed")

    # Same-handler returns before mask repair, proving why coherent reset of
    # the guest table is necessary.  A hardware reset supplies that repair.
    model.mask = 0
    before = list(model.events)
    require(model.set_source(SOURCE3, SOURCE3_HANDLER) == SOURCE3_HANDLER,
            "same-handler return did not preserve the previous identity")
    require(model.mask == 0 and model.registered == 0x0009,
            "same-handler path unexpectedly repaired state")
    require(model.events[len(before):] == [
        ("setter", SOURCE3, SOURCE3_HANDLER, SOURCE3_HANDLER),
        ("same-handler-return", SOURCE3),
    ], "same-handler fast path gained side effects")

    old_generation = model.generation
    model.hardware_reset()
    require(model.generation != old_generation and model.status == 0
            and model.mask == 0 and model.guard == 0
            and model.registered == 0 and model.slots == [0] * 11,
            "hardware reset did not coherently reset IRQ authority")
    model.reset_callback()
    require(model.mask == 0x0009 and model.registered == 0x0009
            and model.slots[SOURCE3] == SOURCE3_HANDLER,
            "reinstallation after coherent reset failed")

    # Masking controls eligibility only.  Assertion always latches status,
    # unmasking retains it, and B1 has no dispatch operation at all.
    model.write_status(0)
    model.mask = 0
    model.assert_sources(0x0008)
    require(model.status == 0x0008 and model.callback_invocations == 0,
            "masked source assertion was dropped or dispatched")
    model.mask = 0x0008
    require(model.status == 0x0008 and model.callback_invocations == 0,
            "unmasking consumed pending status or dispatched")

    # Independent W0C truth table.
    for current, written, expected in (
        (0x0008, 0xFFF7, 0x0000),
        (0x0009, 0xFFFE, 0x0008),
        (0xFFFF, 0xFFFF, 0xFFFF),
        (0xFFFF, 0x0000, 0x0000),
        (0xA55A, 0x0FF0, 0x0550),
    ):
        check = CpuIrqModel(status=current)
        check.write_status(written)
        require(check.status == expected,
                f"W0C {current:04X} & {written:04X} != {expected:04X}")


def main(argv: list[str]) -> int:
    default = pathlib.Path(__file__).resolve().parents[2] / \
        "build" / "disc1.candidate.exe"
    path = pathlib.Path(argv[1]) if len(argv) > 1 else default
    exe = Exe(path)
    verify_static(exe)
    verify_model()
    literal_words = sum(len(window[3]) for window in WINDOWS.values())
    print(f"B53I-B1 IRQ oracle: PASS  ({path})")
    print(f"  retail windows  7 / {literal_words} literal words")
    print(f"  IRQ clear       0x{IRQ_BLOCK:08X}..0x{IRQ_BLOCK_END:08X} "
          f"exclusive ({IRQ_BLOCK_WORDS} words)")
    print("  source 0        slot 0 = 0x8007440C, B(5B)(0), C(0A)(3,0)")
    print("  source 3        slot 3 = 0x80074520")
    print("  final state     I_STAT=0000 I_MASK=0009 registered=0009")
    print("  delivery        absent (no DMA/IRQ/callback service)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
