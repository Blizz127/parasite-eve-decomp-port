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
    if batch == 'cdgen48':
        _load_cdgen48(m)
    if batch in ('cdgen42c', 'cdgen42ci'):
        _load_cdgen42c(m, batch == 'cdgen42ci')
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
    if batch == 'cdgen47':
        b = 'cdgen47'
        # CD host providers with no clean-room derivation record (pe_plat_cd_{ack,status_poll,command_issue,sector_dma,
        # stream_handler,dma_issue}_host).  Stopped callees = retail entries the current provider objects reach through a
        # bound host symbol (`nm -u` NAMES + public pe_plat headers; never provider source): 80071A74 PE_Printf_Guest,
        # 80073C5C PE_GuestCall(.., 0x80073c5c, 1), 80073A44 VSync (PE_GPU_VSyncQuery / HostFB_VSync), 80073DE8
        # pe_plat_ctl_callback_state, 8007AAB4 pe_plat_cd_acknowledge_host, 8007B010 pe_plat_cd_status_poll_host, 8007B9EC
        # (the timeout recovery: PE_Cd_TimeoutRecover is binding-dependent, see REPORT), 8007A488 pe_plat_cd_ready, 8007C214
        # pe_plat_cd_data_ready_callback, 8007CEAC pe_plat_cd_dma_issue_host.  The CD data callbacks (jalr through the
        # words 0x8009AFB4 / 0x8009AFB8, and the data-ready word 0x800B0CCC) are stopped at the sentinel addresses below
        # and at the real callback targets seen at runtime (pe_cdx_cdhost_vectors.py adds them from the capture).
        cb = {0x80131140: 2, 0x80131180: 2}
        _s0 = s

        def s(entry, name, kinds, callees=None, **kw):
            _s0(entry, name, kinds, callees, printf_fmt=True, **kw)   # printf records its format's arguments only (pe_cdx_vectors.run_case)
        s(0x8007AAB4, 'cd_acknowledge', '', {0x80071A74: 4, 0x80073C5C: 1}, batch=b, n=400)
        poll = {0x8007AAB4: 0, 0x80073A44: 1, 0x80071A74: 4, 0x80073C5C: 1, 0x80073DE8: 0, 0x8007B9EC: 0}
        poll.update(cb)
        s(0x8007B010, 'cd_status_poll', 'wp', dict(poll), batch=b, n=400)
        issue = dict(poll)
        issue[0x8007B010] = 2
        s(0x8007B558, 'cd_command_issue', 'bppw', issue, batch=b, n=400)
        s(0x8007BF44, 'cd_sector_dma', 'pw', {}, batch=b, n=300)
        s(0x8007C564, 'cd_stream_handler', '', {0x8007A488: 2, 0x8007C214: 0, 0x8007CEAC: 7, 0x801311C0: 0}, batch=b,
          void=True, inline=(0x8007C444, 0x8007CE80), n=800)
        s(0x8007CEAC, 'cd_dma_issue', 'cwwwwww', {0x80071A74: 4}, batch=b, void=True, n=600)
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


# ------------------------------------------------------------------------------------------------------------------
# cdgen42c / cdgen42ci: the three callback-slot targets that have no native (sw-vec-cd2, 2026-11-27):
#   0x8007F7E8  libds VBlank hook (pump slot 0x800A36A0; no arguments): lane getter 8007FBF0(0) twice, queue record
#               at 0x800A3540 + 24 * [0x800A3604] (count word 0x800A3608), issue 8007FB44(rec[4] byte, rec[12]);
#   0x8007F88C  libds status callback (progress slot 0x800A36A8; status, response): 8007E704(5, response) for status 5
#               with response[0] bit 4, per-status table records at 0x800A3520 / 0x800A3530 (8-byte copy of the response),
#               then the user callback word 0x800B8AB4 (status byte, response);
#   0x80080778  default CD ready callback (status, response): 8008080C(status byte, response), flag byte 0x8009B56C bit 4,
#               user slot 0x800A36A8 gated by 0x8009B554.
# 'cdgen42ci' = 0x80080778 only with 8008080C run inline as original code (for a provider whose decode is not a separate
# call site); 'cdgen42c' stops it as a recorded callee.  Callback targets are stopped callees (sentinels 0x80131300/40/80, and
# the real slot targets that have callee records).  Runtime tuples: CDXT cb7F7E8 lines (callee results observed inline in the
# legacy object) and CDXT afterdecode lines (the state the decode's callers see; 80080164 / 80080778 are indistinguishable).
CB42C_SENT = (0x80131300, 0x80131340, 0x80131380)


