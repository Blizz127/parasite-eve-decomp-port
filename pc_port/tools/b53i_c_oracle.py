#!/usr/bin/env python3
"""Phase 6E-B53I-C independent idle-pump contract oracle.

The accepted B53H oracle owns the independently transcribed 152 literal
words.  This rung reuses that literal array, verifies it again against the
SHA-exact executable, verifies every control-transfer delay slot in the full
function, and executes an independent state model of the idle consumer.

All hardware evolution is explicit input.  GPUSTAT reads never make the GPU
ready, a worker may issue but never completes DMA, and callback identities
remain 32-bit guest values.  No production C is imported or invoked.
"""

from __future__ import annotations

from dataclasses import dataclass
import hashlib
import pathlib
import struct
import sys

import b53h_oracle as literal


EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
FULL_SHA256 = (
    "a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c"
)
IDLE_SHA256 = (
    "fed73d363d43e4ce9ada5534224fca37a044bdb2fc590f494d6b1fb413c954c4"
)
PUMP = 0x80076EE4
IDLE = 0x80076F10
EPILOGUE = 0x80077130
END = 0x80077144
FILE_OFFSET = 0x676E4
IDLE_FILE_OFFSET = 0x67710

READY = 0x04000000
BUSY = 0x01000000
DMA_CB = 0x80076EE4
WORKER = 0x80076664
RING = 0x800BD030
SOURCE = 0x8012B8B8


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FATAL: {message}")


class Exe:
    def __init__(self, path: pathlib.Path):
        self.data = path.read_bytes()
        got = hashlib.sha1(self.data).hexdigest()
        require(got == EXE_SHA1, f"{path} SHA-1 {got} != {EXE_SHA1}")
        require(self.data[:8] == b"PS-X EXE", "not a PS-X EXE")
        self.taddr, self.tsize = struct.unpack_from("<II", self.data, 0x18)
        self.image = self.data[0x800:]
        require(len(self.image) == self.tsize, "EXE header size mismatch")

    def body(self, address: int, size: int) -> bytes:
        offset = address - self.taddr
        require(0 <= offset <= len(self.image) - size,
                f"range 0x{address:08X}+0x{size:X} outside EXE")
        return self.image[offset:offset + size]

    def word(self, address: int) -> int:
        return struct.unpack("<I", self.body(address, 4))[0]

    def file_offset(self, address: int) -> int:
        return 0x800 + address - self.taddr


DELAY_SLOTS = {
    0x80076F08: 0x24020001,
    0x80076F10: 0x00002021,
    0x80076F2C: 0xAC225880,
    0x80076F4C: 0x00000000,
    0x80076F74: 0x00000000,
    0x80076F88: 0x24040002,
    0x80076F90: 0x00002821,
    0x80076FB0: 0x3C040400,
    0x80076FC4: 0x00000000,
    0x80077034: 0x00000000,
    0x8007706C: 0x00000000,
    0x8007708C: 0x00000000,
    0x8007709C: 0x00000000,
    0x800770B8: 0x00000000,
    0x800770D8: 0x00000000,
    0x800770F0: 0x00000000,
    0x80077100: 0x2462FFF8,
    0x8007710C: 0x00000000,
    0x8007713C: 0x27BD0020,
}


