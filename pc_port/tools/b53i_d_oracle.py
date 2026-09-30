#!/usr/bin/env python3
"""Phase 6E-B53I-D independent second-DMA completion oracle.

This model begins at the accepted B53I-C endpoint.  Hardware completion is
an explicit token-specific input; no read, bridge, callback, pump, worker, or
queue operation evolves the DMA automatically.  It imports no production C.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import hashlib
import pathlib
import sys


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
MASK32 = 0xFFFFFFFF
BUSY = 0x01000000
CHCR_LOAD = 0x01000201
DICR_FORCE = 0x00008000
DICR_CH2_ENABLE = 0x00040000
DICR_MASTER = 0x00800000
DICR_CH2_FLAG = 0x04000000
DICR_FLAGS = 0x7F000000
DICR_BIT31 = 0x80000000
SOURCE3 = 0x0008

SOURCE = 0x8012B8B8
MADR = SOURCE
BCR = 0x00020010
DEST_X = 256
DEST_Y = 456
WIDTH = 64
HEIGHT = 1
WORDS = 32

CANONICAL_ENTRY = bytes.fromhex(
    "646607803cd00b80b8b812800001c80140000100" + "00" * 76
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


def physical_dicr(stored: int) -> int:
    level = bool(stored & DICR_FORCE) or bool(
        stored & DICR_MASTER and stored & DICR_FLAGS
    )
    return (stored & ~DICR_BIT31) | (DICR_BIT31 if level else 0)


def source_words() -> list[int]:
    return [
        (0x6000 + i * 2) | ((0x6001 + i * 2) << 16)
        for i in range(WORDS)
    ]


@dataclass
class Machine:
    stored_dicr: int = DICR_MASTER
    i_stat: int = 0
    i_mask: int = 0x0009
    producer: int = 1
    consumer: int = 1
    dma_callback2: int = 0
    marker: int = 1
    drawsync: int = 0
    dispatch_active: int = 0
    gpustat: int = 0x04000000
    vblank: int = 0
    token: int = 2
    serial: int = 2
    active: bool = True
    madr: int = MADR
    bcr: int = BCR
    chcr: int = CHCR_LOAD
    source: list[int] = field(default_factory=source_words)
    vram: dict[tuple[int, int], int] = field(default_factory=dict)
    ring: bytes = CANONICAL_ENTRY
    edge_pending: bool = False
    completion_count: int = 1
    data_order: int = 0
    completion_order: int = 0
    order_counter: int = 2
    checkpoint_calls: int = 0
    token_queries: int = 0
    service_calls: int = 0
    last_captured_token: int = 0
    last_serviced_token: int = 0
    bridge_calls: int = 0
    source3_assertions: int = 0
    irq_service_count: int = 0
    dma_dispatch_count: int = 0
    pump_count: int = 1
    worker_count: int = 1

    def physical(self) -> int:
        return physical_dicr(self.stored_dicr)

    def commit_dicr(self, stored: int) -> None:
        old = bool(self.physical() & DICR_BIT31)
        self.stored_dicr = stored & ~DICR_BIT31 & MASK32
        new = bool(self.physical() & DICR_BIT31)
        if not old and new:
            self.edge_pending = True

    def complete(self, captured: int) -> bool:
        if not self.active or captured == 0 or captured != self.token:
            return False
        for i, word in enumerate(self.source):
            self.vram[(DEST_X + i * 2, DEST_Y)] = word & 0xFFFF
            self.vram[(DEST_X + i * 2 + 1, DEST_Y)] = (word >> 16) & 0xFFFF
        self.order_counter += 1
        self.data_order = self.order_counter
        self.active = False
        self.chcr &= ~BUSY
        if self.stored_dicr & (DICR_MASTER | DICR_CH2_ENABLE) == (
            DICR_MASTER | DICR_CH2_ENABLE
        ):
            self.commit_dicr(self.stored_dicr | DICR_CH2_FLAG)
        self.order_counter += 1
        self.completion_order = self.order_counter
        self.completion_count += 1
        return True

    def bridge(self) -> bool:
        self.bridge_calls += 1
        if not self.edge_pending:
            return False
        self.edge_pending = False
        self.i_stat |= SOURCE3
        self.source3_assertions += 1
        return True

    def later_checkpoint(self) -> str:
        self.checkpoint_calls += 1
        captured = 0
        if self.active:
            self.token_queries += 1
            captured = self.token
            self.last_captured_token = captured
        completed = False
        if captured:
            self.service_calls += 1
            self.last_serviced_token = captured
            completed = self.complete(captured)
        asserted = self.bridge() if self.edge_pending else False
        # B53I-D has no eligible status.  Interrupt dispatch is deliberately
        # not invented by this hardware-only oracle.
        return "returned" if completed or asserted else "idle"

    def reset_hardware(self) -> None:
        self.active = False
        self.token = 0
        self.madr = 0
        self.bcr = 0
        self.chcr = 0x00000401
        self.stored_dicr = 0
        self.edge_pending = False
        self.data_order = 0
        self.completion_order = 0

    def reissue(self, token: int) -> None:
        require(token != 0, "zero reissue token")
        self.serial = token
        self.token = token
        self.active = True
        self.madr = MADR
        self.bcr = BCR
        self.chcr = CHCR_LOAD
        self.data_order = 0
        self.completion_order = 0


def canonical() -> Machine:
    m = Machine()
    blocks = m.bcr >> 16
    words_per_block = m.bcr & 0xFFFF
    dma_words = blocks * words_per_block
    rect_pixels = WIDTH * HEIGHT
    required_words = (rect_pixels + 1) // 2
    source_bytes = dma_words * 4
    require((blocks, words_per_block, dma_words, rect_pixels,
             required_words, source_bytes) == (2, 16, 32, 64, 32, 128),
            "independently derived second-transfer geometry")
    require(len(m.ring) == 0x60, "canonical ring entry length")
    require(m.source[0] == 0x60016000 and m.source[-1] == 0x603F603E,
            "canonical source endpoints")
    require(all(m.vram.get((DEST_X + i, DEST_Y), 0) == 0
                for i in range(WIDTH)), "pre-completion visibility")
    require((m.madr, m.bcr, m.chcr, m.active) ==
            (MADR, BCR, CHCR_LOAD, True), "canonical DMA entry")
    require((m.stored_dicr, m.physical(), m.i_stat, m.i_mask) ==
            (0x00800000, 0x00800000, 0, 0x0009), "canonical IRQ entry")
    return m


def assert_pixels(m: Machine) -> None:
    expected = {(DEST_X + i, DEST_Y) for i in range(WIDTH)}
    require(set(m.vram) == expected,
            "VRAM changed outside the exact 64-pixel destination")
    for i in range(WIDTH):
        require(m.vram.get((DEST_X + i, DEST_Y), 0) == 0x6000 + i,
                f"pixel {i} order/visibility")
    require(m.vram.get((DEST_X - 1, DEST_Y), 0) == 0 and
            m.vram.get((DEST_X + WIDTH, DEST_Y), 0) == 0,
            "adjacent VRAM corruption")


def assert_no_software_progress(m: Machine) -> None:
    require((m.i_stat, m.i_mask) == (0, 0x0009), "IRQ register mutation")
    require((m.producer, m.consumer) == (1, 1), "queue movement")
    require(m.ring == CANONICAL_ENTRY, "ring mutation")
    require((m.dma_callback2, m.marker, m.drawsync) == (0, 1, 0),
            "callback/marker mutation")
    require((m.dispatch_active, m.irq_service_count, m.dma_dispatch_count) ==
            (0, 0, 0), "CPU/DMA dispatcher entered")
    require((m.pump_count, m.worker_count) == (1, 1),
            "pump/worker re-entry")
    require((m.gpustat, m.vblank) == (0x04000000, 0),
            "GPUSTAT/VBlank mutation")


def run_scenarios() -> int:
    count = 0

    # 1 canonical interrupt-disabled second completion.
    m = canonical()
    token = m.token
    require(m.complete(token), "canonical token rejected")
    assert_pixels(m)
    require((m.chcr, m.stored_dicr, m.physical(), m.edge_pending) ==
            (0x00000201, 0x00800000, 0x00800000, False),
            "canonical completion endpoint")
    require((m.data_order, m.completion_order, m.completion_count) == (3, 4, 2),
            "visibility/completion ordering")
    assert_no_software_progress(m)
    count += 1

    # 2 channel enabled + master enabled.
    m = canonical()
    m.stored_dicr = 0x00840000
    require(m.complete(m.token), "enabled completion rejected")
    require((m.stored_dicr, m.physical(), m.edge_pending, m.i_stat) ==
            (0x04840000, 0x84840000, True, 0), "enabled gating")
    count += 1

    # 3 channel disabled, master retained.
    m = canonical()
    require(m.complete(m.token) and m.stored_dicr == 0x00800000 and
            m.physical() == 0x00800000 and not m.edge_pending,
            "channel-disabled gating")
    count += 1

    # 4 channel enabled, master disabled.
    m = canonical()
    m.stored_dicr = 0x00040000
    require(m.complete(m.token) and m.stored_dicr == 0x00040000 and
            m.physical() == 0x00040000 and not m.edge_pending,
            "master-disabled gating")
    count += 1

    # 5 already-completed token.
    m = canonical()
    require(m.complete(m.token) and not m.complete(m.token) and
            m.completion_count == 2, "token completed twice")
    count += 1

    # 6 wrong/stale token is inert.
    m = canonical()
    frozen = (m.active, m.chcr, dict(m.vram), m.completion_count)
    require(not m.complete(m.token - 1) and
            frozen == (m.active, m.chcr, m.vram, m.completion_count),
            "stale token changed hardware")
    count += 1

    # 7 reset/reissue rejects old identity but preserves the new one.
    m = canonical()
    old = m.token
    m.reset_hardware()
    m.reissue(old + 1)
    require(not m.complete(old) and m.active and m.token == old + 1,
            "pre-reset token completed reissued DMA")
    require(m.complete(old + 1), "post-reset token rejected")
    count += 1

    # 8 unrelated retained DICR flag survives disabled channel2 completion.
    m = canonical()
    m.stored_dicr = 0x40800055
    require(m.physical() == 0xC0800055 and m.complete(m.token) and
            m.stored_dicr == 0x40800055 and m.physical() == 0xC0800055,
            "unrelated DICR flag/control changed")
    count += 1

    # 9 an already-high force level does not retrigger on completion.
    m = canonical()
    m.stored_dicr = 0x00808000
    require(m.physical() == 0x80808000 and m.complete(m.token) and
            m.physical() == 0x80808000 and not m.edge_pending,
            "force-bit completion retrigger")
    count += 1

    # 10 canonical checkpoint has no source edge.
    m = canonical()
    require(m.later_checkpoint() == "returned" and m.bridge_calls == 0 and
            m.source3_assertions == 0 and m.i_stat == 0,
            "canonical checkpoint called the bridge or asserted source3")
    count += 1

    # 11 no pump.
    require(m.pump_count == 1, "pump re-entered")
    count += 1

    # 12 no worker.
    require(m.worker_count == 1, "worker re-entered")
    count += 1

    # 13 queue and callback state stay frozen.
    assert_no_software_progress(m)
    count += 1

    # 14 I_MASK is hardware-completion-inert.
    m = canonical()
    m.i_mask = 0xBEEF
    require(m.complete(m.token) and m.i_mask == 0xBEEF and m.i_stat == 0,
            "alternate I_MASK changed")
    count += 1

    # 15 repeated runs are deterministic and one-token checkpoints do not
    # requery after completion.
    endpoints = []
    for _ in range(3):
        m = canonical()
        require(m.later_checkpoint() == "returned", "repeat checkpoint")
        require((m.checkpoint_calls, m.token_queries, m.service_calls,
                 m.last_captured_token, m.last_serviced_token) ==
                (1, 1, 1, m.token, m.token),
                "checkpoint did not capture/service its explicit token once")
        require(m.later_checkpoint() == "idle" and
                (m.checkpoint_calls, m.token_queries, m.service_calls,
                 m.bridge_calls) == (2, 1, 1, 0),
                "empty checkpoint requeried or re-serviced current token")
        endpoints.append((m.chcr, m.stored_dicr, m.physical(),
                          m.completion_count, tuple(sorted(m.vram.items()))))
    require(endpoints[0] == endpoints[1] == endpoints[2],
            "repeated completion nondeterminism")
    count += 1

    require(count == 15, "scenario count is not 15")
    return count


def main(argv: list[str]) -> int:
    require(len(argv) == 2, "usage: b53i_d_oracle.py executable")
    path = pathlib.Path(argv[1])
    data = path.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
    require(data[:8] == b"PS-X EXE", "not a PS-X EXE")
    text_size = int.from_bytes(data[0x1C:0x20], "little")
    require(len(data) == 0x800 + text_size,
            "PS-X EXE header/file-size mismatch")
    scenarios = run_scenarios()
    print(f"B53I-D oracle: PASS ({scenarios}/{scenarios} scenarios; {path})")
    print("  second DMA  token=2 MADR/BCR/CHCR=8012B8B8/00020010/01000201")
    print("  completion  64 pixels visible; CHCR=00000201; DICR=00800000")
    print("  delivery    no flag26, bit31, source3, pump, worker, or queue progress")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
