#!/usr/bin/env python3
"""Additional original upload shapes with synthetic, ready GPU scripts.

The existing B53E oracle executes the original worker with scripted GPU, VSync
and timeout providers. This survey uses the already verified local EXE and does
not copy assets or execute the held timeout dependency.
"""
import argparse
import contextlib
import io
import os
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
os.environ.setdefault('PE_EXE_PATH', str(ROOT / 'build/disc1.candidate.exe'))
from b53e_oracle import Exe, run_success


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--start', type=int, default=0)
    parser.add_argument('--count', type=int, default=200)
    options = parser.parse_args()
    if options.start < 0 or options.count < 1:
        parser.error('start must be nonnegative and count positive')
    exe = Exe(ROOT / 'build/disc1.candidate.exe')
    rng = random.Random(0x80076664)
    coverage = {'cpu_only': 0, 'dma_only': 0, 'mixed': 0}
    for case in range(options.start + options.count):
        width, height = rng.randrange(1, 129), rng.randrange(1, 9)
        x, y = rng.randrange(1024), rng.randrange(512)
        pixels = width * height
        words = (pixels + 1) // 2
        blocks, prefix = divmod(words, 16)
        source = 0x80110000
        values = [rng.getrandbits(32) for _ in range(prefix)]
        if case < options.start:
            continue
        coverage['cpu_only' if not blocks else 'dma_only' if not prefix else 'mixed'] += 1
        with contextlib.redirect_stdout(io.StringIO()):
            run_success(exe, name=f'random-{case}', rect=(x, y, width, height),
                        source=source, source_words=values,
                        rect_word1=(height << 16) | width, prefix_words=values,
                        blocks=blocks,
                        arithmetic=(pixels, pixels + 1, 0, pixels + 1, words, blocks))
    print(f'PASS {options.count} original upload shapes from case {options.start}, ready GPU and scripted callbacks;', coverage)


if __name__ == '__main__':
    main()