def verify_literal(exe: Exe) -> None:
    body = exe.body(PUMP, END - PUMP)
    idle = exe.body(IDLE, EPILOGUE - IDLE)
    words = list(struct.unpack("<152I", body))
    require(exe.file_offset(PUMP) == FILE_OFFSET,
            "full function file offset is not 0x676E4")
    require(exe.file_offset(IDLE) == IDLE_FILE_OFFSET,
            "idle suffix file offset is not 0x67710")
    require(words == literal.PUMP_WORDS,
            "reused independent 152-word transcription differs from EXE")
    require(hashlib.sha256(body).hexdigest() == FULL_SHA256,
            "full function SHA-256 mismatch")
    require(len(idle) == 0x220 and len(idle) // 4 == 136,
            "idle suffix is not 0x220 bytes / 136 words")
    require(hashlib.sha256(idle).hexdigest() == IDLE_SHA256,
            "idle suffix SHA-256 mismatch")
    for pc, delay in DELAY_SLOTS.items():
        require(exe.word(pc + 4) == delay,
                f"delay slot at 0x{pc + 4:08X} differs")
    require(exe.word(0x80076F90) == 0x0C01CF3D,
            "callback removal is not func_80073CF4")
    require(exe.word(0x80077034) == 0x0040F809,
            "worker call is not jalr v0")
    require(exe.word(0x80077054) == 0xAC225878,
            "consumer store is not exactly PC 0x80077054")
    require(exe.word(0x8007709C) == 0x0C01CF84,
            "I_MASK restoration call differs")
    require(exe.word(0x80077108) == 0xAC400008 and
            exe.word(0x8007710C) == 0x0080F809,
            "marker-clear-before-DrawSync order differs")


@dataclass
class Entry:
    worker: int
    argument: int
    auxiliary: int
    rect0: int = 0x01C80100
    rect1: int = 0x00010040


class Boundary(Exception):
    def __init__(self, kind: str, target: int = 0):
        self.kind = kind
        self.target = target & 0xFFFFFFFF


class WaitHeld(Exception):
    pass


class PumpModel:
    """State-transition model driven only by explicit scenario inputs."""

    def __init__(self, *, producer=0, consumer=0, entries=None,
                 chcr=0x00000201, gpustat=(READY,), imask=0,
                 dicr=0x00840000, dma_callback=DMA_CB,
                 marker=1, drawsync=0, worker_mode="issue",
                 drawsync_returns=False):
        self.producer = producer & 0xFFFFFFFF
        self.consumer = consumer & 0xFFFFFFFF
        self.entries = dict(entries or {})
        self.chcr = chcr & 0xFFFFFFFF
        self.gpustat = list(gpustat)
        self.imask = imask & 0xFFFF
        self.saved_imask = 0
        self.dicr = dicr & 0x7FFFFFFF
        self.dma_callback = dma_callback & 0xFFFFFFFF
        self.marker = marker & 0xFFFFFFFF
        self.drawsync = drawsync & 0xFFFFFFFF
        self.worker_mode = worker_mode
        self.drawsync_returns = drawsync_returns
        self.trace: list[str] = []
        self.worker_calls: list[tuple[int, int, int]] = []
        self.consumer_stores: list[int] = []
        self.gpustat_reads = 0
        self.returned = False
        self.dma_active = bool(self.chcr & BUSY)
        self.dma_madr = 0
        self.dma_bcr = 0
        self.dma_completed = False
        self.completion_count = 0

    def read_status(self) -> int:
        self.gpustat_reads += 1
        if not self.gpustat:
            raise WaitHeld()
        value = self.gpustat.pop(0)
        if not self.gpustat:
            self.gpustat.append(value)
        return value

    def remove_callback(self) -> None:
        self.trace.append("callback_remove")
        if self.dma_callback != 0:
            self.dma_callback = 0
            controls = ((self.dicr & 0x00FFFFFF) | 0x00800000)
            controls &= ~0x00040000
            retained_flags = self.dicr & 0x7F000000
            self.dicr = retained_flags | controls

    def call_worker(self, entry: Entry) -> None:
        self.trace.append("worker")
        self.worker_calls.append(
            (entry.worker, entry.argument, entry.auxiliary))
        if entry.worker != WORKER or self.worker_mode == "boundary":
            raise Boundary("worker", entry.worker)
        if self.worker_mode == "issue":
            require(entry.rect0 == 0x01C80100 and
                    entry.rect1 == 0x00010040,
                    "canonical worker received the wrong RECT")
            self.dma_madr = entry.auxiliary
            self.dma_bcr = 0x00020010
            self.chcr = 0x01000201
            self.dma_active = True
            self.trace.append("dma_issue")
        elif self.worker_mode != "failure":
            raise SystemExit(f"FATAL: unknown worker mode {self.worker_mode}")

    def pump(self) -> int:
        if self.chcr & BUSY:
            self.returned = True
            return 1

        old = self.imask
        self.imask = 0
        self.saved_imask = old
        self.trace.append("mask_disable")
        if self.producer != self.consumer and not (self.chcr & BUSY):
            while True:
                next_consumer = (self.consumer + 1) & 63
                if next_consumer == self.producer and self.drawsync == 0:
                    self.remove_callback()
                while not (self.read_status() & READY):
                    pass
                self.trace.append("ready")
                entry = self.entries.get(self.consumer)
                if entry is None:
                    raise Boundary("ring_span")
                # Argument, auxiliary, then worker are live values in retail.
                argument = entry.argument
                auxiliary = entry.auxiliary
                worker = entry.worker
                self.call_worker(Entry(worker, argument, auxiliary,
                                       entry.rect0, entry.rect1))
                self.consumer = (self.consumer + 1) & 63
                self.consumer_stores.append(self.consumer)
                self.trace.append("consumer_store")
                if self.producer == self.consumer or (self.chcr & BUSY):
                    break

        self.imask = self.saved_imask
        self.trace.append("mask_restore")
        if (self.producer == self.consumer and not (self.chcr & BUSY)
                and self.marker != 0 and self.drawsync != 0):
            self.marker = 0
            self.trace.append("marker_clear")
            self.trace.append("drawsync_call")
            if not self.drawsync_returns:
                raise Boundary("drawsync", self.drawsync)
        result = (self.producer - self.consumer) & 63
        self.returned = True
        return result


