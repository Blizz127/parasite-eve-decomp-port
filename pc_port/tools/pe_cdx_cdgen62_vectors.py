#!/usr/bin/env python3
"""cdgen62: black-box vectors for the libc formatter 0x80071A84 (sprintf ABI: a0 = destination, a1 = format, a2.. = argument words, words beyond the
fourth in the caller's outgoing area at sp+0x10).  The ORIGINAL code runs from the user's EXE under the test-only interpreter (pe_cdx_vectors.run_case);
the BIOS memory routines the formatter reaches through its two veneers (A(1Bh) strlen, A(2Eh) memchr) run inline with their byte loads logged.
Only synthetic format strings, synthetic %s strings and synthetic integers are used - never in-game text and never retail strings.  Rows of the existing
public retail test table pc_port/tests/bios_sprintf_retail_vectors.tsv (ctest `pe-bios-sprintf-retail-tests`, tool bios_sprintf_oracle.py) are re-run
as inputs and their outputs cross-checked against this oracle (report: agreement count); the table's alternate-form-zero rows ("%#x" of 0 is "0x0", "%#.0o"
of 0 is empty: retail is not host C there) are therefore included.
Sets / classes: tsv (the public table), grid (flag x width x precision x length x value per conversion), star (width / precision from the argument list),
chr, str (long strings, precisions, widths), multi (several conversions, stack arguments), edge (odd destinations, long literal output, invalid
conversions), alt (alternate-form zero, precision zero).
Usage: pe_cdx_cdgen62_vectors.py OUTDIR [--random N] [--seed S]     writes OUTDIR/format.txt (+ .classes) and prints the statistics line."""
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_cdx_cdhost_vectors as g   # noqa: E402

m, c, gv, pl, v = g.m, g.c, g.gv, g.pl, g.v
ENTRY = 0x80071A84
M32 = 0xFFFFFFFF
ZONE = 0x80100000
FMT = 0x80100100
STRS = (0x80100400, 0x80100600, 0x80100800, 0x80100A00, 0x80100C00)
STR_TEXT = ('', 'a', 'abc', 'synthetic string', 'The quick brown fox', 'x' * 100, 'y' * 300, 'MiXeD 0123 ~!@#', 'tab\there', '%d %s %%')
DSTS = (0x80101000, 0x80101001, 0x80101002, 0x80101003, 0x80140000, 0x80150002, 0x801C0000, 0x801C0A01)   # high RAM below the interpreter stack (accesses at 0x801F0000.. are hidden and unfaithful)
TSV = 'pc_port/tests/bios_sprintf_retail_vectors.tsv'
INTS = (0, 1, M32, 2, 7, 8, 9, 10, 15, 16, 42, 99, 100, 255, 256, 1000, 12345, 0xFFFF, 0x10000, 0x7FFF, 0x8000, 0xFFFF8000, 0x7FFFFFFF, 0x80000000,
        0xFFFFFFFE, 0x12345678, 0xDEADBEEF, 0xABCDEF, 0xFFFFFFD6, 0x80000001)
STATS = {}


def setup():
    m.spec(ENTRY, 'libc_formatter', 'wwwwwwwwww', {}, bios=True, batch='cdgen62', n=1)
    sp = m.SPEC[ENTRY]
    sp['budget'] = 2000000
    sp['inline'] = (0x80072334,)
    c.CALL_CAP[0] = 600

    def strlen_h(ram, r, log):
        a = r[4] & 0xFFFFFFFF
        n = 0
        while n < 4096:
            if log(ram, a + n) == 0:
                break
            n += 1
        return n

    def memchr_h(ram, r, log):
        a, ch, cnt = r[4] & 0xFFFFFFFF, r[5] & 0xFF, r[6] & 0xFFFFFFFF
        for i in range(min(cnt, 4096)):
            if log(ram, a + i) == ch:
                return a + i
        return 0
    sp['bios_inline'] = {0x1B: strlen_h, 0x2E: memchr_h}
    return sp


