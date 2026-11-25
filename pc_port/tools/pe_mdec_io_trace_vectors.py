#!/usr/bin/env python3
"""Black-box register-trace vectors for the libpress MDEC input/output wrapper rows 0x8010BFA0 and 0x8010C01C.

The ORIGINAL functions (movie-module overlay, Disc 1 LBA 1940 x38) are run as opaque boxes under the test-only
interpreter with load/store logging (pe_pad_leaf_vectors.run; registers are plain memory there, so reads return the
seeded value or the last written one).  For every case the header records, per call:
  * the seeds: DPCR (0x1F8010F0) and MDEC status (0x1F801824) before the call;
  * `terminated`: 1 when the call returned normally, 0 when it exhausted the instruction budget (a wait/poll that
    the seeded status never satisfies);
  * every hardware-register WRITE (address >= 0x1F800000) in write order, as (reg,size,value);
  * every hardware-register READ in order (the first MDEC_IO_MAXR; the total is kept in nr_total);
  * for 0x8010BFA0: the command word in guest RAM after the call (`edited`).
Nothing about how the original computes anything is recorded.  Output: pc_port/tests/retail_mdec_io_trace_cases.h
    PE_DISC1_BIN=... python3 -I pc_port/tools/pe_mdec_io_trace_vectors.py [--check]
"""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_pad_leaf_vectors as pl
import pe_mdec_vlc_vectors as V

MAXW = 8; MAXR = 12
IN_CMD = (0x30000020, 0x3E000040, 0x20000020, 0x28000040, 0x38000020, 0x00000000, 0xFFFFFFFF, 0x08000010, 0x02000000,
          0x04000001, 0x10000000, 0x30000000, 0x3A00FFFF)
IN_MODE = (0, 1, 2, 3, 4, 5, 0xFFFFFFFF, 0x80000000)
OUT_DEST = (0x80160000, 0x80160003, 0x80000000, 0x801FFFFC, 0x00160000, 0xA0160000, 0)
OUT_WORDS = (0, 1, 2, 31, 32, 33, 64, 96, 128, 159, 0x1FF, 0xFFE0, 0xFFFF, 0x10000, 0x1FFFE0, 0x200000, 0xFFFFFFE0, 0xFFFFFFFF)
DPCR = (0, 0x08, 0x10, 0x20, 0x40, 0x80, 0x78, 0x88, 0xF8, 0xF0, 0x70, 0x12345678, 0x333333BB, 0xFFFFFFFF)
STATUS = (0, 0x80040000, 0x40000000, 0x10000000, 0x08000000)
# MDEC status bit 29 (command busy) makes the original wait for it to clear; with registers as plain memory it never
# does, so those seeds are recorded as non-terminating (terminated=0) for a few representative calls only (4 s each).
BUSY = (0x20000000, 0xFFFFFFFF, 0x20000000 | 0x80040000)


def one(base, entry, args, dpcr, status, pre=None):
    r = bytearray(base)
    struct.pack_into('<I', r, 0x10F0, dpcr); struct.pack_into('<I', r, 0x1824, status)
    if pre: pre(r)
    try:
        ret, loads, stores = pl.run(r, entry, args)
        term = 1
    except AssertionError as e:
        assert 'instruction budget' in str(e), e
        ret, loads, stores = 0, [], []; term = 0
        return term, [], [], 0, r
    w = [s for s in stores if s[0] >= 0x1F800000]
    rd = [l for l in loads if l[0] >= 0x1F800000]
    return term, w, rd, ret, r


def fmt(acc, n):
    rows = [list(x) for x in acc[:n]] + [[0, 0, 0]] * (n - len(acc[:n]))
    return '{' + ','.join('{0x%08Xu,%du,0x%08Xu}' % tuple(x) for x in rows) + '}'


