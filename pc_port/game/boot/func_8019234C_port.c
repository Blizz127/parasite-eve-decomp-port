/*
 * Original transition overlay outer loop, 8019234C..80192740 (253 words).
 * SHA256 caa53e35c6e87356c8743445c2bac01cc2b77d97946ea64eb717f077bd3fc0e8.
 *
 * This is the M0000I frame driver reached from func_8001220C's A8 sentinel
 * (0xA8000048) call site.  It is a real translation: it preserves the
 * constructor hand-off, the button exit, the ordered per-frame calls, the
 * first-frame/retained effect handle logic, the ordering-table compaction,
 * the double-buffered descriptor flip and the exit selector.
 *
 * Behavioral caveats (see docs/ai_context/TRANSITION_OUTER_LOOP.md):
 *  - func_80191E30 is the first-frame effect start.  Only its retail call
 *    (handle id 0xABE, position *801EA578+0x1C) is reproduced; its package
 *    argument (0x801D0260) is not modelled.
 *  - func_80073A44 = VSync.  HostFB_VSync now returns the original contract
 *    values (absolute counter for negative modes, entry-time 16-bit timer
 *    delta otherwise) so the mode-1 store and the RNG seed query are real.
 *  - func_80191D94 is called but its fields are not modelled (data-only).
 *  - func_80037870 (message/tutorial update) is a translated leaf port.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

/* Original 80191D94: ~11-word initializer with no calls; not in pe_port
 * (data-only effect state).  Kept as an explicit no-call placeholder. */
static void transition_field_initialize(uint32_t value)
{
    PE_StoreU32(0x8019C028u, value);
}