def run(base, sp, sysc, case, seed, flagwords=None):
    dst, fmt, args, strs = case
    ram = bytearray(base)
    if flagwords is not None:       # cdgen62b: the three default flag / width / precision words 0x80094528 / 2C / 30
        struct.pack_into('<III', ram, 0x94528, *[x & M32 for x in flagwords])
    f = fmt.encode('latin-1') + b'\0'
    ram[FMT & 0x1FFFFF:(FMT & 0x1FFFFF) + len(f)] = f
    for addr, text in strs:
        t = text.encode('latin-1') + b'\0'
        ram[addr & 0x1FFFFF:(addr & 0x1FFFFF) + len(t)] = t
    full = [dst, FMT] + [x & M32 for x in args]
    g.reset_models()
    c.UNFAITHFUL[0] = False
    try:
        res = m.run_case(ram, ENTRY, full, {}, sp, sysc, {})
    except (struct.error, IndexError, g.MipsBreak):
        STATS['drop:exception'] = STATS.get('drop:exception', 0) + 1
        return None
    except AssertionError as e:
        key = 'drop:budget' if 'instruction budget' in str(e) else 'drop:unsupported(%s)' % str(e)[:40]
        STATS[key] = STATS.get(key, 0) + 1
        return None
    finally:
        g.reset_models()
    if c.UNFAITHFUL[0]:
        STATS['drop:unfaithful(low-RAM / own-written-mixed load)'] = STATS.get('drop:unfaithful(low-RAM / own-written-mixed load)', 0) + 1
        return None
    ret, loads, stores, calls = res
    ln = gv.line(ENTRY, seed, 0, full, (ret, loads, stores)) + ' RETS - CALL -'
    if len(ln.split()) > 15000:
        STATS['drop:too-long'] = STATS.get('drop:too-long', 0) + 1
        return None
    out = {}
    for a, n, val in stores:
        for i in range(n):
            out[(a + i) & 0x1FFFFF] = (val >> (8 * i)) & 0xFF
    start = dst & 0x1FFFFF
    text = bytearray()
    k = start
    while k in out and out[k] != 0:
        text.append(out[k]); k += 1
    return ln, ret, bytes(text), (k in out and out[k] == 0)


def conv_value(rng, conv):
    if conv in 'di':
        return rng.choice(INTS)
    if conv == 'c':
        return rng.choice((0x41, 0x7A, 0x20, 0, 0x80, 0xFF, 0x141, 0x7F, 0x30, 0x25))
    if conv == 'p':
        return rng.choice((0, 1, 0x2A, 0x8001ABCD, M32, 0x80000000))
    return rng.choice(INTS)


def spec_text(rng, conv, allow_star=True, force=None):
    """Returns (spec string, argument words list) for one conversion; force = dict(flags, width, prec, length)."""
    flags = ''.join(ch for ch in '-+ #0' if rng.random() < 0.3)
    width = rng.choice(('', '', '', '1', '2', '3', '5', '8', '10', '12', '16', '20', '33', '40', '50', '64', '100'))
    prec = rng.choice(('', '', '', '.', '.0', '.1', '.2', '.5', '.8', '.12', '.33', '.40'))
    length = rng.choice(('', '', '', 'h', 'l'))
    if force:
        flags = force.get('flags', flags); width = force.get('width', width); prec = force.get('prec', prec); length = force.get('length', length)
    args = []
    if allow_star and rng.random() < 0.12:
        width = '*'
        args.append(rng.choice((0, 1, 5, 10, 20, M32, 0xFFFFFFF6, 0x7FFFFFFF)))
    if allow_star and rng.random() < 0.12:
        prec = '.*'
        args.append(rng.choice((0, 1, 5, 10, 20, M32, 0xFFFFFFF6)))
    return '%' + flags + width + prec + length + conv, args


