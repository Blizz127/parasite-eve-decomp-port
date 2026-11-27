"""Per-batch entry specs for pe_cdx_vectors.py (batches cdgen42..cdgen45).
Argument kinds, stopped callees (entry: argument count) and hooks only; the
function behaviour is whatever the original does under the interpreter.

Callee rule (derived from `nm -u` NAMES of the current provider object and the
public pe_plat headers, never from provider source): a retail callee is
stopped and recorded when the current provider reaches it through a bound
host symbol; any other callee runs as original code inline, so its stores and
register accesses are part of the entry's observable behaviour."""


def _cdirq_hook(m):
    """CD interrupt-flag model after psx-spx (CDROM controller): with index 1 selected,
    0x1F801803 reads the pending interrupt type in bits 0-2 (bits 5-7 set) and a write of
    1 bits acknowledges them; a seeded queue of further responses surfaces one at a time."""
    c, gv = m.c, m.gv

    def hook(ram, x, args, rets, seed):
        x = gv.xs32(x)
        c.CdIrq.active = True
        c.CdIrq.flags = (0, 1, 2, 3, 4, 5, x >> 8)[x % 7] & 0xFF
        x = gv.xs32(x)
        c.CdIrq.queue = [1 + (x >> (3 * i)) % 5 for i in range(x % 4)]
        c.CdIrq.noise = ((x >> 12) & 0x18) if seed % 2 else 0
        x = gv.xs32(x)
        ram[0x1800] = x & 0xFF
        c.CdIrq.refresh(ram)
        return x, None
    return hook