def _load_cdgen42c(m, inline_decode):
    import os
    import re
    import struct
    c, gv = m.c, m.gv
    s = m.spec
    b = 'cdgen42ci' if inline_decode else 'cdgen42c'

    def put32(ram, a, v):
        struct.pack_into('<I', ram, a & 0x1FFFFF, v & 0xFFFFFFFF)

    def put8(ram, a, v):
        ram[a & 0x1FFFFF] = v & 0xFF

    def hsh(*k):
        h = 0x9E3779B1
        for q in k:
            h = gv.xs32((h ^ (q & 0xFFFFFFFF)) * 2654435761 & 0xFFFFFFFF or 1)
        return h
    RT = {'vb': [], 'afterdecode': [], 'scb': []}
    for path in [q for q in os.environ.get('CDX42B_TRACE', '').split(',') if q]:
        for ln in open(path, errors='replace'):
            mm = re.match(r'CDXT cb7F7E8 .*in\[idx=(\w+) cnt=(\w+) rec0=(\w+) rec4=(\w+) rec12=(\w+)\]((?: lane\(\w+\)->\w+| issue\(\w+,\w+\)->\w+)*)', ln)
            if mm:
                tail = mm.group(6)
                t = dict(zip(('idx', 'cnt', 'rec0', 'rec4', 'rec12'), (int(g, 16) for g in mm.groups()[:5])))
                t['lane'] = [int(q, 16) for q in re.findall(r'lane\(\w+\)->(\w+)', tail)]
                t['issue'] = [int(q, 16) for q in re.findall(r'issue\(\w+,\w+\)->(\w+)', tail)]
                if t not in RT['vb']:
                    RT['vb'].append(t)
                continue
            mm = re.match(r'CDXT table3520 vsync=\d+ slotB8AB4=(\w+) \| ((?:[0-9A-Fa-f]{2} ?){32})', ln)
            if mm:      # 0x8007F88C wrote its record (word 1, status byte, 8 response bytes): the observed (status, response bytes, user slot)
                bts = [int(q, 16) for q in mm.group(2).split()]
                for half in (0, 16):
                    if bts[half] == 1 and not any(bts[half + 1:half + 4]):
                        t = {'status': bts[half + 4], 'bytes': bts[half + 5:half + 13], 'slot': int(mm.group(1), 16)}
                        if t not in RT['scb']:
                            RT['scb'].append(t)
                continue
            mm = re.match(r'CDXT afterdecode status=(\w+) resp=(\w+) b56C=(\w+) slot36A8=(\w+) g554=(\w+)', ln)
            if mm:
                t = dict(zip(('status', 'resp', 'b56C', 'slot', 'g554'), (int(g, 16) for g in mm.groups())))
                if t not in RT['afterdecode']:
                    RT['afterdecode'].append(t)
    REP, NRAND = 8, 400
    for key in RT:
        RT[key] = [t for t in RT[key] for _ in range(REP)]
    COV = {}

    def cov(fn, key):
        COV.setdefault(fn, {})
        COV[fn][key] = COV[fn].get(key, 0) + 1
    m.COV = COV
    m.RT = RT
    cbs = {a: 2 for a in CB42C_SENT}

    # ---- 0x8007F7E8 ---------------------------------------------------------------------------------------------
    LANE1 = (0, 1, 2, 0xFFFFFFFF, 0x10, 0x80000000)
    LANE2 = ('eq', 'plus1', 0, 1, 0xFFFFFFFF)
    CNT = (0, 1, 2, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
    IDX = (0, 1, 2, 4, 5, 0x1F, 0xFFFFFFFF)
    R0 = (0, 1, 'rand')
    NVB0 = len(LANE1) * len(LANE2) * len(CNT) * len(IDX) * len(R0)
    # deep block: the path that reads the queue record and issues (lane 1 / 1, positive count): count x idx x rec0 x rec4 x rec12
    DCNT, DR0, DR4, DR12 = (1, 2, 0x7FFFFFFF), (0, 1, 2, 0x80000000, 0x152, 'rand'), (0, 9, 0xFF, 'rand'), (0, 'rand', 0x80100040)
    NDEEP = len(DCNT) * len(IDX) * len(DR0) * len(DR4) * len(DR12)
    NVB = NVB0 + NDEEP

    def vb_apply(ram, rets, idx, cnt, r0, r4, r12, lane, issue):
        put32(ram, 0x800A3604, idx); put32(ram, 0x800A3608, cnt)
        rec = (0x800A3540 + idx * 24) & 0xFFFFFFFF
        put32(ram, rec, r0); put8(ram, rec + 4, r4); put32(ram, rec + 12, r12)
        rets[0x8007FBF0] = list(lane)
        rets[0x8007FB44] = issue
        st = [0]
        seq = list(lane)

        def lane_model(ram_, r, seq=seq, st=st):
            v_ = seq[min(st[0], len(seq) - 1)]
            st[0] += 1
            return v_
        return {0x8007FBF0: lane_model}

    def vb_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['vb'])
        h = hsh(seed, 41)
        if m.PILOT[0] or k >= nrt + NVB:
            idx = (IDX + (h >> 5,))[(h >> 3) % 8] if (h >> 3) % 8 < 7 else h % 0x40
            cnt = (CNT + (h >> 9, 3))[(h >> 12) % 8]
            l1 = (LANE1 + (h >> 16,))[(h >> 20) % 7]
            l2 = (l1, (l1 + 1) & 0xFFFFFFFF, h >> 4, 1)[(h >> 22) % 4]
            models = vb_apply(ram, rets, idx, cnt, (0, 1, h >> 7)[(h >> 24) % 3], h >> 8, h >> 14, [l1, l2], (0, 1, 0xFFFFFFFF, h >> 6)[(h >> 26) % 4])
            if k >= nrt + NVB:
                cov('7F7E8', 'random')
            return x, models
        if k < nrt:
            t = RT['vb'][k]
            lane = t['lane'] or [0]
            models = vb_apply(ram, rets, t['idx'], t['cnt'], t['rec0'], t['rec4'], t['rec12'], lane, (t['issue'] or [1])[0])
            cov('7F7E8', 'runtime')
            return x, models
        p = k - nrt
        if p >= NVB0:
            p -= NVB0
            j1, p = p % len(DCNT), p // len(DCNT)
            j2, p = p % len(IDX), p // len(IDX)
            j3, p = p % len(DR0), p // len(DR0)
            j4, p = p % len(DR4), p // len(DR4)
            j5 = p
            rnd = h | 1
            models = vb_apply(ram, rets, IDX[j2], DCNT[j1], rnd if DR0[j3] == 'rand' else DR0[j3], (h >> 9) & 0xFF if DR4[j4] == 'rand' else DR4[j4],
                              (rnd >> 3) if DR12[j5] == 'rand' else DR12[j5], [1, 1], (0, 1, 0xFFFFFFFF)[(h >> 13) % 3])
            cov('7F7E8', 'deep')
            return x, models
        i1, p = p % len(LANE1), p // len(LANE1)
        i2, p = p % len(LANE2), p // len(LANE2)
        i3, p = p % len(CNT), p // len(CNT)
        i4, p = p % len(IDX), p // len(IDX)
        i5 = p
        l1 = LANE1[i1]
        l2 = l1 if LANE2[i2] == 'eq' else (l1 + 1) & 0xFFFFFFFF if LANE2[i2] == 'plus1' else LANE2[i2]
        r0 = R0[i5] if R0[i5] != 'rand' else (h | 1)
        models = vb_apply(ram, rets, IDX[i4], CNT[i3], r0, (0, 9, 0xFF, h >> 9)[(h >> 3) % 4], (0, h >> 5, ZONE_PTR)[(h >> 11) % 3],
                          [l1, l2], (0, 1, 0xFFFFFFFF)[(h >> 13) % 3])
        cov('7F7E8', 'product')
        return x, models
    ZONE_PTR = 0x80100040
    s(0x8007F7E8, 'ds_vblank_hook', '', {0x8007FBF0: 1, 0x8007FB44: 2}, batch=b, void=True,
      n=len(RT['vb']) + NVB + NRAND)
    m.HOOKS[0x8007F7E8] = vb_hook

    # ---- 0x8007F88C ---------------------------------------------------------------------------------------------
    ST = (0, 1, 2, 3, 4, 5, 6, 7, 0x10, 0xFF, 0x1234505)
    R0B = (0x00, 0x10, 0x20, 0x30, 0xFF, 0xEF)
    PT = (0x80100000, 0x801FE400, 0x801FEC00, 0x00100100, 0xA0100200, 0x800A3460)
    SL = (0, CB42C_SENT[0], CB42C_SENT[1])
    NCB = len(ST) * len(R0B) * len(PT) * len(SL)

    def cb_hook(ram, x, args, rets, seed):
        k = seed - 1
        h = hsh(seed, 43)
        nrt = len(RT['scb'])
        if m.PILOT[0] or k >= nrt + NCB:
            put32(ram, 0x800B8AB4, (0, CB42C_SENT[(h >> 3) % 3])[(h >> 6) & 1])
            if k >= nrt + NCB:
                cov('7F88C', 'random')
            return x, None
        if k < nrt:
            # runtime proxy: the record the inline body wrote at boot (status, 8 response bytes, user slot); the response pointer
            # is not observable (inline body) -- 0x800A3468 is the pointer of the DS data callback of the same event (CDXT call line)
            t = RT['scb'][k]
            args[0] = t['status']; args[1] = 0x800A3468
            for q, v_ in enumerate(t['bytes']):
                ram[(0x800A3468 + q) & 0x1FFFFF] = v_
            put32(ram, 0x800B8AB4, t['slot'])
            cov('7F88C', 'runtime')
            return x, None
        k -= nrt
        p = k
        i1, p = p % len(ST), p // len(ST)
        i2, p = p % len(R0B), p // len(R0B)
        i3, p = p % len(PT), p // len(PT)
        i4 = p
        args[0] = ST[i1]
        args[1] = PT[i3]
        base_ = PT[i3] & 0x1FFFFF
        for q in range(8):
            ram[(base_ + q) & 0x1FFFFF] = (h >> (3 * q)) & 0xFF
        ram[base_ & 0x1FFFFF] = R0B[i2]
        put32(ram, 0x800B8AB4, SL[i4])
        cov('7F88C', 'product')
        return x, None
    s(0x8007F88C, 'ds_status_callback', 'ew', {**{0x8007E704: 2}, **cbs}, batch=b, void=True, inline=(0x80080998,),
      n=len(RT['scb']) + NCB + NRAND) if not inline_decode else None
    if not inline_decode:
        m.HOOKS[0x8007F88C] = cb_hook

    # ---- 0x80080778 ---------------------------------------------------------------------------------------------
    ST3 = (0, 1, 2, 3, 5, 0xFF, 0x1234502)
    FL = (0x00, 0x10, 0x02, 0x12, 0xFF, 0xE0)
    SL3 = (0, 0x8007F88C, CB42C_SENT[0])
    G554 = (0, 1, 2, 0xFFFFFFFF)
    RP = (0x80100000, 0x00100100, 0x800A3460, 0x801FED80) if inline_decode else (0, 0x80100000, 0x800A3460, 0xFFFFFFFF)   # inline decode dereferences the response
    N778 = len(ST3) * len(FL) * len(SL3) * len(G554) * len(RP)

    def rd_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['afterdecode'])
        h = hsh(seed, 47)
        if m.PILOT[0] or k >= nrt + N778:
            put8(ram, 0x8009B56C, (FL + (h >> 9,))[(h >> 3) % 7]); put32(ram, 0x800A36A8, (SL3 + (h >> 7,))[(h >> 12) % 4] if (h >> 12) % 4 < 3 else CB42C_SENT[1])
            put32(ram, 0x8009B554, (G554 + (h >> 11,))[(h >> 15) % 5])
            if k >= nrt + N778:
                cov('80080778', 'random')
            return x, None
        if k < nrt:
            t = RT['afterdecode'][k]
            args[0] = t['status']; args[1] = t['resp']
            put8(ram, 0x8009B56C, t['b56C']); put32(ram, 0x800A36A8, t['slot']); put32(ram, 0x8009B554, t['g554'])
            if inline_decode:
                # the decode runs inline and rewrites 0x8009B56C from resp[idx]: reproduce the observed post-decode byte through the
                # command table (command 1 -> table word 1 -> index 0) and resp[0]
                put8(ram, 0x8009B558, 1); put32(ram, 0x8009B624 + 4, 1)
                ram[t['resp'] & 0x1FFFFF] = t['b56C']
            cov('80080778', 'runtime')
            return x, None
        p = k - nrt
        i1, p = p % len(ST3), p // len(ST3)
        i2, p = p % len(FL), p // len(FL)
        i3, p = p % len(SL3), p // len(SL3)
        i4, p = p % len(G554), p // len(G554)
        i5 = p
        args[0] = ST3[i1]
        args[1] = RP[i5]
        put8(ram, 0x8009B56C, FL[i2]); put32(ram, 0x800A36A8, SL3[i3]); put32(ram, 0x8009B554, G554[i4])
        put32(ram, 0x8009B574, h); put32(ram, 0x8009B578, h >> 3)      # overwritten by the original on the bit-4 path
        cov('80080778', 'product')
        return x, None
    callees778 = {**cbs, 0x8007F88C: 2}
    if inline_decode:
        s(0x80080778, 'cd_ready_default_cb', 'ew', callees778, batch=b, void=True, inline=(0x8008080C, 0x80080998),
          n=len(RT['afterdecode']) + N778 + NRAND)
    else:
        callees778[0x8008080C] = 2
        s(0x80080778, 'cd_ready_default_cb', 'ew', callees778, batch=b, void=True,
          n=len(RT['afterdecode']) + N778 + NRAND)
    m.HOOKS[0x80080778] = rd_hook