def cases(nrandom, seed):
    out = []
    rng = random.Random(seed)
    # tsv: the public retail table as inputs
    try:
        for ln in open(os.path.join(str(v.ROOT), TSV)):
            if ln.startswith('#') or not ln.strip():
                continue
            fmt, words, _ret, want = ln.rstrip('\n').split('\t')
            vals = [int(w, 16) for w in words.split(',')] if words else []
            out.append(('tsv', want[1:-1], (0x80101000, fmt, vals, ())))
    except FileNotFoundError:
        pass
    # alt: alternate-form zero and precision zero over every integer conversion
    for conv in 'xXo':
        for flags in ('', '#', '#0', '#-', '+#', ' #'):
            for prec in ('', '.', '.0', '.1', '.3'):
                for width in ('', '1', '5', '8'):
                    for val in (0, 1, 8, 0x2A, M32):
                        out.append(('alt', None, (DSTS[0], '%' + flags + width + prec + conv + '|', [val], ())))
    # grid: every conversion x flag subset (full) x a few widths / precisions
    for conv in 'diuxXopc':
        for fl in range(32):
            flags = ''.join(ch for i, ch in enumerate('-+ #0') if fl >> i & 1)
            for _ in range(2):
                width = rng.choice(('', '1', '5', '12', '40'))
                prec = rng.choice(('', '.0', '.3', '.12'))
                length = rng.choice(('', 'h', 'l'))
                out.append(('grid', None, (DSTS[0], '%' + flags + width + prec + length + conv + '|', [conv_value(rng, conv)], ())))
    # star
    for _ in range(200):
        conv = rng.choice('diuxXo')
        s_, extra = spec_text(rng, conv, True)
        if not extra:
            s_, extra = '%*d', [rng.choice((0, 5, M32, 20))]
            conv = 'd'
        out.append(('star', None, (DSTS[0], s_ + '|', extra + [conv_value(rng, conv)], ())))
    # chr
    for _ in range(100):
        s_, extra = spec_text(rng, 'c', True)
        out.append(('chr', None, (DSTS[0], s_ + '|', extra + [conv_value(rng, 'c')], ())))
    # str
    for i in range(300):
        s_, extra = spec_text(rng, 's', True)
        sa = rng.choice(STRS)
        strs = [(STRS[j], STR_TEXT[(i + 3 * j) % len(STR_TEXT)]) for j in range(len(STRS))]
        out.append(('str', None, (DSTS[0], s_ + '|', extra + [sa], strs)))
    for i in range(10):
        out.append(('str', None, (DSTS[0], '%s', [0], ())))       # NULL %s: reads kernel RAM in the original (dropped: not representable)
    # multi
    lits = ('|', ' ', ',', '/', '[', ']', 'x=', 'synthetic ', '%%', '-', ':', 'abc')
    for i in range(nrandom):
        n = rng.randint(1, 6)
        fmt, args, strs = '', [], [(STRS[j], STR_TEXT[(i + 2 * j) % len(STR_TEXT)]) for j in range(len(STRS))]
        for _ in range(n):
            fmt += rng.choice(lits)
            conv = rng.choice('diuxXopcs')
            s_, extra = spec_text(rng, conv, True)
            fmt += s_
            args += extra
            args.append(rng.choice(STRS) if conv == 's' else conv_value(rng, conv))
        fmt += rng.choice(('', '', '|', '.'))
        out.append(('multi', None, (rng.choice(DSTS), fmt, args, strs)))
    # edge: odd destinations, long literals, invalid conversions, trailing percent
    for dst in DSTS:
        out.append(('edge', None, (dst, '%d|%s|%x', [-12345, STRS[0], 0xABC], [(STRS[0], 'edge string')])))
    out.append(('edge', None, (DSTS[0], 'L' * 300, [], ())))
    out.append(('edge', None, (DSTS[0], '', [], ())))
    out.append(('edge', None, (DSTS[0], 'plain', [], ())))
    out.append(('edge', None, (DSTS[0], '100%', [], ())))
    out.append(('edge', None, (DSTS[0], '%', [], ())))
    for bad in ('%y', '%n', '%e', '%f', '%g', '%ll d', '%lld', '%hhd', '%zd', '%q', '%5', '%-', '%.', '%l', '%h', '%b', '%a', '%m', '%S', '%C', '%D', '%U', '%O', '%X%', '%#', '%0'):
        for val in (7, 0x80000000):
            out.append(('edge', None, (DSTS[0], bad + '|', [val], ())))
    for width in ('100', '200', '255', '256', '300', '500', '1000'):
        out.append(('edge', None, (DSTS[0], '%' + width + 'd|', [5], ())))
        out.append(('edge', None, (DSTS[0], '%-' + width + 'd|', [5], ())))
        out.append(('edge', None, (DSTS[0], '%.' + width + 'd|', [5], ())))
    return out


