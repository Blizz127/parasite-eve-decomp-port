#!/usr/bin/env python3
"""Compare independent queue-pump state model to the verified retail oracle.

Hardware readiness and callback/worker outcomes are scripted. This checks
finite states, not autonomous DMA completion or real-device timing.
"""
import argparse
import os
import pathlib
import random

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--exe", required=True, type=pathlib.Path,
                    help="owner's verified Disc 1 executable")
args = parser.parse_args()
os.environ["PE_EXE_PATH"] = str(args.exe.resolve())
import b53h_oracle as retail
import b53i_c_oracle as model
rng = random.Random(0x76ee4)
exe = retail.Exe(args.exe)
retail.verify_static(exe)
for case in range(200):
    consumer = rng.randrange(64)
    count = rng.randrange(6)
    producer = (consumer + count) & 63
    busy = bool(rng.randrange(4) == 0)
    issue = bool(rng.randrange(2))
    marker = rng.randrange(2)
    callback = 0x80012345 if rng.randrange(2) else 0
    imask = rng.randrange(65536)
    chcr = 0x201 | (0x1000000 if busy else 0)
    ent_r = {}
    ent_m = {}
    for j in range(count):
        slot = (consumer+j)&63
        arg = retail.RING + slot * retail.RING_STRIDE + 12
        aux = rng.randrange(0x80010000, 0x8001fffc, 4)
        ent_r[slot] = (model.WORKER, arg, aux)
        ent_m[slot] = model.Entry(model.WORKER, arg, aux)
    r = retail.Machine(exe, chcr=chcr, gpustat=retail.GPUSTAT_READY,
        producer=producer, consumer=consumer, entries=ent_r, work_marker=marker,
        drawsync_callback=callback, imask=imask, worker_starts_dma=issue)
    m = model.PumpModel(producer=producer, consumer=consumer, entries=ent_m,
        chcr=chcr, gpustat=(model.READY,), imask=imask, marker=marker,
        drawsync=callback, worker_mode='issue' if issue else 'failure',
        drawsync_returns=True)
    got_r, got_m = r.run(), m.pump()
    checks = {
      'return': (got_r,got_m), 'consumer': (r.get32(retail.CONSUMER),m.consumer),
      'imask': (r.imask,m.imask), 'marker': (r.get32(retail.WORK_MARKER),m.marker),
      'worker calls': (r.worker_calls,m.worker_calls),
      'deregister count': (len(r.deregister_calls),m.trace.count('callback_remove')),
      'callback count': (len(r.callback_calls),m.trace.count('drawsync_call')),
      'DMA busy': (bool(r.chcr & retail.CHCR_BUSY),bool(m.chcr & model.BUSY)),
    }
    for label,(a,b) in checks.items():
        if a != b: raise AssertionError(f'case={case} {label}: retail={a!r} model={b!r}')
print('PASS 200 seeded original-vs-independent-model cases; scripted ready GPU, worker and callback')