def load_specs(m, batch):
    s = m.spec
    if batch == 'cdgen42':
        b = 'cdgen42'
        s(0x8007FA2C, 'cd_initialize_state', '', {0x80080B44: 2}, batch=b, void=True)
        s(0x8007A4D0, 'cd_control', 'bpp', {0x8007B558: 4}, batch=b)
        s(0x8007A60C, 'cd_control_async', 'bp', {0x8007B558: 4}, batch=b)
        s(0x8007A740, 'cd_control_wait', 'bpp', {0x8007B010: 2, 0x8007B558: 4}, batch=b)
        s(0x8007A3CC, 'cd_reset_latches', '', {0x8007B9EC: 0}, batch=b, void=True)
        s(0x8007A8EC, 'cd_set_dma3_callback', 'w', {0x80073CF4: 2}, batch=b, void=True)
        s(0x80080404, 'cd_command_progress', 'bw', {}, batch=b, void=True, n=1500)
        s(0x8008080C, 'cd_decode_status', 'ww', {}, batch=b, void=True, inline=(0x80080998,), n=600)
        s(0x8007CE80, 'stream_copy_words', 'ppn', {}, batch=b, void=True)
        s(0x8007FE24, 'cd_status_pump', '', {0x8007FCFC: 2, 0x800800F4: 0}, batch=b, void=True)
        s(0x8007A2A4, 'cd_shutdown_stream', '', {0x80072714: 0, 0x80072724: 0, 0x8007A4BC: 1, 0x80073CF4: 2,
                                                  0x800824C8: 1, 0x800824F0: 1}, batch=b, void=True, inline=(0x8007A8EC,))
        s(0x8007BBFC, 'cd_initialize_device', '', {0x80073C94: 0, 0x80073CC4: 2, 0x8007B010: 2, 0x8007B558: 4},
          batch=b, inline=(0x80071A74, 0x80073C5C), bios=True,
          bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)))
        m.HOOKS[0x8007BBFC] = _cdirq_hook(m)
    if batch == 'cdgen42b':
        _load_cdgen42b(m)
    if batch == 'cdgen44':
        b = 'cdgen44'
        # SPU DMA event installer / SPU init chain (libsnd): BIOS B(08h) OpenEvent and B(0Ch) EnableEvent are
        # the pseudo callees B0000008 / B000000C (the provider reaches them through host event services)
        evt = {0x800726E4: 4, 0x80072704: 1, 0x80072714: 0, 0x80072724: 0, 0x8007DD14: 1}
        s(0x8007D15C, 'spu_install_dma_event', '', dict(evt), batch=b, void=True)
        init = dict(evt)
        init.update({0x80073C94: 0, 0x8007D1D4: 1, 0x8007DAE0: 3})
        s(0x8007D074, 'spu_init_wrapper', 'w', dict(init), batch=b, void=True, inline=(0x8007D15C,))
        s(0x8007D054, 'spu_init_default', '', dict(init), batch=b, void=True,
          inline=(0x8007D074, 0x8007D15C))
        # SPU reverb mode transition and the streaming command dispatcher
        s(0x8008CB54, 'spu_reverb_mode_transition', 'w', {0x80085A64: 1, 0x8008CF70: 1}, batch=b, void=True,
          inline=(0x8008D7C0,))
        s(0x8008CBA8, 'stream_command_dispatch', '', {0x80085084: 1, 0x80085A64: 1, 0x8008CF70: 1}, batch=b,
          inline=(0x8008CB08, 0x8008CB54, 0x8008D7C0), n=300)
        gv = m.gv

        def guard_hook(ram, x, args, rets, seed):
            # the DMA event installer is guarded by a one-shot flag word: clear it in most cases so the body runs
            x = gv.xs32(x)
            if x % 5 < 3:
                ram[0x9B3EC:0x9B3F0] = bytes(4)
            return x, None
        for e in (0x8007D15C, 0x8007D074, 0x8007D054):
            m.HOOKS[e] = guard_hook
        # Register-level vectors only (no smoke against the current binding: it drives host PE_SpuDma_* /
        # pe_plat_spu_* models, not registers; see REPORT).  Everything runs inline as original code.
        s(0x80085E54, 'spu_write', 'pw', {}, batch=b, regonly=True, inline=(0x8007D9F8,), bios=True, bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)))
        s(0x80085DF4, 'spu_read', 'pw', {}, batch=b, regonly=True, inline=(0x8007DA7C,), bios=True, bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)))
        s(0x80085644, 'stream_bringup', '', {0x80072714: 0, 0x80072724: 0}, batch=b, void=True, regonly=True, n=64, bios=True,
          unsupported='superseded by batch cdgen46 (the interpreter gained DIV/DIVU/BREAK, see pe_battle_hud_oracle); kept out of cdgen44 so its vector hashes do not change', bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)),
          inline=(0x8007D15C, 0x80085A04, 0x80085F14, 0x80085EB4, 0x800850F4, 0x80085174, 0x80085290, 0x80085C44,
                  0x80085D84, 0x80085814, 0x800858E8, 0x800726E4, 0x80072704))
    if batch == 'cdgen46':
        b = 'cdgen46'
        # 0x80085644: stream (SPU) bring-up; inline libspu code runs DIV/DIVU (interpreter support: pe_battle_hud_oracle).
        # Stopped = bound host symbol in the current pe_stream.o's nm -u (header bindings: pe_plat/state.h, ctl.h, spu_a.h,
        # audio.h); everything else (SpuSetTransferMode 85F14 .. SpuSetIRQCallback 85D84, heap configure 85A04, ...) runs
        # inline as original code, so its register accesses are part of the entry's observable behaviour.
        s(0x80085644, 'stream_bringup', '',
          {0x8007D15C: 0, 0x800850F4: 2, 0x80085814: 3, 0x800858E8: 1, 0x800726E4: 4, 0x80072704: 1,
           0x80072714: 0, 0x80072724: 0, 0x80085A64: 1, 0x8008CF70: 1, 0x80085BB4: 1, 0x80085F74: 1,
           0x800862F4: 5, 0x80085084: 1},
          batch=b, void=True, n=64, bios=True, mips_div=True, bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)),
          inline=(0x80085A04, 0x80085F14, 0x80085EB4, 0x80085174, 0x80085290, 0x80085C44, 0x80085D84))
        gv46 = m.gv

        def retry_hook(ram, x, args, rets, seed):
            # The bring-up retries each of four calls until it succeeds: the counter setup 85814 and start 858E8
            # until nonzero, BIOS OpenEvent 726E4 while it returns -1 (the handle is stored at 0x800A CDE0), EnableEvent
            # 72704 until nonzero.  Per-call return sequences: k failures (k = 0..3, edge cases up to 5) then a success;
            # the checker replays them per call (RETS token fn=a/b/c, last value repeats).
            models = {}
            x = gv46.xs32(x)
            for i, (fn, fail) in enumerate(((0x80085814, 0), (0x800858E8, 0), (0x800726E4, 0xFFFFFFFF), (0x80072704, 0))):
                x = gv46.xs32(x)
                k = (x >> 8) % 4 if seed % 7 else 4 + (x >> 12) % 2
                if getattr(m, 'EDGE46', False):
                    # -edge set: retail only tests zero / -1, so any other word is a success; a native that tests
                    # `== 1` / `> 0` diverges here (documented in the REPORT, not a modelling claim)
                    good = (2, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xF2000002, gv46.xs32(x) | 1)[(x >> 16) % 6]
                    if fail == 0xFFFFFFFF and good == 0xFFFFFFFF:
                        good = 0xFFFFFFFE
                else:
                    # main set: the documented success values (psx-spx): SetRCnt / StartRCnt / EnableEvent return 1,
                    # OpenEvent returns a handle 0xF1000000 + n
                    good = 0xF1000000 + (x >> 16) % 8 if fail == 0xFFFFFFFF else 1
                seq = [fail] * k + [good]
                rets[fn] = list(seq)
                st = [0]

                def model(ram_, r, seq=seq, st=st):
                    v_ = seq[min(st[0], len(seq) - 1)]
                    st[0] += 1
                    return v_
                models[fn] = model
            return x, models
        m.HOOKS[0x80085644] = retry_hook
    if batch == 'cdgen45':
        b = 'cdgen45'
        # libpad bring-up wrappers bound in platform/pe_save.c (audit conflict: the file header calls them game
        # logic, the project classification TSV calls them libpad PadStartCom / PadInitDirect).
        s(0x80082534, 'pad_initialize_host', '', {0x80072714: 0, 0x80072724: 0, 0x80073C84: 2}, batch=b, void=True,
          inline=(0x80082CF0, 0x8007E1E4, 0x8007E1F4), bios=True,
          bios_silent=((0xA0, 0x3F), (0xB0, 0x3F), (0xC0, 2), (0xC0, 3)))
        s(0x800844E4, 'pad_records_initialize_host', 'ww', {}, batch=b, void=True,
          inline=(0x80084B44, 0x80071A24, 0x80082ADC), bios=True, bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)))
        gv45 = m.gv

        def nohooks(ram, x, args, rets, seed):
            # libcard hook slots 0x8009B724..0x8009B74C hold no callbacks at bring-up (a random word would be
            # called as guest code; the provider has no slot dispatcher to compare against)
            ram[0x9B724:0x9B750] = bytes(0x2C)
            return x, None
        m.HOOKS[0x80082534] = nohooks
        m.HOOKS[0x800844E4] = nohooks