def main():
    ram, path = V.env(); base = bytearray(ram)
    rows_in = []; rows_out = []; maxw = 0; maxr = 0
    for cmd in IN_CMD:
        for mode in IN_MODE:
            for dpcr, status in [(d, 0) for d in DPCR] + [(0, s) for s in STATUS[1:]]:
                def pre(r, cmd=cmd): struct.pack_into('<I', r, 0x140000, cmd)
                term, w, rd, ret, r = one(base, 0x8010BFA0, [0x80140000, mode], dpcr, status, pre)
                edited = struct.unpack_from('<I', r, 0x140000)[0] if term else 0
                rows_in.append((cmd, mode, dpcr, status, term, edited, w, rd))
    for cmd, mode in ((0x30000020, 0), (0x30000020, 3), (0x28000040, 1), (0x00000000, 0xFFFFFFFF)):
        for status in BUSY:
            def pre(r, cmd=cmd): struct.pack_into('<I', r, 0x140000, cmd)
            term, w, rd, ret, r = one(base, 0x8010BFA0, [0x80140000, mode], 0, status, pre)
            rows_in.append((cmd, mode, 0, status, term, struct.unpack_from('<I', r, 0x140000)[0] if term else 0, w, rd))
    for dest, words in ((0x80160000, 32), (0x80160000, 0)):
        for status in BUSY:
            term, w, rd, ret, r = one(base, 0x8010C01C, [dest, words], 0, status)
            rows_out.append((dest, words, 0, status, term, w, rd))
    for dest in OUT_DEST:
        for words in OUT_WORDS:
            for dpcr, status in [(d, 0) for d in DPCR] + [(0, s) for s in STATUS[1:]]:
                term, w, rd, ret, r = one(base, 0x8010C01C, [dest, words], dpcr, status)
                rows_out.append((dest, words, dpcr, status, term, w, rd))
    for rows in (rows_in, rows_out):
        for row in rows:
            maxw = max(maxw, len(row[-2])); maxr = max(maxr, len(row[-1]))
    pool = []; index = {}
    def ref(acc):
        key = tuple(tuple(x) for x in acc)
        if key not in index:
            index[key] = len(pool); pool.extend(key)
        return index[key]
    in_refs = [(ref(r[6]), len(r[6]), ref(r[7][:MAXR]), len(r[7])) for r in rows_in]
    out_refs = [(ref(r[5]), len(r[5]), ref(r[6][:MAXR]), len(r[6])) for r in rows_out]
    L = ['/* Original libpress MDEC wrapper rows 0x8010BFA0 / 0x8010C01C: write-ordered hardware-register traces (black box). */',
         '/* Generated by pc_port/tools/pe_mdec_io_trace_vectors.py. Registers are (address,size,value); each case holds an offset+count',
         ' * into MDECIOT_pool for its writes (nw, program order) and its reads (nr = total reads; at most %d are kept, the first ones). */' % MAXR,
         'typedef struct { uint32_t reg, size, value; } MdecIoAccess;',
         '/* longest write trace %d, longest kept read trace %d, %d unique access runs */' % (maxw, maxr, len(pool))]
    L.append('static const MdecIoAccess MDECIOT_pool[] = {')
    L += ['{0x%08Xu,%du,0x%08Xu},' % tuple(x) for x in pool]
    L.append('};')
    L.append('static const struct { uint32_t command, mode, dpcr, status, terminated, edited, w_off, nw, r_off, nr; } MDECIOT_input[] = {')
    for row, (wo, nw, ro, nr) in zip(rows_in, in_refs):
        cmd, mode, dpcr, status, term, edited = row[:6]
        L.append('{0x%08Xu,0x%08Xu,0x%08Xu,0x%08Xu,%du,0x%08Xu,%du,%du,%du,%du},' % (cmd, mode, dpcr, status, term, edited, wo, nw, ro, nr))
    L.append('};')
    L.append('static const struct { uint32_t destination, words, dpcr, status, terminated, w_off, nw, r_off, nr; } MDECIOT_output[] = {')
    for row, (wo, nw, ro, nr) in zip(rows_out, out_refs):
        dest, words, dpcr, status, term = row[:5]
        L.append('{0x%08Xu,0x%08Xu,0x%08Xu,0x%08Xu,%du,%du,%du,%du,%du},' % (dest, words, dpcr, status, term, wo, nw, ro, nr))
    L.append('};')
    out = '\n'.join(L) + '\n'
    p = V.ROOT / 'pc_port/tests/retail_mdec_io_trace_cases.h'
    if '--check' in sys.argv: assert p.read_text() == out, 'trace header differs'
    else: p.write_text(out)
    nterm_in = sum(1 for r in rows_in if r[4]); nterm_out = sum(1 for r in rows_out if r[4])
    print('PASS %d input cases (%d terminated), %d output cases (%d terminated); longest write trace %d, read trace %d' %
          (len(rows_in), nterm_in, len(rows_out), nterm_out, maxw, maxr))


if __name__ == '__main__':
    main()