# ------------------------------------------------------------------------------------------------------------------
# cdgen48: the SPU data upload 0x800850F4 (SpuWrite with prepare: arms the DMA-complete flag and callback, then the
# libspu write path) and its DMA-complete wait 0x80085174 (sw-vec-spu-upload, 2026-11-27).
#
# Retail structure (disassembly of the EXE, tools/analysis/pe_dis.sh; the vectors are what the interpreter does):
#   800850F4(src, size): 800850C0 (stores 1 to 0x8009D24C, then 80085F44(0x80085098) = SetTransferCallback: stores the
#     callback word 0x8009B434 only when it differs), then 80085E54(src, size) (size clamped to 0x7EFF0 unsigned; calls
#     8007D9F8; returns the clamped size; clears 0x8009B430 only when the callback word is 0).
#     8007D9F8: transfer-mode word 0x8009B418 == 0 (DMA): 8007D778(2, B414 << B424) (TSA halfword B414 = value >> B424,
#     store to SPU TSA 0x1F801DA6), 8007D778(1) (read back TSA up to 3841 times, then SPUCNT 0x1F801DAA mode bits 0x20),
#     8007D778(3, src, size) (poll SPUCNT & 0x30 == 0x20, DPCR 0x1F8010F0 RMW through 8007DC5C, then DMA4 MADR = src,
#     BCR = ((size >> 6) + (size & 0x3F != 0)) << 16 | 0x10, CHCR = 0x01000201); nonzero B418 (manual transfer):
#     8007D454(src, size) writes halfwords to the data port 0x1F801DA8 in chunks of 64 bytes, polling SPUSTAT 0x1F801DAE.
#   80085174(): returns at once unless 0x8009D24C == 1, else polls the word while it reads 1.  Only the DMA4 interrupt
#     handler (callback 0x80085098) clears the word in production; the interpreter has no interrupts, so the clearing is a
#     HARNESS MODEL: the word reads 1 for m reads and then a terminal value T (MEMSEQ, replayed in order by the checker).
# Variants (pe_cdx_vectors.py -variant NAME): '' = nothing stopped (register level, MMIO footprint is the observable);
# 'bound' = 80085F44 and 80085E54 stopped (their host-symbol bindings in pe_plat/spu_b.h and state.h) so the observable is
# the callee records and the RAM word; 'fault' = register level plus stuck TSA / sticky SPUCNT hardware models (timeouts).
# ------------------------------------------------------------------------------------------------------------------
def _load_cdgen48(m):
    import os
    import re
    import struct
    c, gv = m.c, m.gv
    s = m.spec
    b = 'cdgen48'
    variant = getattr(m, 'VARIANT', '')
    A_B414, A_B418, A_B420, A_B424, A_B428, A_B42C, A_B430, A_B434 = (0x9B414, 0x9B418, 0x9B420, 0x9B424, 0x9B428, 0x9B42C, 0x9B430, 0x9B434)
    A_D24C = 0x9D24C
    R_TSA, R_CNT, R_STAT, R_DELAY = 0x1DA6, 0x1DAA, 0x1DAE, 0x1014   # R_DELAY = SPU_DELAY 0x1F801014 (the register 0x8009B410 points at; 8007DC5C RMWs it)
    CB_RETAIL = 0x80085098

    def p32(ram, a, v):
        struct.pack_into('<I', ram, a & 0x1FFFFF, v & 0xFFFFFFFF)

    def p16(ram, a, v):
        struct.pack_into('<H', ram, a & 0x1FFFFF, v & 0xFFFF)

    def hsh(*k):
        h = 0x9E3779B1
        for q in k:
            h = gv.xs32((h ^ (q & 0xFFFFFFFF)) * 2654435761 & 0xFFFFFFFF or 1)
        return h

    # ---- runtime tuples (CDXT lines of pe_cdx_trace_wrap48.c, env CDX48_TRACE = comma-separated logs) ---------------
    RT = {'upload': [], 'wait': []}
    for path in [q for q in os.environ.get('CDX48_TRACE', '').split(',') if q]:
        for ln in open(path, errors='replace'):
            mm = re.match(r'CDXT (upload|wait|hostwait) ', ln)
            if not mm:
                continue
            kv = {k: int(v, 16) for k, v in re.findall(r'(\w+)=([0-9A-Fa-f]+)', ln) if k not in ('call', 'frame')}
            if kv not in RT['wait' if mm.group(1) == 'hostwait' else mm.group(1)]:
                RT['wait' if mm.group(1) == 'hostwait' else mm.group(1)].append(kv)
    REP = 4
    for key in RT:
        RT[key] = [t for t in RT[key] for _ in range(REP)]
    COV = {}

    def cov(fn, key):
        COV.setdefault(fn, {})
        COV[fn][key] = COV[fn].get(key, 0) + 1
    m.COV = COV
    m.RT = RT

    SRC_DMA = (0x8009B7FC, 0x80100000, 0x80100001, 0x80100002, 0x80100003, 0x801FE000, 0xA0100000, 0x00100000, 0,
               0xFFFFFFFF, 0x1F800000, 0x80000000, 0x8009B7FD)
    SIZE_DMA = (0x20, 0, 1, 2, 3, 0x3F, 0x40, 0x41, 0x7F, 0x80, 0x1000, 0x12345, 0x7EFEF, 0x7EFF0, 0x7EFF1, 0x80000,
                0xFFFFFFFF, 0x80000000)
    SIZE_PIO = (0, 1, 2, 3, 0x20, 0x3F, 0x40, 0x41, 0x80, 0xC1, 0x100, 0x180)
    SRC_PIO = (0x80100000, 0x80100200, 0x80100ffe, 0x8009B7FC)
    SHIFT = (3, 0, 4, 16, 31)
    TSAV = (0x200, 0, 0x202, 0x1010, 0xFFFF, 0x8000, 0x101)
    CBV = (CB_RETAIL, 0, 0x80123456, 0xFFFFFFFF)
    FLAGV = (0, 1, 2, 0xFFFFFFFF)
    CNTV = (0xC000, 0xC020, 0, 0x8000, 0x0030, 0xFFFF, 0x1234)
    DELAYV = (0x07654321, 0, 0xFFFFFFFF, 0x20000000, 0x12345678)
    MODEV = (1, 2, 0x100, 0xFFFFFFFF)

    def common_state(ram, h, b414, b418, b424, b434):
        p16(ram, A_B414, b414)
        p32(ram, A_B418, b418)
        p32(ram, A_B424, b424)
        p32(ram, A_B434, b434)
        # words none of the entries reads (SpuInit alignment state B420/B428/B42C, the clear word B430): seeded in the
        # interpreter run only.  The vector lists loads, so these words are NOT in the vector (the checker zero-fills RAM);
        # a provider that reads them fails the read-outside-original-loads check
        p32(ram, A_B420, (2, 0, 1, 7, 0x80000000, h >> 3)[(h >> 5) % 6])
        p32(ram, A_B428, (8, 1, 2, 0x100, 0xFFFFFFFF, h >> 9)[(h >> 11) % 6])
        p32(ram, A_B42C, (7, 0, 0xFF, 0x7FFFFFFF, h >> 13)[(h >> 17) % 5])
        p32(ram, A_B430, (0, 1, 0xFFFFFFFF, h >> 7)[(h >> 19) % 4])
        p32(ram, A_D24C, FLAGV[(h >> 21) % 4])
        p16(ram, R_CNT, CNTV[(h >> 23) % len(CNTV)])
        p16(ram, R_TSA, (b414 ^ (0x100 << ((h >> 27) % 4))) & 0xFFFF if (h >> 26) & 1 else h >> 4)
        p32(ram, R_DELAY, DELAYV[(h >> 25) % len(DELAYV)])

    def pio_stat_seq(ram, h, size):
        """SPUSTAT read sequence for the manual-transfer path: entry read, per 64-byte chunk k busy reads then a clear read,
        optional settle mismatches, final read equal to the entry read's low 11 bits (harness model of the busy bit)."""
        st = ((h >> 3) & 0xFBFF) if (h >> 20) & 1 else (h >> 8) & 0x03FF
        clear, busy = st & ~0x400, st | 0x400
        chunks = (size + 63) // 64
        seq = [clear]
        for i in range(chunks):
            k = (0, 0, 1, 2, 3, 0, 5)[((h >> (4 + i)) ^ i) % 7]
            seq += [busy] * k + [clear]
        f = (0, 0, 1, 3)[(h >> 14) % 4]
        seq += [clear ^ 1 if (clear ^ 1) & 0x7FF != clear & 0x7FF else clear ^ 2] * f + [clear]
        m.REGSEQ[R_STAT] = [seq, 0]
        p16(ram, R_STAT, clear)

    # ---- 0x800850F4: product cells ------------------------------------------------------------------------------
    NA = len(SIZE_DMA) * len(SRC_DMA)
    CELLS = len(SHIFT) * len(TSAV) * len(CBV)
    NB = 0 if variant == 'bound' else CELLS * 2     # bound: no state word is read, the cells would only repeat the size x source rows
    NC = len(MODEV) * len(SIZE_PIO) * len(SRC_PIO) // 2 + len(SIZE_PIO) * 3
    FAULT_SZ = (0x20, 0, 1, 0x40, 0x41, 0x7EFF1)
    FAULT_SRC = (0x8009B7FC, 0x80100000, 0x80100003)
    NF = 3 * len(FAULT_SZ) * len(FAULT_SRC) * 2
    NRAND = 160

    def up_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['upload'])
        if variant == 'fault':
            h = hsh(seed, 48)
            if m.PILOT[0]:
                args[0], args[1] = 0x8009B7FC, 0x20
                common_state(ram, h, 0x200, 0, 3, CB_RETAIL)
                return x, None
            if k < NF:
                kind, j = k % 3, k // 3
                sz, src = FAULT_SZ[j % len(FAULT_SZ)], FAULT_SRC[(j // len(FAULT_SZ)) % len(FAULT_SRC)]
                args[0], args[1] = src, sz
                common_state(ram, h, TSAV[j % len(TSAV)], 0, SHIFT[j % 3], CB_RETAIL)
                p16(ram, R_CNT, 0xC000)
                if kind in (0, 2):
                    m.STUCK.add(R_TSA)                       # the TSA store is not latched: readback keeps the entry value
                    b414 = struct.unpack_from('<H', ram, A_B414)[0]
                    p16(ram, R_TSA, b414 ^ 0x0100)
                if kind in (1, 2):
                    m.c.SPU_STICKY[0] = (0x10, 0x30)[j % 2]  # SPUCNT mode bits that a store cannot clear
                cov('upload', 'fault%d' % kind)
                return x, None
            args[0], args[1] = (SRC_DMA[h % len(SRC_DMA)], SIZE_DMA[(h >> 8) % len(SIZE_DMA)])
            common_state(ram, h, TSAV[(h >> 4) % len(TSAV)], 0, SHIFT[(h >> 10) % 3], CB_RETAIL)
            cov('upload', 'random')
            return x, None
        if m.PILOT[0]:
            h = hsh(seed, 31)
            pio = h & 1
            args[0], args[1] = (SRC_PIO[h % 4] if pio else SRC_DMA[(h >> 8) % 8]), (SIZE_PIO[(h >> 4) % 12] if pio else SIZE_DMA[(h >> 12) % 18])
            common_state(ram, h, TSAV[(h >> 4) % 7], MODEV[(h >> 16) % 4] if pio else 0, SHIFT[(h >> 8) % 5], CBV[(h >> 12) % 4])
            if pio and variant != 'bound':
                pio_stat_seq(ram, h, args[1])
            return x, None
        h = hsh(seed, 77)
        if k < nrt:
            t = RT['upload'][k]
            args[0], args[1] = t['src'], t['size']
            common_state(ram, h, t['b414'], t['b418'], t['b424'], t['b434'])
            p32(ram, A_D24C, t['d24c'])
            cov('upload', 'runtime')
            return x, None
        p = k - nrt
        pio_sz = size = src = None
        if p < NA:                                   # A: every size x every source at the boot state
            size, src = SIZE_DMA[p // len(SRC_DMA)], SRC_DMA[p % len(SRC_DMA)]
            common_state(ram, h, 0x200, 0, 3, CB_RETAIL)
            cov('upload', 'size-x-src')
        elif p < NA + NB:                            # B: (shift, tsa, callback) cells, two cases each
            q = p - NA
            cell, j = divmod(q, 2)
            sh, ts, cb = SHIFT[cell % len(SHIFT)], TSAV[(cell // len(SHIFT)) % len(TSAV)], CBV[cell // (len(SHIFT) * len(TSAV))]
            size, src = SIZE_DMA[(cell * 2 + j) % len(SIZE_DMA)], SRC_DMA[(cell * 5 + j) % len(SRC_DMA)]
            common_state(ram, h, ts, 0, sh, cb)
            cov('upload', 'state-cells')
        elif p < NA + NB + NC and variant != 'bound':   # C: manual-transfer (PIO) path, SPUSTAT busy sequences
            q = p - NA - NB
            mode = MODEV[q % len(MODEV)]
            size, src = SIZE_PIO[(q // 2) % len(SIZE_PIO)], SRC_PIO[(q // 3) % len(SRC_PIO)]
            common_state(ram, h, TSAV[q % len(TSAV)], mode, SHIFT[q % 3], CBV[q % 2])
            pio_stat_seq(ram, h, size)
            cov('upload', 'pio')
        else:                                        # D: random fill
            size, src = SIZE_DMA[(h >> 8) % len(SIZE_DMA)] if (h >> 3) & 1 else (h >> 6) & 0xFFFFFF, \
                SRC_DMA[(h >> 16) % len(SRC_DMA)] if (h >> 4) & 1 else gv.xs32(h)
            b418 = 0 if (h >> 5) % 5 else 1
            if b418 and variant != 'bound':
                src, size = SRC_PIO[(h >> 18) % 4], SIZE_PIO[(h >> 20) % 12]
                pio_stat_seq(ram, h, size)
            common_state(ram, h, TSAV[(h >> 9) % 7] if (h >> 7) & 1 else (h >> 12) & 0xFFFF, b418,
                         SHIFT[(h >> 22) % 5] if (h >> 24) & 1 else (h >> 25) % 32,
                         CBV[(h >> 26) % 4])
            cov('upload', 'random')
        args[0], args[1] = src, size
        return x, None

    if variant == 'bound':
        s(0x800850F4, 'spu_data_upload', 'ww', {0x80085F44: 1, 0x80085E54: 2}, batch=b, void=True,
          inline=(0x800850C0,), n=len(RT['upload']) + NA + NB + NRAND)
    else:
        s(0x800850F4, 'spu_data_upload', 'ww', {}, batch=b, void=True, bios=True,
          bios_silent=((0xA0, 0x3F), (0xB0, 0x3F)), inline=(0x800850C0, 0x80085E54),
          n=(NF + 92 if variant == 'fault' else len(RT['upload']) + NA + NB + NC + NRAND))
    m.HOOKS[0x800850F4] = up_hook

    # ---- 0x80085174: DMA-complete wait ------------------------------------------------------------------------------
    F0 = (0, 1, 2, 0xFFFFFFFF, 0x80000000, 0x100, 3)
    POLLS = (1, 2, 3, 4, 5, 8, 16, 63, 64, 65, 255, 256, 1000, 4000)
    TERM = (0, 2, 0xFFFFFFFF, 0x100, 3, 0x80000000)
    CLEAR_F = (0, 2, 0xFFFFFFFF, 0x80000000, 0x100, 3)
    NW1 = len(POLLS) * len(TERM) * 3
    NW0 = len(F0) * 6
    A_NEAR = (0x9D240, 0x9D244, 0x9D248, 0x9D250, 0x9D254, 0x9D258)

    def wait_hook(ram, x, args, rets, seed):
        k = seed - 1
        nrt = len(RT['wait'])
        h = hsh(seed, 55)
        for i, a in enumerate(A_NEAR):                       # neighbours of the flag: a provider that reads them is caught
            p32(ram, a, (0, 1, 0xFFFFFFFF, h >> (i + 3), 2)[(h >> (2 * i)) % 5])
        p32(ram, A_B414, (0x200, 0, h >> 4)[(h >> 9) % 3])
        p32(ram, A_B418, (0, 1)[(h >> 11) & 1])
        p32(ram, A_B424, (3, 0, 4, 5)[(h >> 12) % 4])
        p32(ram, A_B434, (CB_RETAIL, 0, h)[(h >> 14) % 3])
        p32(ram, A_B430, h >> 5)
        if m.PILOT[0]:
            return x, None
        if k < nrt:
            f0, mm_, term = RT['wait'][k]['d24c'], 1 + (h >> 8) % 3, (0, 2, 3)[(h >> 12) % 3]
            cov('wait', 'runtime')
        elif k < nrt + NW1:
            q = k - nrt
            f0, mm_, term = 1, POLLS[q % len(POLLS)], TERM[(q // len(POLLS)) % len(TERM)]
            cov('wait', 'polls-x-terminal')
        elif k < nrt + NW1 + NW0 * 4:
            q = k - nrt - NW1
            f0, mm_, term = (CLEAR_F + ((h >> 6) | 2,))[q % 7], 0, 0
            cov('wait', 'flag-clear')
        else:
            if (h >> 3) % 3:
                f0, mm_, term = 1, 1 + (h >> 8) % 70, (0, 2, h | 2)[(h >> 20) % 3]
            else:
                f0, mm_, term = (h >> 6) | 2, 0, 0
            cov('wait', 'random')
        if f0 == 1:
            m.c.MEMSEQ[A_D24C] = [[1] * mm_ + [term & 0xFFFFFFFF], 0]
        else:
            p32(ram, A_D24C, f0)
        return x, None

    s(0x80085174, 'spu_dma_wait', '', {}, batch=b, void=True,
      n=len(RT['wait']) + NW1 + NW0 * 4 + 120)
    m.HOOKS[0x80085174] = wait_hook

    # ---- 0x80085098: the DMA-complete callback registered by 800850C0 (SetTransferCallback arg) -----------------------
    # Retail: 80085F44(0) (clears the callback word 0x8009B434 when it is nonzero), then 0x8009D24C := 0.  Inputs: the
    # callback word only (read by 80085F44, inline in the '' variant).  'bound': 80085F44 stopped (1 argument, always 0).
    CBW = (0, CB_RETAIL, 0x80123456, 0xFFFFFFFF, 1, 0x80000000)

    def cb_hook(ram, x, args, rets, seed):
        h = hsh(seed, 98)
        p32(ram, A_B434, CBW[seed % len(CBW)] if seed <= len(CBW) * 4 * 4 else (CBW[(h >> 4) % len(CBW)] if (h >> 9) & 1 else h))
        p32(ram, A_D24C, FLAGV[(seed // len(CBW)) % 4] if seed <= len(CBW) * 16 else (FLAGV[(h >> 11) % 4] if (h >> 10) & 1 else h >> 3))
        p32(ram, A_B430, h >> 5)
        p16(ram, A_B414, h >> 13)
        p32(ram, A_B418, h >> 7)
        cov('callback', 'product' if seed <= len(CBW) * 16 else 'random')
        return x, None

    if variant == 'bound':
        s(0x80085098, 'spu_dma_irq_callback', '', {0x80085F44: 1}, batch=b, void=True, n=300)
    else:
        s(0x80085098, 'spu_dma_irq_callback', '', {}, batch=b, void=True, inline=(0x80085F44,), n=300)
    m.HOOKS[0x80085098] = cb_hook

    # ---- 0x800850C0: the prepare step (arms the completion flag and registers the callback) --------------------------
    # Retail: 0x8009D24C := 1, then 80085F44(0x80085098) (inline in the '' variant: B434 read, stored only when different;
    # 'bound': 80085F44 stopped, 1 argument).  Inputs: the callback word only.
    def prep_hook(ram, x, args, rets, seed):
        h = hsh(seed, 0xC0)
        p32(ram, A_B434, CBW[seed % len(CBW)] if seed <= len(CBW) * 16 else (CBW[(h >> 4) % len(CBW)] if (h >> 9) & 1 else h))
        p32(ram, A_D24C, FLAGV[(seed // len(CBW)) % 4] if seed <= len(CBW) * 16 else (FLAGV[(h >> 11) % 4] if (h >> 10) & 1 else h >> 3))
        p32(ram, A_B430, h >> 5)
        p16(ram, A_B414, h >> 13)
        cov('prepare', 'product' if seed <= len(CBW) * 16 else 'random')
        return x, None

    if variant == 'bound':
        s(0x800850C0, 'spu_prepare_transfer', '', {0x80085F44: 1}, batch=b, void=True, n=300)
    else:
        s(0x800850C0, 'spu_prepare_transfer', '', {}, batch=b, void=True, inline=(0x80085F44,), n=300)
    m.HOOKS[0x800850C0] = prep_hook
