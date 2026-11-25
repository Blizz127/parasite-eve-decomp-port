#!/usr/bin/env python3
"""Black-box evidence for the call contract of the original VLC decoder row 0x8010C89C as the production
movie code uses it.  The ORIGINAL movie updater (0x80122040) and player (0x80121C04) are run under the
test-only interpreter with the same explicit SDK/frame providers as pe_movie_update_oracle.py /
pe_movie_player_oracle.py (original control flow, nothing patched); every call to 0x8010C89C is intercepted
at its entry and replaced by a provider: the decoded output for the real frame (taken from the original
decoder run in isolation, pe_mdec_vlc_vectors.run_original), a chosen return value, and chosen returned a0/a1.

Facts established (printed; the word at 0x8011EBB4 is excluded from the state hash and varied separately):
  1. the arguments of every intercepted call (distinct (a0,a1,a2,a3) tuples; a0 == 0 = resume never occurs);
  2. whether the caller's final outcome (return value, call sequence, overlay/work state hash) depends on the
     returned v0 (variants 0,1,2,0xFFFFFFFF) or on the returned a0/a1 (variants: advanced, zero, garbage).
  PE_DISC1_BIN=... python3 -I pc_port/tools/pe_c89c_caller_contract.py"""
import hashlib, itertools, os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pe_mdec_vlc_vectors as V
from pe_movie_init_oracle import load, execute
from pe_cd_ack_oracle import Stops

RET_VARIANTS = (0, 1, 2, 0xFFFFFFFF)
REG_VARIANTS = ('advance+end', 'zero+unwritten', 'garbage+garbage')


def provider(r, regs, out, cursor, ret, regvar):
    struct.pack_into('<%dB' % len(out), r, regs[5] & 0x1FFFFF, *out)
    # word at 0x8011EBB4 (overlay data): the original leaves a1 + output length there (black-box observation);
    # variants: 'end' as observed, 'unwritten', 'garbage'
    mode = regvar.split('+')[1] if '+' in regvar else 'unwritten'
    regvar = regvar.split('+')[0]
    if mode == 'end': struct.pack_into('<I', r, 0x11EBB4, (regs[5] + len(out)) & 0xFFFFFFFF)
    elif mode == 'garbage': struct.pack_into('<I', r, 0x11EBB4, 0xA5A5A5A5)
    regs[2] = ret
    if regvar == 'advance': regs[4] += cursor; regs[5] += len(out)
    elif regvar == 'zero': regs[4] = 0; regs[5] = 0
    else: regs[4] = 0xDEADBEEF; regs[5] = 0x12345678


def digest(r):
    return hashlib.sha256(bytes(r[0x120D00:0x122D00]) + bytes(r[0xB0D00:0xB0F00]) + bytes(r[0x11E000:0x11EBB4]) + bytes(r[0x11EBB8:0x11F000])).hexdigest()[:12]


def updater(base, ovl, data, bank, end, retry, timeout, active, ret, regvar, ref):
    calls_at = (0x80121004, 0x8010BFA0, 0x8010C01C, 0x80121270, 0x8010C89C, 0x8007C394, 0x8007C2A0, 0x8007F72C,
                0x8007F778, 0x80080D5C, 0x80081314, 0x8010C0D8, 0x8007A2A4, 0x80080DC4, 0x800870F0, 0x80122258)
    r = bytearray(base); r[0x120D00:0x122D00] = ovl
    def sw(a, v): struct.pack_into('<I', r, a, v)
    def sh(a, v): struct.pack_into('<H', r, a, v)
    r[0xB0DBA] = active; r[0xB0DBB] = 1; r[0x1223F5] = end; r[0x1223F6] = 3; r[0x1223F8] = 0
    sw(0x1223FC, 0x11223344); sw(0x9CDDC, 1); sh(0xB0DBC, 65535)
    sw(0x122430, 0x80130000); sw(0x1228CC, 0x80160000); sw(0x1228D0, 0x80164000)
    r[0x1228D4] = bank; r[0x1228E0] = bank; sw(0x1228D8, 0x80170000); sw(0x1228DC, 0x80174000)
    sh(0x1228F8, 24); sh(0x1228FA, 240); r[0x1228F2] = 0; r[0x1228FC] = 0 if timeout else 1
    sh(0x1228EA, 17); sh(0x1228EC, 240)
    r[0x150000:0x150000 + len(data)] = data
    stops = Stops(calls_at); pc = 0x80122040; initial = None; log = []; polls = 0
    while True:
        stops.last = 0; regs = execute(r, pc, initial_regs=initial, stop_at=stops, instruction_budget=3000000)
        fn = stops.last
        if not fn: break
        if fn == 0x80122258:
            sw((regs[29] + 16) & 0x1FFFFF, 1); pc = fn + 4; regs[2] = 1; initial = dict(enumerate(regs)); continue
        log.append((fn, tuple(regs[4:8]))); regs[2] = 0
        if fn == 0x80121270: polls += 1; regs[2] = 0 if retry and polls <= 2000 else 0x80150000
        elif fn in (0x8007F72C, 0x80081314): regs[2] = 1
        elif fn == 0x8010C89C: provider(r, regs, ref[0], ref[1], ret, regvar)
        pc = regs[31]; initial = dict(enumerate(regs))
    return log, regs[2], digest(r)