def canonical_entry(slot=0) -> dict[int, Entry]:
    return {slot: Entry(WORKER, RING + slot * 0x60 + 0x0C, SOURCE)}


def run_scenarios() -> int:
    count = 0

    # 1 canonical idle queue.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(), imask=9)
    require(m.pump() == 0 and m.returned and m.consumer == 1, "canonical")
    require(m.dma_callback == 0 and m.dicr == 0x00800000,
            "canonical callback/DICR")
    count += 1

    # 2 empty queue.
    m = PumpModel(producer=7, consumer=7, entries={}, imask=0xBEEF)
    require(m.pump() == 0 and m.imask == 0xBEEF and not m.worker_calls,
            "empty queue")
    count += 1

    # 3 busy regression.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  chcr=0x01000201, gpustat=(0,), imask=9)
    require(m.pump() == 1 and m.imask == 9 and not m.trace, "busy path")
    count += 1

    # 4 held-not-ready: removal and mask-zero persist, but no issue/consume.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  gpustat=())
    try:
        m.pump()
        require(False, "held GPUSTAT returned")
    except WaitHeld:
        pass
    require(m.imask == 0 and m.consumer == 0 and not m.worker_calls and
            m.dma_callback == 0, "held GPUSTAT mutation")
    count += 1

    # 5 explicit ready transition.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  gpustat=(0, 0, READY))
    require(m.pump() == 0 and m.gpustat_reads == 3, "ready transition")
    count += 1

    # 6 installed slot is removed for final/no-DrawSync.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  dma_callback=DMA_CB)
    m.pump()
    require(m.trace.index("callback_remove") < m.trace.index("worker") and
            m.dma_callback == 0, "installed callback removal")
    count += 1

    # 7 already-zero callback follows the same call but remains zero.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  dma_callback=0, dicr=0x00800000)
    m.pump()
    require(m.dma_callback == 0 and m.dicr == 0x00800000,
            "already-zero callback")
    count += 1

    # 8 canonical typed worker ABI.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry())
    m.pump()
    require(m.worker_calls == [(WORKER, RING + 0x0C, SOURCE)], "worker ABI")
    count += 1

    # 9 unbound worker is a boundary after removal, before consume/restore.
    m = PumpModel(producer=1, consumer=0,
                  entries={0: Entry(0xF1234567, RING + 0x0C, SOURCE)},
                  imask=0x1234)
    try:
        m.pump()
        require(False, "unbound worker returned")
    except Boundary as cut:
        require(cut.kind == "worker" and cut.target == 0xF1234567,
                "unbound identity")
    require(m.consumer == 0 and m.imask == 0 and
            m.trace[-1] == "worker", "unbound worker prefix")
    count += 1

    # 10 normal success consumes and restores.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(), imask=7)
    m.pump()
    require(m.consumer_stores == [1] and m.imask == 7, "worker success")
    count += 1

    # 11 ordinary worker failure is still an ordinary return and consumes.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  worker_mode="failure")
    require(m.pump() == 0 and m.consumer == 1 and not m.dma_active,
            "worker failure")
    count += 1

    # 12 consumer wrap.
    m = PumpModel(producer=0, consumer=63, entries=canonical_entry(63),
                  worker_mode="failure")
    require(m.pump() == 0 and m.consumer == 0, "consumer wrap")
    count += 1

    # 13 alternate indices/non-last item retains slot and leaves one pending.
    m = PumpModel(producer=2, consumer=0, entries=canonical_entry(), imask=9)
    require(m.pump() == 1 and m.consumer == 1 and
            m.dma_callback == DMA_CB, "non-last entry")
    count += 1

    # 14 arbitrary saved mask is restored, never hard-coded.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  imask=0xA55A)
    m.pump()
    require(m.saved_imask == 0xA55A and m.imask == 0xA55A,
            "alternate mask")
    count += 1

    # 15 zero DrawSync callback leaves marker set.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  marker=1, drawsync=0, worker_mode="failure")
    m.pump()
    require(m.marker == 1 and "drawsync_call" not in m.trace,
            "zero DrawSync")
    count += 1

    # 16 reachable DrawSync return: marker clears before callback.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry(),
                  marker=1, drawsync=0x80012345,
                  worker_mode="failure", drawsync_returns=True)
    require(m.pump() == 0 and m.marker == 0 and
            m.trace.index("mask_restore") < m.trace.index("marker_clear") <
            m.trace.index("drawsync_call"), "DrawSync return branch")
    count += 1

    # 17 exact second DMA issue.
    m = PumpModel(producer=1, consumer=0, entries=canonical_entry())
    m.pump()
    require((m.dma_madr, m.dma_bcr, m.chcr, m.dma_active) ==
            (SOURCE, 0x00020010, 0x01000201, True), "second DMA issue")
    count += 1

    # 18 issue never implies completion in this model.
    require(not m.dma_completed and m.completion_count == 0,
            "automatic second completion")
    count += 1

    # 19 removal is before issue.
    require(m.trace.index("callback_remove") < m.trace.index("dma_issue"),
            "callback removal after issue")
    count += 1

    # 20 worker call is before the sole consumer store.
    require(m.trace.index("worker") < m.trace.index("consumer_store"),
            "consumer advanced before worker")
    count += 1

    require(count == 20, "scenario count is not 20")
    return count


def main(argv: list[str]) -> int:
    default = pathlib.Path(__file__).resolve().parents[2] / \
        "build" / "disc1.candidate.exe"
    path = pathlib.Path(argv[1]) if len(argv) > 1 else default
    exe = Exe(path)
    verify_literal(exe)
    scenarios = run_scenarios()
    print(f"B53I-C oracle: PASS ({scenarios}/{scenarios} scenarios; {path})")
    print("  func_80076EE4  0x80076EE4..0x80077143, 152 words")
    print("  idle suffix    0x80076F10..0x8007712F, 136 words")
    print("  consumer store 0x80077054; callback removal and worker precede it")
    print("  hardware       explicit inputs only; issued DMA never auto-completes")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