def cases_b(seed):
    """cdgen62b: the three default words 0x80094528 (flags) / 0x8009452C (width) / 0x80094530 (precision) swept - the value sets the two suite tables
    (DAY1_formatter_frame, DAY1_card_operation_frame) seed: (0,0,0), (0,12,3), (0x2000,12,3) - plus every single flag bit, edge widths / precisions
    and random words - over synthetic formats that use the conversions the tables use (integers, %c, %p, %s with the string in the data buffer,
    `*` width / precision, length modifiers h l L, %n / %hn / %hhn storing the count through a pointer into the data buffer, the destination, the
    caller's home area), the data buffer 0x80142000 the tables seed, destinations at odd alignments and a destination inside the formatter's own frame
    window, and an unsupported conversion.  Returns [(class, case, flagwords)]."""
    rng = random.Random(seed)
    sets = [(0, 0, 0), (0, 12, 3), (0x2000, 12, 3), (0, 0, 3), (0, 12, 0), (0x2000, 0, 0)]
    sets += [(1 << k, 0, 0) for k in range(16)] + [(1 << k, 12, 3) for k in (0, 1, 2, 3, 4, 5, 8, 12, 13, 15)]
    sets += [(M32, 0, 0), (M32, M32, M32), (0, M32, 0), (0, 0, M32), (0, 40, 0), (0, 0, 40), (0, 0x7FFFFFFF, 0), (0, 0, 0x7FFFFFFF), (0x10, 5, 5), (4, 5, 5)]
    for _ in range(30):
        sets.append((rng.getrandbits(32) & rng.getrandbits(32), rng.choice((0, 1, 3, 12, 40, M32, rng.getrandbits(8))), rng.choice((0, 1, 3, 12, 40, M32, rng.getrandbits(8)))))
    DATA = 0x80142000
    stack_home = 0x801FF00C
    fmts = [('%d', 'int'), ('%i', 'int'), ('%u', 'int'), ('%o', 'int'), ('%x', 'int'), ('%X', 'int'), ('%p', 'ptr'), ('%hd', 'int'), ('%hu', 'int'),
            ('%ld', 'int'), ('%Ld', 'int'), ('%+08d', 'int'), ('%#08x', 'int'), ('%.0d', 'int'), ('%#.0o', 'int'), ('%-12d|', 'int'), ('% 6d', 'int'),
            ('%*d', 'star'), ('%.*u', 'prec'), ('%%:%c', 'chr'), ('%c%c', 'chr2'), ('%#s', 'str'), ('%#.2s', 'str'), ('%s', 'str'), ('%.4s', 'str'), ('%*s', 'starstr'),
            ('A%nB', 'n'), ('A%hnB', 'n'), ('A%hhnB', 'n'), ('xy%d0:ABCDE0000000%c%c', 'mix'), ('zz%ld0:', 'int'), ('A%qB', 'int'), ('plain', 'none'), ('', 'none')]
    out = []
    for i, fw in enumerate(sets):
        for fmt, kind in fmts:
            if (i + len(fmt)) % 3 == 2 and i >= 6:       # thin the full cross product: the first six sets (the table values) run every format
                continue
            val = rng.choice(INTS)
            strs = [(DATA, '\x03abcXYZ'), (STRS[0], 'synthetic')]
            dst = DSTS[0]
            if kind in ('int', 'ptr'):
                args = [val, 0x30, 0x41]
            elif kind == 'star':
                args = [rng.choice((8, 0xFFFFFFF8, 0)), val]
            elif kind == 'prec':
                args = [rng.choice((4, 0xFFFFFFFC, 0)), val]
            elif kind == 'chr':
                args = [rng.choice((0x41, 0, 0xFF))]
            elif kind == 'chr2':
                args = [0x41, 0x7A]
            elif kind == 'str':
                args = [DATA]
            elif kind == 'starstr':
                args = [8, DATA]
            elif kind == 'n':
                target = rng.choice((DATA + 0x20, stack_home, DSTS[0] + 0x80, 0x80143000 - 0x10))
                args = [target, 0]
            elif kind == 'mix':
                args = [val, 0x41, 0x42]
            else:
                args = []
            out.append(('flags', (dst, fmt, args, strs), fw))
            if i < 6 and fmt in ('%d', '%u', '%x', '%s', '%c%c', 'A%nB'):      # the table variants: odd destinations, destination inside the frame window
                for d in (0x80142001, 0x80142002, 0x801FEF90):
                    out.append(('dest', (d, fmt, args, strs), fw))
    return out