def player(base, ovl, data, vid, wide, failfirst, ret, regvar, ref):
    calls_at = (0x80121004, 0x800719F4, 0x80081414, 0x8007F72C, 0x8007F778, 0x80080D5C, 0x80081314, 0x8010BE3C,
                0x8010C0D8, 0x8007A214, 0x8007C304, 0x800870F0, 0x8010BD4C, 0x80121270, 0x8010C89C, 0x8007C394)
    r = bytearray(base); r[0x120D00:0x122D00] = ovl
    def sw(a, v): struct.pack_into('<I', r, a, v)
    def sh(a, v): struct.pack_into('<H', r, a, v)
    def strcat(dst, src):
        end = dst
        while r[end]: end += 1
        while r[src]: r[end] = r[src]; end += 1; src += 1
        r[end] = 0
    sw(0x120FF4, 0x80130060); sw(0x120FFC, 0x80130060)
    r[0x130060:0x130068] = b'\\MV\\' + bytes(2)
    record = 0x122438 + vid * 20
    r[0x130070:0x13007C] = b'\\A.STR;1' + bytes(1)
    sw(record, 0x80130070); r[record + 4] = wide
    sh(record + 6, 0x1234); sh(record + 0xA, 0x5678); sh(record + 0xC, 0x300)
    sw(0x122420, 0x80160000); sw(0x122424, 0x80164000); sw(0x122428, 0x80170000); sw(0x12242C, 0x80174000)
    sw(0x122430, 0x80130000); sw(0x122434, 0x80190000)
    r[0xB0DBA] = 3; sh(0xB0DBC, 0); sw(0x9CDDC, 0x100); r[0xB0DBE] = 0x98
    r[0x150000:0x150000 + len(data)] = data
    stops = Stops(calls_at); pc = 0x80121C04; initial = {4: vid & 0xFFFFFFFF}; log = []; searched = 0
    while True:
        stops.last = 0; regs = execute(r, pc, initial_regs=initial, stop_at=stops, instruction_budget=3000000)
        fn = stops.last
        if not fn: break
        args = tuple(regs[4:8])
        if fn == 0x800719F4: strcat(regs[4] & 0x1FFFFF, regs[5] & 0x1FFFFF)
        elif fn == 0x80081414:
            searched += 1; sw(regs[4] & 0x1FFFFF, 0x4321); regs[2] = 0 if (failfirst and searched == 1) else 0x1234
        elif fn == 0x8007F72C: regs[2] = 1
        elif fn == 0x8007F778: regs[2] = 0
        elif fn in (0x80080D5C, 0x80081314): regs[2] = 1
        elif fn == 0x80121270: regs[2] = 0x80150000
        elif fn == 0x8010C89C: provider(r, regs, ref[0], ref[1], ret, regvar)
        else: regs[2] = 0
        log.append((fn, args))
        pc = regs[31]; initial = dict(enumerate(regs))
    return log, regs[2], digest(r)


def main():
    ex, ovl = load(); ram, path = V.env()
    base = bytearray(ram)
    frames = V.frames_on_disc(path)
    data = V.read_frame(path, frames[3])
    ref = V.run_original(ram, data, 3000000)[:2]   # (output, cursor)
    seen = set(); calls = 0; differs = 0; total = 0
    for active, end, bank, retry, timeout in itertools.product((2, 255), (0, 1, 2), (0, 1), (0, 1), (0, 1)):
        results = []
        for ret, regvar in [(v, 'advance+end') for v in RET_VARIANTS] + [(0, v) for v in REG_VARIANTS[1:]]:
            results.append(updater(base, ovl, data, bank, end, retry, timeout, active, ret, regvar, ref))
        total += 1
        if not all(x[1:] == results[0][1:] for x in results): differs += 1
        for fn, a in results[0][0]:
            if fn == 0x8010C89C: calls += 1; seen.add(a)
    print('UPDATER scenarios (active,end,bank,retry,timeout): %d; outcome (return, state hash) differing across C89C return/a0/a1 variants: %d' % (total, differs))
    print('UPDATER C89C calls: %d, distinct (a0,a1,a2,a3): %s' % (calls, sorted(tuple(hex(x) for x in s) for s in seen)))
    seen = set(); calls = 0; differs = 0; total = 0
    for vid, wide, failfirst in itertools.product((0, 0x14, 0x15, 0x2E), (0, 1), (0, 1)):
        results = []
        for ret, regvar in [(v, 'advance+end') for v in RET_VARIANTS] + [(0, v) for v in REG_VARIANTS[1:]]:
            results.append(player(base, ovl, data, vid, wide, failfirst, ret, regvar, ref))
        total += 1
        if not all(x[1:] == results[0][1:] for x in results): differs += 1
        for fn, a in results[0][0]:
            if fn == 0x8010C89C: calls += 1; seen.add(a)
    print('PLAYER scenarios (vid,wide,failfirst): %d; outcome differing across C89C return/a0/a1 variants: %d' % (total, differs))
    print('PLAYER C89C calls: %d, distinct (a0,a1,a2,a3): %s' % (calls, sorted(tuple(hex(x) for x in s) for s in seen)))


if __name__ == '__main__':
    main()