# ------------------------------------------------------------------------------------------------------------------
# cdgen42b: the CD-stream pump 0x8007FE24, the command-progress handler 0x80080404 and the status decoder 0x8008080C
# sampled in the states the running game reaches (fresh-author request, sw-vec-cd2 2026-11-27).
#   * the discrete seed dimensions the author asked for, as a full product (each combination sampled);
#   * RUNTIME-OBSERVED input tuples (CDXT lines of the scratch trace hook, env CDX42B_TRACE = comma-separated logs);
#   * random fill (generic discovery seeding + random callback slots) up to >= 400 cases per function.
# The callback slots 0x800A36A0..36AC hold real guest functions at runtime (0x8007F7E8, 0x8007E964, 0x8007F88C,
# 0x8007F960 in the boot trace); they are stopped callees here, plus three sentinels, so every `jalr` through a slot
# is a CALL line.  Nothing here reads a provider.
CB_SENTINELS = (0x80131300, 0x80131340, 0x80131380)
CB_RUNTIME = (0x8007F7E8, 0x8007E964, 0x8007F88C, 0x8007F960)


def _load_cdgen42b(m):
    import os
    import re
    import struct
    c, gv = m.c, m.gv
    s = m.spec
    b = 'cdgen42b'
    cb = {a: 2 for a in CB_SENTINELS + CB_RUNTIME}
    cb0 = {a: 0 for a in CB_SENTINELS + CB_RUNTIME}

    def put32(ram, a, v):
        struct.pack_into('<I', ram, a & 0x1FFFFF, v & 0xFFFFFFFF)

    def put8(ram, a, v):
        ram[a & 0x1FFFFF] = v & 0xFF

    def hsh(*k):
        h = 0x9E3779B1
        for q in k:
            h = gv.xs32((h ^ (q & 0xFFFFFFFF)) * 2654435761 & 0xFFFFFFFF or 1)
        return h

    # ---- runtime tuples ------------------------------------------------------------------------------------------
    RT = {'pump': [], 'progress': [], 'decode': []}
    for path in [q for q in os.environ.get('CDX42B_TRACE', '').split(',') if q]:
        for ln in open(path, errors='replace'):
            mm = re.match(r'CDXT (pump|progress|decode) ', ln)
            if not mm:
                continue
            kv = {k: int(v, 16) for k, v in re.findall(r'(\w+)=([0-9A-Fa-f]+)', ln) if k != 'call'}
            if kv not in RT[mm.group(1)]:
                RT[mm.group(1)].append(kv)
    REP = 8      # each runtime tuple becomes REP cases (different seeds: callee returns and every word the tuple does not pin)
    for key in RT:
        RT[key] = [t for t in RT[key] for _ in range(REP)]
    NRAND = 400
    COV = {}

    def cov(fn, key):
        COV.setdefault(fn, {})
        COV[fn][key] = COV[fn].get(key, 0) + 1
    m.COV = COV
    m.RT = RT

    def cb_pick(h, nonzero, allowed=None):
        pool = allowed or (CB_RUNTIME + CB_SENTINELS)
        return pool[h % len(pool)] if nonzero else 0

    # ---- pump 0x8007FE24 -----------------------------------------------------------------------------------------
    PH, ST = (0, 1, 2, 3), tuple(range(0x15))
    CD3 = (0, 1, 2)
    SUBS = (0x15, 0x16, 0x17, 0x18, 0, 0x14, 0x19)
    PUMP_CELLS = len(PH) * len(ST) * 3 * 2 * 2          # phase x state x gate x slot(0/nz) x byte 0x8009B58B
    NPUMP = PUMP_CELLS * 27
    PUMP_SLOT_NZ = (0x80080164, 0x80080404, 0x8008080C) + CB_SENTINELS[:1] + CB_RUNTIME[:1]

    def pump_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['pump'])
        if m.PILOT[0] or k >= nrt + NPUMP:
            h = hsh(seed, 77)
            put32(ram, 0x8009B574, (0, 1, 2, 2, 3, h >> 8)[h % 6])
            put32(ram, 0x8009B578, (list(ST) + [0x15, 0x20, h >> 12])[(h >> 4) % 24])
            put32(ram, 0x800A36A0, cb_pick(h >> 3, (h >> 20) & 1, PUMP_SLOT_NZ))
            if k >= nrt + NPUMP:
                cov('pump', 'random')
            return x, None
        if k < nrt:
            t = RT['pump'][k]
            put32(ram, 0x8009B574, t['574']); put32(ram, 0x8009B578, t['578']); put32(ram, 0x8009B57C, t['57C'])
            put32(ram, 0x8009B554, t['554']); put32(ram, 0x8009B598, t['598']); put32(ram, 0x8009B594, t['594'])
            put32(ram, 0x8009B5A0, t['5A0']); put8(ram, 0x8009B588, t['b588']); put8(ram, 0x8009B58A, t['b58A'])
            put8(ram, 0x8009B58B, t['b58B']); put32(ram, 0x8009B6A4, t['6A4'])
            slot = t['36A0']
            put32(ram, 0x800A36A0, slot if slot == 0 or slot in cb else CB_SENTINELS[0])
            cov('pump', 'runtime')
            return x, None
        p = k - nrt
        cell, j = divmod(p, 27)
        b58b, slotc, gate = cell % 2, (cell // 2) % 2, (cell // 4) % 3
        st, ph = (cell // 12) % 21, cell // 252
        h = hsh(cell, j)
        put32(ram, 0x8009B574, PH[ph]); put32(ram, 0x8009B578, ST[st]); put32(ram, 0x8009B554, gate)
        put8(ram, 0x8009B58B, b58b)
        put32(ram, 0x8009B598, CD3[j // 9]); put32(ram, 0x8009B594, CD3[(j // 3) % 3]); put32(ram, 0x8009B5A0, CD3[j % 3])
        put32(ram, 0x8009B57C, SUBS[(j + cell) % len(SUBS)])         # j covers 0..26: every substate in every cell
        put32(ram, 0x8009B6A4, ((j + cell) >> 1) & 1)
        put8(ram, 0x8009B588, ((j + cell) // 3) & 1)
        put8(ram, 0x8009B58A, ((j + cell) // 5) & 1)
        put32(ram, 0x8009B58C, (0, 1, 0x7FFFFFFF, 0xFFFFFFFF)[((j + cell) // 7) % 4])     # incremented word of state 0xF / substate 0x16
        put32(ram, 0x800A36A0, PUMP_SLOT_NZ[(j + cell) % len(PUMP_SLOT_NZ)] if slotc else 0)
        cov('pump', 'product')
        return x, None

    s(0x8007FE24, 'cd_status_pump', '', {**{0x8007FCFC: 2, 0x800800F4: 0, 0x80080404: 0, 0x8008080C: 0, 0x80080164: 0}, **cb0},
      batch=b, void=True, n=len(RT['pump']) + NPUMP + NRAND)
    m.HOOKS[0x8007FE24] = pump_hook

    # ---- progress 0x80080404 -------------------------------------------------------------------------------------
    PSTATUS, PSTATE = (1, 2, 3, 5), tuple(range(0xB, 0x12))
    FLAGS = (0x00, 0x10, 0x02, 0x12)
    CNT = (0, 299, 300, 301, 400, 0x7FFFFFFF, 0xFFFFFFFF)
    NPROG_PRODUCT = len(PSTATUS) * len(PSTATE) * len(FLAGS) * 2 * 8
    NIDLE = len(FLAGS) * len(SUBS) * 3 * 8              # state 0xE, phase 2, status 2, cmd 1; flags x substate x countdown x slots
    RESP = (0, 1, 0x80100000, 0x80100040, 0x801FEC10, 0x00100000, 0x1F800000, 0xFFFFFFFF)

    def prog_common(ram, cell, v, h):
        put32(ram, 0x8009B57C, SUBS[(v + cell) % len(SUBS)])       # v covers 0..7: every substate / counter value per cell
        put32(ram, 0x8009B5A0, (v + cell // 7) % 3)
        put32(ram, 0x8009B58C, CNT[(v * 3 + cell) % len(CNT)])
        put32(ram, 0x8009B554, (v // 2 + cell) % 3)
        put32(ram, 0x8009B574, (v + cell // 3) % 4)
        for bit, a in enumerate((0x800A36A4, 0x800A36A8, 0x800A36AC)):      # all eight zero / nonzero patterns per cell
            put32(ram, a, cb_pick(h >> (21 + 3 * bit), (v >> bit) & 1))

    def prog_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['progress'])
        if m.PILOT[0] or k >= nrt + NPROG_PRODUCT + NIDLE:
            h = hsh(seed, 99)
            args[0] = (0, 1, 2, 3, 4, 5, 0xFF, h >> 8)[h % 8] & 0xFF
            put32(ram, 0x8009B578, (list(PSTATE) + [0, 0xA, 0x12, 0x13, 0x20, h >> 12])[(h >> 4) % 13])
            put8(ram, 0x8009B56C, (0, 0x10, 0x02, 0x12, 0xFF, h >> 20)[(h >> 9) % 6])
            put32(ram, 0x8009B590, (0, 1, 2, h >> 5)[(h >> 14) % 4])
            for bit, a in enumerate((0x800A36A4, 0x800A36A8, 0x800A36AC)):
                put32(ram, a, cb_pick(h >> (3 + 4 * bit), (h >> (15 + bit)) & 1))
            if k >= nrt + NPROG_PRODUCT + NIDLE:
                cov('progress', 'random')
            return x, None
        if k < nrt:
            t = RT['progress'][k]
            args[0] = t['status'] & 0xFF; args[1] = t['resp']
            put32(ram, 0x8009B574, t['574']); put32(ram, 0x8009B578, t['578']); put32(ram, 0x8009B57C, t['57C'])
            put8(ram, 0x8009B56C, t['b56C']); put32(ram, 0x8009B590, t['590']); put32(ram, 0x8009B554, t['554'])
            put32(ram, 0x8009B5A0, t['5A0']); put32(ram, 0x8009B58C, t['58C'])
            for a, key in ((0x800A36A4, '36A4'), (0x800A36A8, '36A8'), (0x800A36AC, '36AC')):
                v_ = t[key]
                put32(ram, a, v_ if v_ == 0 or v_ in cb else CB_SENTINELS[0])
            cov('progress', 'runtime')
            return x, None
        p = k - nrt
        if p < NPROG_PRODUCT:
            cell, v = divmod(p, 8)
            hi = hsh(cell, v)
            flag, b590 = cell % 4, (cell // 4) % 2
            state = (cell // 8) % 7
            status = cell // 56
            args[0] = PSTATUS[status]
            args[1] = RESP[(hi >> 25) % len(RESP)]
            put32(ram, 0x8009B578, PSTATE[state]); put8(ram, 0x8009B56C, FLAGS[flag]); put32(ram, 0x8009B590, b590)
            prog_common(ram, cell, v, hi)
            cov('progress', 'product')
            return x, None
        p -= NPROG_PRODUCT          # idle path: state 0xE, phase 2, status 2, command byte 1
        cell, v = divmod(p, 8)
        hi = hsh(cell, v, 5)
        args[0] = 2
        args[1] = RESP[(hi >> 25) % len(RESP)]
        put32(ram, 0x8009B578, 0xE); put8(ram, 0x8009B558, 1)
        put8(ram, 0x8009B56C, FLAGS[cell % 4]); put32(ram, 0x8009B590, hi >> 30 & 1)
        prog_common(ram, cell, v, hi)
        put32(ram, 0x8009B57C, SUBS[(cell // 4) % len(SUBS)]); put32(ram, 0x8009B5A0, (cell // 28) % 3)
        put32(ram, 0x8009B574, 2)
        cov('progress', 'idle')
        return x, None

    s(0x80080404, 'cd_command_progress', 'bw', dict(cb), batch=b, void=True,
      n=len(RT['progress']) + NPROG_PRODUCT + NIDLE + NRAND)
    m.HOOKS[0x80080404] = prog_hook

    # ---- decode 0x8008080C ---------------------------------------------------------------------------------------
    DST = (5, 1, 2, 3, 0, 0xFF)
    DCMD = (0, 1, 2, 3, 9, 0xE, 0x15, 0x1B)
    DTAB = (0, 1, 2, 3, 4, 8, 0x10, 0x80000000)
    DPTR = (0x80100000, 0x801FE400, 0x801FEC00, 0x00100100, 0xA0100200, 0x801FED80)    # zone, own-window (arena / above / top), KUSEG, KSEG1
    DBYTE = (0x00, 0x80, 0x40, 0x20, 0x02, 0xE2, 0xFF, None)
    NDEC = len(DST) * len(DCMD) * len(DTAB) * len(DPTR)

    def dec_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['decode'])
        if m.PILOT[0] or k >= nrt + NDEC:
            if k >= nrt + NDEC:
                cov('decode', 'random')
            return x, None
        if k < nrt:
            t = RT['decode'][k]
            args[0] = t['status']; args[1] = t['resp']
            put8(ram, 0x8009B558, t['cmd']); put32(ram, 0x8009B624 + 4 * (t['cmd'] & 0xFF), t['tab'])
            hi = hsh(k, 11)
            rb = t['resp'] & 0x1FFFFF
            for q in range(16):               # the response buffer is not part of the trace tuple: seed its bytes (status byte patterns cycle)
                ram[(rb + q) & 0x1FFFFF] = (hi >> (2 * q)) & 0xFF
            bt = DBYTE[k % len(DBYTE)]
            if bt is not None:
                ram[(rb + (t['tab'] - 1 if t['status'] != 5 and 0 < t['tab'] < 0x100 else 0)) & 0x1FFFFF] = bt
            cov('decode', 'runtime')
            return x, None
        p = k - nrt
        hi = hsh(p, 3)
        pi = p % len(DPTR); p //= len(DPTR)
        ti = p % len(DTAB); p //= len(DTAB)
        ci = p % len(DCMD); si = p // len(DCMD)
        args[0] = (0x1234500 + DST[si]) & 0xFFFFFFFF if si % 2 else DST[si]
        args[1] = DPTR[pi]
        put8(ram, 0x8009B558, DCMD[ci]); put32(ram, 0x8009B624 + 4 * DCMD[ci], DTAB[ti])
        bt = DBYTE[hi % len(DBYTE)]
        idx = DTAB[ti] - 1 if DST[si] != 5 and 0 < DTAB[ti] < 0x100 else 0
        base_ = DPTR[pi] & 0x1FFFFF
        for q in range(16):                   # response bytes resp[0..15] (the 8-byte copy and the indexed status byte)
            ram[(base_ + q) & 0x1FFFFF] = (hi >> (2 * q)) & 0xFF if q % 2 else (hi >> (3 * q + 1)) & 0xFF
        if bt is not None:
            ram[(base_ + idx) & 0x1FFFFF] = bt
        cov('decode', 'product')
        return x, None

    s(0x8008080C, 'cd_decode_status', 'ww', {}, batch=b, void=True, inline=(0x80080998,),
      n=len(RT['decode']) + NDEC + NRAND)
    m.HOOKS[0x8008080C] = dec_hook