def main():
    argv = sys.argv[1:]
    if '--set' in argv and argv[argv.index('--set') + 1] == 'b':
        return main_b(argv)
    nrandom, seed = 700, 1
    for flag in ('--random', '--seed'):
        if flag in argv:
            i = argv.index(flag)
            val = int(argv[i + 1])
            del argv[i:i + 2]
            if flag == '--random':
                nrandom = val
            else:
                seed = val
    outdir = argv[0]
    os.makedirs(outdir, exist_ok=True)
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    sp = setup()
    sysc = m.syscall_pcs(base, ENTRY)
    c._check_callees_complete(base, ENTRY, (), inline=sp['inline'] + (0x80072314, 0x80072324))
    allc = cases(nrandom, seed)
    lines, classes = [], []
    tsv_total = tsv_agree = 0
    for i, (cls, want, case) in enumerate(allc):
        dst, fmt, args, strs = case
        r = run(base, sp, sysc, (dst, fmt, args, strs), 62000 + i)
        if r is None:
            STATS['dropped-class:' + cls] = STATS.get('dropped-class:' + cls, 0) + 1
            continue
        ln, ret, text, term = r
        if cls == 'tsv':
            tsv_total += 1
            tsv_agree += int(text.decode('latin-1') == want)
        lines.append(ln)
        classes.append(cls)
    open(os.path.join(outdir, 'format.txt'), 'w').write('\n'.join(lines) + '\n')
    open(os.path.join(outdir, 'format.txt.classes'), 'w').write(''.join('%d %s\n' % (i + 1, t) for i, t in enumerate(classes)))
    from collections import Counter
    print('format.txt cases=%d tried=%d tsv_rows=%d tsv_output_agrees=%d own_written_loads_excluded=%d mixed=%d classes=%s %s' % (
        len(lines), len(allc), tsv_total, tsv_agree, sum(m.OWNLOAD.values()), sum(m.MIXEDLOAD.values()),
        ','.join('%s:%d' % kv for kv in sorted(Counter(classes).items())), ' '.join('%s=%d' % kv for kv in sorted(STATS.items()))))


def main_b(argv):
    seed = 1
    if '--seed' in argv:
        seed = int(argv[argv.index('--seed') + 1])
    outdir = [x for x in argv if not x.startswith('--') and x != 'b' and not x.isdigit()][0]
    os.makedirs(outdir, exist_ok=True)
    exe = (v.ROOT / 'build/disc1.candidate.exe').read_bytes()
    base = bytes(v.fixture(exe, 0, 0, 0))
    sp = setup()
    sysc = m.syscall_pcs(base, ENTRY)
    c._check_callees_complete(base, ENTRY, (), inline=sp['inline'] + (0x80072314, 0x80072324))
    allc = cases_b(seed)
    lines, classes = [], []
    for i, (cls, case, fw) in enumerate(allc):
        r = run(base, sp, sysc, case, 62500 + i, fw)
        if r is None:
            STATS['dropped-class:' + cls] = STATS.get('dropped-class:' + cls, 0) + 1
            continue
        lines.append(r[0])
        classes.append(cls)
    open(os.path.join(outdir, 'format_b.txt'), 'w').write('\n'.join(lines) + '\n')
    open(os.path.join(outdir, 'format_b.txt.classes'), 'w').write(''.join('%d %s\n' % (i + 1, t) for i, t in enumerate(classes)))
    from collections import Counter
    print('format_b.txt cases=%d tried=%d own_written_loads_excluded=%d mixed=%d classes=%s %s' % (
        len(lines), len(allc), sum(m.OWNLOAD.values()), sum(m.MIXEDLOAD.values()),
        ','.join('%s:%d' % kv for kv in sorted(Counter(classes).items())), ' '.join('%s=%d' % kv for kv in sorted(STATS.items()))))


if __name__ == '__main__':
    main()