void func_8019234C(void)
{
    unsigned epoch = PE_Port_StopEpoch();
    pe_addr_t descriptor;
    pe_addr_t ot;
    uint32_t flags;
    uint32_t mode;
    uint32_t handle = 0u;
    uint32_t first_rendered;
    uint32_t empty_start;
    uint32_t target;

    /* 8019236C..80192380: clear the double-buffer index and the exit byte,
     * construct the scene, then enter the empty-run loop with s2=0. */
    PE_StoreU32(0x8009CDDCu, 0u);
    PE_StoreU8(0x8019C00Eu, 0u);
    func_80196498();
    if (PE_Port_StopEpoch() != epoch) return;
    func_80191DE8(PE_LoadU16(0x8019C024u) == 10 ? 0 : 1);
    if (PE_Port_StopEpoch() != epoch) return;

    /* 8019239C..801923AC: seed the BIOS RNG from the absolute VBlank
     * counter, then scatter the low bits over the two 32-bit state words. */
    func_80071A64(func_80073A44(-1));
    if (PE_Port_StopEpoch() != epoch) return;
    func_80071A54();

    /* 801923B4..8019240C: rand()%3000000 (the modulo-by-multiply idiom). */
    {
        uint32_t random = func_80071A54();
        PE_StoreU32(0x8019C03Cu, random % 3000000u);
    }
    mode = PE_LoadU16(0x8019C024u);
    if (mode == 10)
        PE_StoreU32(0x8019C03Cu, 120000u);

    /* 8019242C..80192438: a nonzero entry-mode half skips the empty-run
     * index; the retained handle starts at zero either way. */
    if (PE_LoadU16(0x8019C024u) != 0) goto exit_check;

    first_rendered = 0;
    empty_start = 0;
    transition_field_initialize(1u);

    for (;;) {
        if (PE_Port_ShouldStop()) return;

        /* 80192450: input first, or held-mask test sees stale edges. */
        func_8003EB04();
        if (PE_Port_ShouldStop()) return;

        /* 80192458..80192498: the whole held mask calls the exit path. */
        flags = PE_LoadU32(0x8009D1A0u) & 0x0F000006u;
        if (flags == 0x0F000006u) {
            func_80073A44(0);
            if (PE_Port_StopEpoch() != epoch) return;
            func_80074D28(0);
            if (PE_Port_StopEpoch() != epoch) return;
            PE_StoreU8(0x8019C00Eu, 1u);
            PE_StoreU16(0x8019C024u, 1u);
            func_8006A25C();
            if (PE_Port_StopEpoch() != epoch) return;
        }

        func_801942FC();
        if (PE_Port_StopEpoch() != epoch) return;

        if (PE_LoadU8(0x8019C00Eu) != 0u) goto exit_check;

        /* 801924BC..8019250C: ordered transform, bounds, fade, scene. */
        func_8018F05C();
        if (PE_Port_StopEpoch() != epoch) return;
        func_8018F92C(0x8019C330u);
        if (PE_Port_StopEpoch() != epoch) return;
        if ((int32_t)(int16_t)PE_LoadU16(0x8019C02Au) != 0) {
            PE_StoreU16(0x8019C02Au,
                        (uint16_t)func_80194108((int)(int16_t)PE_LoadU16(0x8019C02Au)));
            if (PE_Port_StopEpoch() != epoch) return;
        }
        func_80192740();
        if (PE_Port_StopEpoch() != epoch) return;
        func_80192800();
        if (PE_Port_StopEpoch() != epoch) return;
        func_80193478();
        if (PE_Port_StopEpoch() != epoch) return;

        /* 80192510..8019255C: on the first rendered frame start the effect
         * and retain its handle; later frames only update, and only while
         * the effect-time word is clear.  A zero handle never retries. */
        if (first_rendered == 0u) {
            target = PE_LoadU32(0x801EA578u) + 0x1Cu;
            handle = (uint32_t)func_80191E30(0xABEu);
            if (PE_Port_StopEpoch() != epoch) return;
            first_rendered = 1u;
        } else if (PE_LoadU32(0x8019C0C0u) == 0u) {
            target = PE_LoadU32(0x801EA578u) + 0x1Cu;
            func_80191EFC(handle, target);
            if (PE_Port_StopEpoch() != epoch) return;
        }

        /* 8019255C..80192598: messages run in mode 10, or for the
         * single-shot C044/C045 transition (exactly one). */
        mode = PE_LoadU16(0x8019C024u);
        if (mode == 10u ||
            (PE_LoadU8(0x8019C044u) == 1u && PE_LoadU8(0x8019C045u) == 0u)) {
            func_80037870();
            if (PE_Port_StopEpoch() != epoch) return;
        }

        /* 801925A0..8019262C: ordering-table compaction. */
        empty_start = PE_TransitionCompactOT(empty_start);

        /* 8019262C..80192738: display pass, descriptor flip, next-bank
         * reset.  The descriptor is reloaded between each SDK call and the
         * bank is flipped/cleared before the new bank is initialized. */
        PE_StoreU32(0x8019CC14u, func_80073A44(1));
        if (PE_Port_StopEpoch() != epoch) return;
        func_80074DC0(0);
        if (PE_Port_StopEpoch() != epoch) return;
        func_80073A44(2);
        if (PE_Port_StopEpoch() != epoch) return;
        func_80193AB0();
        if (PE_Port_StopEpoch() != epoch) return;
        func_80074A44(1);
        if (PE_Port_StopEpoch() != epoch) return;

        descriptor = PE_LoadU32(0x8019C9C0u);
        func_80075424(descriptor + 8u);
        if (PE_Port_StopEpoch() != epoch) return;
        descriptor = PE_LoadU32(0x8019C9C0u);
        func_800755F0(descriptor + 100u);
        if (PE_Port_StopEpoch() != epoch) return;
        descriptor = PE_LoadU32(0x8019C9C0u);
        ot = PE_LoadU32(descriptor + 4u);
        func_800753B4(ot + 16380u);
        if (PE_Port_StopEpoch() != epoch) return;

        descriptor = PE_LoadU32(0x8019C9C0u);
        if (descriptor == 0x8019C1F8u) descriptor = 0x8019C270u;
        else descriptor = 0x8019C1F8u;
        ot = PE_LoadU32(descriptor + 4u);
        PE_StoreU32(0x8019C9C0u, descriptor);
        PE_StoreU32(0x8009CDDCu, PE_LoadU32(0x8009CDDCu) ^ 1u);
        func_800752AC(ot, 4096);
        if (PE_Port_StopEpoch() != epoch) return;
        func_8019BF8C(PE_LoadU32(0x8019C9C0u));
        if (PE_Port_StopEpoch() != epoch) return;

    exit_check:
        if (PE_LoadU16(0x8019C024u) == 0u) continue;
        break;
    }

    /* 801926FC..80192714: exit selector only on the normal path. */
    if (PE_LoadU8(0x8019C00Eu) == 0u) {
        func_80192030();
        if (PE_Port_StopEpoch() != epoch) return;
    }
}
