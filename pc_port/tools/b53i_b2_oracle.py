#!/usr/bin/env python3
"""Phase 6E-B53I-B2 independent DMA/IRQ delivery oracle.

Verifies the complete 96-word func_80074520 body and the literal CPU
source-dispatch/acknowledgement windows against the SHA-exact executable,
then evaluates explicit hardware, DICR, I_STAT, callback, boundary, and reset
scenarios.  It imports no production C and performs no automatic hardware
evolution: completion, edge bridging, and CPU service are separate calls.
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
DICR_FLAGS = 0x7F000000
DICR_FORCE = 0x00008000
DICR_MASTER = 0x00800000
DICR_MASTER_FLAG = 0x80000000
SOURCE3 = 0x0008
CPU_DMA_HANDLER = 0x80074520
DMA2_PUMP_HANDLER = 0x80076EE4


DMA_WORDS = exe_words(0x80074520, 96)

# 0x80073F58..0x8007401F: active publication, eligibility, low-to-high
# source scan, I_STAT W0C, live callback lookup/jalr, and resampling.
CPU_CORE_WORDS = exe_words(0x80073F58, 50)

# 0x800740A4..0x800740AF: the only normal dispatch-active clear is the
# delay slot of the terminal BIOS call.
CPU_TERMINAL_WORDS = [0x3C018009, 0x0C01D0E1, 0xA42045E6]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


class Exe:
    def __init__(self, path: pathlib.Path):
        data = path.read_bytes()
        got = hashlib.sha1(data).hexdigest()
        require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
        require(data[:8] == b"PS-X EXE", f"{path} is not a PS-X EXE")
        self.taddr, self.tsize = struct.unpack_from("<II", data, 0x18)
        require(len(data) == 0x800 + self.tsize,
                "PS-X EXE size/header mismatch")
        self.image = data[0x800:]

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset + size <= len(self.image),
                f"executable range 0x{address:08X}+0x{size:X}")
        return self.image[offset:offset + size]

    def words(self, address: int, count: int) -> list[int]:
        return list(struct.unpack(f"<{count}I", self.body(address, count * 4)))

    def cstring(self, address: int) -> bytes:
        offset = address - self.taddr
        require(offset >= 0 and offset < len(self.image),
                f"string address 0x{address:08X}")
        end = self.image.find(b"\0", offset)
        require(end >= 0, f"unterminated string at 0x{address:08X}")
        return self.image[offset:end]


@dataclass
class Dicr:
    raw: int = 0
    edge: bool = False
    writes: list[int] = field(default_factory=list)

    @staticmethod
    def level(raw: int) -> bool:
        return bool(raw & DICR_FORCE) or bool(
            (raw & DICR_MASTER) and (raw & DICR_FLAGS))

    def commit(self, value: int) -> None:
        old = self.level(self.raw)
        self.raw = value & ~DICR_MASTER_FLAG & MASK32
        self.edge |= not old and self.level(self.raw)

    def read(self) -> int:
        return self.raw | (DICR_MASTER_FLAG if self.level(self.raw) else 0)

    def write(self, value: int) -> None:
        value &= MASK32
        self.writes.append(value)
        flags = (self.raw & DICR_FLAGS) & ~(value & DICR_FLAGS)
        self.commit((value & 0x00FFFFFF) | flags)

    def complete(self, channel: int) -> None:
        enable = 1 << (16 + channel)
        if self.raw & DICR_MASTER and self.raw & enable:
            self.commit(self.raw | (1 << (24 + channel)))

    def take_edge(self) -> bool:
        result = self.edge
        self.edge = False
        return result

    def reset(self) -> None:
        self.raw = 0
        self.edge = False


@dataclass
class Irq:
    status: int = 0
    mask: int = 0
    registered: int = 0
    generation: int = 1
    active: int = 0
    cpu_slots: list[int] = field(default_factory=lambda: [0] * 11)

    def assert_sources(self, bits: int, generation: int) -> bool:
        if generation != self.generation:
            return False
        self.status = (self.status | bits) & 0xFFFF
        return True

    def w0c(self, retain: int) -> None:
        self.status &= retain & 0xFFFF

    def reset(self) -> None:
        self.generation += 1
        self.status = self.mask = self.registered = self.active = 0
        self.cpu_slots = [0] * 11


@dataclass
class Machine:
    dicr: Dicr = field(default_factory=Dicr)
    irq: Irq = field(default_factory=Irq)
    dma_slots: list[int] = field(default_factory=lambda: [0] * 8)
    events: list[tuple] = field(default_factory=list)
    vram_visible: bool = False
    chcr: int = 0x01000201
    dma_active: bool = True
    dma_token: int = 1
    dma_serial: int = 1
    irq_guard: int = 1
    dma_madrs: list[int] = field(default_factory=lambda: [0] * 7)

    def issue_dma(self) -> int:
        require(not self.dma_active, "DMA issue while active")
        self.dma_serial = (self.dma_serial + 1) & 0xFFFFFFFFFFFFFFFF
        if not self.dma_serial:
            self.dma_serial = 1
        self.dma_token = self.dma_serial
        self.dma_active = True
        self.chcr = 0x01000201
        self.vram_visible = False
        return self.dma_token

    def hardware_complete(self, token: int | None = None) -> bool:
        if token is None:
            token = self.dma_token
        if not self.dma_active or token == 0 or token != self.dma_token:
            return False
        self.vram_visible = True
        self.chcr &= ~0x01000000
        self.dma_active = False
        self.dicr.complete(2)
        self.events.append(("hardware-complete", self.dicr.raw))
        return True

    def hardware_reset(self) -> None:
        self.dma_active = False
        self.dma_token = 0
        self.chcr = 0x00000401
        self.dicr.reset()

    def bridge(self, generation: int) -> str:
        if generation != self.irq.generation:
            return "stale"
        if not self.dicr.take_edge():
            return "none"
        require(self.irq.assert_sources(SOURCE3, generation),
                "generation changed inside bridge")
        self.events.append(("source3-edge", self.irq.status))
        return "asserted"

    def dma_dispatch(self, pump: str = "boundary") -> str:
        pending = (self.dicr.read() >> 24) & 0x7F
        while pending:
            snapshot = pending
            channel = 0
            while snapshot and channel < 7:
                if snapshot & 1:
                    flag = 1 << (24 + channel)
                    write = self.dicr.read() & (0x00FFFFFF | flag)
                    self.dicr.write(write)
                    self.events.append(("dma-ack", channel, write))
                    handler = self.dma_slots[channel]
                    if handler:
                        self.events.append(("dma-callback", channel,
                                            handler))
                        if handler == DMA2_PUMP_HANDLER:
                            if pump == "boundary":
                                return "boundary"
                        else:
                            return "unbound"
                snapshot >>= 1
                channel += 1
            pending = (self.dicr.read() >> 24) & 0x7F
        physical = self.dicr.read()
        diagnostic = (physical & 0xFF000000) == 0x80000000
        if not diagnostic:
            diagnostic = bool(self.dicr.read() & DICR_FORCE)
        if diagnostic:
            self.events.append(("dma-diagnostic", self.dicr.read(),
                                tuple(self.dma_madrs)))
        return "returned"

    def cpu_service(self, generation: int, pump: str = "boundary") -> str:
        if generation != self.irq.generation:
            return "stale"
        if not self.irq.status & self.irq.mask:
            return "returned"
        if not self.irq_guard:
            self.events.append(("guard-boundary", self.irq.status))
            return "guard-boundary"
        self.irq.active = 1
        pending = self.irq.registered & self.irq.status & self.irq.mask
        while pending:
            snapshot = pending
            source = 0
            while snapshot and source < 11:
                if snapshot & 1:
                    self.irq.w0c(~(1 << source))
                    self.events.append(("cpu-ack", source,
                                        self.irq.status))
                    handler = self.irq.cpu_slots[source]
                    if handler:
                        self.events.append(("cpu-callback", source,
                                            handler))
                        if handler == CPU_DMA_HANDLER:
                            result = self.dma_dispatch(pump)
                        else:
                            result = "unbound"
                        if result != "returned":
                            return result
                snapshot >>= 1
                source += 1
            pending = self.irq.registered & self.irq.status & self.irq.mask
        self.irq.active = 0
        return "returned"


def canonical_machine() -> Machine:
    m = Machine()
    m.dicr.raw = 0x00840000
    m.irq.mask = m.irq.registered = 0x0009
    m.irq.cpu_slots[0] = 0x8007440C
    m.irq.cpu_slots[3] = CPU_DMA_HANDLER
    m.dma_slots[2] = DMA2_PUMP_HANDLER
    return m


def run_scenarios() -> None:
    # Canonical observable phases and nested non-return.
    m = canonical_machine()
    generation = m.irq.generation
    require(m.hardware_complete(m.dma_token), "canonical completion token")
    require(m.vram_visible and m.chcr == 0x00000201 and
            m.dicr.raw == 0x04840000 and m.dicr.read() == 0x84840000 and
            m.irq.status == 0, "canonical hardware completion state")
    require(m.bridge(generation) == "asserted" and m.irq.status == SOURCE3,
            "canonical source3 bridge")
    require(m.cpu_service(generation) == "boundary", "idle boundary")
    require(m.irq.status == 0 and m.dicr.raw == 0x00840000 and
            m.irq.active == 1, "nested boundary acknowledgement/active")
    require([e[0] for e in m.events] == [
        "hardware-complete", "source3-edge", "cpu-ack", "cpu-callback",
        "dma-ack", "dma-callback"], "canonical event order")

    # Channel-disabled and master-disabled completion: data/CHCR only.
    for raw in (0x00800000, 0x00040000):
        m = canonical_machine()
        m.dicr.raw = raw
        require(m.hardware_complete(m.dma_token), "gated completion token")
        require(m.vram_visible and m.chcr == 0x00000201 and
                not (m.dicr.raw & 0x04000000) and not m.dicr.edge,
                f"completion gating raw=0x{raw:08X}")

    # Masked source retained, then unmasked and dispatched.
    m = canonical_machine()
    generation = m.irq.generation
    require(m.hardware_complete(m.dma_token), "masked completion")
    m.irq.mask = 0x0001
    require(m.bridge(generation) == "asserted", "edge")
    require(m.cpu_service(generation) == "returned" and
            m.irq.status == SOURCE3 and m.dicr.raw == 0x04840000,
            "masked pending retention")
    m.irq.mask = 0x0009
    require(m.cpu_service(generation) == "boundary", "unmasked delivery")

    # Retained flag/master edge and force edge; high levels do not retrigger.
    d = Dicr(0x04040000)
    d.write(0x00840000)
    require(d.read() == 0x84840000 and d.take_edge(), "retained master edge")
    d.write(0x00840000)
    require(not d.take_edge(), "high-level retrigger")
    d.write(0x04000000); d.write(DICR_FORCE)
    require(d.read() == 0x80008000 and d.take_edge(), "force edge")

    # CPU W0C while DICR remains physically high does not create another
    # edge; the bridge consumes transitions rather than polling the level.
    m = canonical_machine(); generation = m.irq.generation
    require(m.hardware_complete(m.dma_token), "high-level completion")
    require(m.bridge(generation) == "asserted", "high-level first bridge")
    m.irq.w0c(0xFFF7)
    require(m.dicr.read() == 0x84840000 and
            m.bridge(generation) == "none" and m.irq.status == 0,
            "high DICR retrigger after I_STAT W0C")

    # A rise survives a fall until the bridge consumes it.
    d = Dicr(0x00840000); d.complete(2); d.write(0x04840000)
    require(d.read() == 0x00840000 and d.take_edge(), "sticky edge")

    # Zero callback still acknowledges; multiple flags scan 0,2,6.
    m = Machine(); m.dicr.raw = 0x00C50000
    for channel in (6, 2, 0): m.dicr.complete(channel)
    require(m.dma_dispatch() == "returned" and
            [e[1] for e in m.events if e[0] == "dma-ack"] == [0, 2, 6],
            "multiple flags / zero callbacks")

    # Nonzero unbound DMA identity stops after DICR acknowledgement.
    m = Machine(); m.dicr.raw = 0x00840000; m.dicr.complete(2)
    m.dma_slots[2] = 0xE2345678
    require(m.dma_dispatch() == "unbound" and
            not (m.dicr.raw & 0x04000000), "unbound DMA boundary/ack")

    # Nonzero unbound CPU identity stops after I_STAT acknowledgement.
    m = canonical_machine(); generation = m.irq.generation
    m.irq.cpu_slots[3] = 0xF1234567; m.irq.status = SOURCE3
    require(m.cpu_service(generation) == "unbound" and
            m.irq.status == 0 and m.irq.active == 1,
            "unbound CPU boundary/ack")

    # Stale generation and reset between completion and CPU service.
    m = canonical_machine(); old = m.irq.generation
    require(m.hardware_complete(m.dma_token), "reset-between completion")
    require(m.bridge(old) == "asserted", "edge")
    m.irq.reset(); m.hardware_reset()
    require(m.cpu_service(old) == "stale" and m.irq.status == 0 and
            m.irq.active == 0, "reset/stale CPU service")
    require(m.bridge(old) == "stale", "stale edge bridge")

    # DMA tokens are explicit and reset invalidates old work. A newly issued
    # post-reset transfer receives a different token; the old token cannot
    # complete it, while the new token can complete it exactly once.
    m = canonical_machine(); old_token = m.dma_token
    m.hardware_reset()
    require(not m.hardware_complete(old_token),
            "reset accepted stale DMA completion token")
    new_token = m.issue_dma()
    require(new_token != old_token and not m.hardware_complete(old_token) and
            m.dma_active, "old token completed new transfer")
    require(m.hardware_complete(new_token) and not m.dma_active and
            not m.hardware_complete(new_token),
            "new token did not complete exactly once")

    # Guard-zero exception entry stops at the B(17h) boundary before active
    # publication or acknowledgement.
    m = canonical_machine(); generation = m.irq.generation
    m.irq_guard = 0; m.irq.status = SOURCE3
    require(m.cpu_service(generation) == "guard-boundary" and
            m.irq.status == SOURCE3 and m.irq.active == 0 and
            m.events == [("guard-boundary", SOURCE3)],
            "guard-zero path entered the CPU scan")

    # The diagnostic tail observes all seven live MADR register values.
    m = Machine(); m.dicr.raw = DICR_FORCE
    m.dma_madrs = [0x10000000 + i * 0x1111 for i in range(7)]
    require(m.dma_dispatch() == "returned" and
            m.events == [("dma-diagnostic", 0x80008000,
                          tuple(m.dma_madrs))],
            "DMA diagnostic tail or MADR order")


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print(f"usage: {argv[0]} RETAIL_PSX_EXE", file=sys.stderr)
        return 2
    exe = Exe(pathlib.Path(argv[1]))
    require(len(DMA_WORDS) == 96, "func_80074520 literal count")
    require(exe.words(0x80074520, 96) == DMA_WORDS,
            "func_80074520 literal body changed")
    require(hashlib.sha256(exe.body(0x80074520, 0x180)).hexdigest() ==
            "3dd9a2f9f85757a6e5c28f1fa6f48cf929370c6ad4da3ae8c08e08dc852259cd",
            "func_80074520 body SHA-256")
    require(len(CPU_CORE_WORDS) == 50, "CPU core literal count")
    require(exe.words(0x80073F58, 50) == CPU_CORE_WORDS,
            "CPU source-dispatch core changed")
    require(exe.words(0x800740A4, 3) == CPU_TERMINAL_WORDS,
            "CPU terminal active-clear window changed")
    require(hashlib.sha256(exe.body(0x80073F00, 0x1D0)).hexdigest() ==
            "4862b3fbc3bb78e49db65b4ebacc3c188f4238d6f4cc9cbd8d2ba3dce59bcace",
            "func_80073F00 body SHA-256")
    require(exe.cstring(0x80011740) == b"unexpected interrupt(%04x)\n",
            "CPU guard diagnostic string")
    require(exe.cstring(0x8001177C) == b"DMA bus error: code=%08x\n",
            "DMA diagnostic header string")
    require(exe.cstring(0x80011798) == b"MADR[%d]=%08x\n",
            "DMA MADR diagnostic string")
    run_scenarios()
    print("B53I-B2 ORACLE PASS")
    print(f"  executable SHA-1  {EXE_SHA1}")
    print("  func_80074520     96/96 words, 0x80074520..0x8007469F")
    print("  CPU IRQ windows   53 literal words + full-body SHA-256")
    print("  explicit cases    17 (no automatic hardware evolution)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
