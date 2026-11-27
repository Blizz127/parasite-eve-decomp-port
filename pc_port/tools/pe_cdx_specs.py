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
