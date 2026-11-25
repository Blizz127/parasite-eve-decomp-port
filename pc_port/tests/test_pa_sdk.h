#include "pe_bios_string.h"
/*
 * port_absent lane (2026-09-23) — tests for the sdk group
 * (pc_port/game/decomp_hand/absent_sdk_port.c).  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c, never from the port.
 * Guest scratch: 0x80158000..0x8015BFFF.
 */
#include "pe_irq.h"
#include "pe_gpu.h"
#include "pe_timer1.h"

#define PS_S 0x80158000u

/* 1-based position of a boundary name in the registry, 0 when absent. */
static unsigned PS_HasBoundary(const char *name)
{
    for (unsigned i = 0; i < PE_Decomp_BoundaryCount(); i++)
        if (strcmp(PE_Decomp_BoundaryName(i), name) == 0)
            return i + 1u;
    return 0u;
}

static void PS_Reset(void)
{
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    PE_Bios_PrintfLogReset();
}

static void test_PA_sdk_80038954(void)
{
    TEST("PA_sdk_80038954");
    PS_Reset();
    PE_StoreU32(0x80091A2Cu + 4u, 0x80010F00u);    /* D_80091A2C[1] */
    func_80038954(PS_S, PS_S + 0x10u, PS_S + 0x20u, 0x101); /* (u8)a3 == 1 */
    /* printf (0x80071A74) is the host TTY backend PE_Bios_Printf, not a
     * boundary (native only, 2026-10-07). */
    {
        pe_addr_t f; uint32_t a[4];
        ASSERT(PE_Bios_PrintfLog(&f, a) == 2 && g_bootstrap_arg4_call_count == 0, "two printf calls");
        ASSERT(f == 0x80010EC8u && a[0] == PS_S + 0x10u && a[1] == PS_S + 0x20u,
               "second print args");
    }
    ASSERT(PE_Port_ShouldStop(), "low byte 1 is the retail halt");

    PS_Reset();
    func_80038954(PS_S, 0, 0, 2);
    ASSERT(!PE_Port_ShouldStop() && PE_Bios_PrintfLog(0, 0) == 2,
           "other codes return after both prints");
    PASS();
}

static void test_PA_sdk_8005CAEC(void)
{
    TEST("PA_sdk_8005CAEC");
    PS_Reset();
    PE_StoreU8(0x800C0E0Cu, 3u);                 /* func_80052F70() -> 3 */
    PE_StoreU16(0x800C0E48u + 0u, 0x100u);       /* id0 -> D_800BEEAC+0x2000 */
    PE_StoreU16(0x800C0E48u + 2u, 0x201u);       /* id1 -> D_8009DE64+0x4020 */
    PE_StoreU16(0x800C0E48u + 4u, 0x102u);       /* id2 -> D_800BEEAC+0x2040 */
    /* func_800542A0(0x3FE) selects kinds 1..9 (record +6). */
    PE_StoreU8(0x800C0EACu + 6u, 1u);
    PE_StoreU8(0x800C0EACu + 1u, 5u);
    PE_StoreU8(0x800C0EACu + 0x14u, 2u);         /* 2 < 5  -> bit */
    PE_StoreU8(0x800A1E84u + 6u, 0u);            /* kind 0: not selected */
    PE_StoreU8(0x800C0EECu + 6u, 2u);
    PE_StoreU8(0x800C0EECu + 1u, 1u);
    PE_StoreU8(0x800C0EECu + 0x14u, 1u);         /* 1 < 1 false */
    PE_StoreU32(0x8009D05Cu, 0xFFFFFFFFu);       /* cleared first */
    PE_StoreU32(0x8009D060u, 0xFFFFFFFFu);

    ASSERT(func_8005CAEC() == 1, "one slot below its limit");
    ASSERT(PE_LoadU32(0x8009D048u) == 0x800C0E48u &&
           PE_LoadU32(0x8009D050u) == 3u &&
           PE_LoadU32(0x8009D058u) == 0x8009D05Cu &&
           PE_LoadU32(0x8009D064u) == 2u, "list/count/bitmap globals");
    ASSERT(PE_LoadU32(0x8009D040u) == 2u, "two selected slots");
    ASSERT(PE_LoadU32(0x8009D05Cu) == 1u && PE_LoadU32(0x8009D060u) == 0u,
           "bit 0 of the cleared bitmap");
    PASS();
}

static void test_PA_sdk_device_hook(void)
{
    const pe_addr_t tbl = PS_S + 0x100u, name = PS_S + 0x400u;

    TEST("PA_sdk_device_hook");
    PS_Reset();
    PE_StoreU8(name + 0, 'a'); PE_StoreU8(name + 1, 'b');
    PE_StoreU8(name + 2, ':'); PE_StoreU8(name + 3, 'x'); PE_StoreU8(name + 4, 0);
    PE_StoreU8(0x800A32DAu, 0x77u);
    /* empty kernel table: no match, 0, no open */
    ASSERT(func_800727B4(name, 9) == 0, "no device -> 0");
    ASSERT(PE_LoadU8(0x800A32D8u) == 'a' && PE_LoadU8(0x800A32D9u) == 'b' &&
           PE_LoadU8(0x800A32DAu) == 0u, "prefix up to ':' copied, NUL");
    ASSERT(!PS_HasBoundary("func_80072A64"), "no forward without a device");

    PS_Reset();
    PE_StoreU8(name + 0, 'c'); PE_StoreU8(name + 1, ':');
    PE_StoreU32(0x80000150u, tbl);
    PE_StoreU32(0x80000154u, 0xA0u);             /* two 0x50-byte entries */
    PE_StoreU32(tbl + 0x00u, 0u);                /* skipped (null name) */
    PE_StoreU32(tbl + 0x50u, PS_S + 0x300u);
    PE_StoreU32(tbl + 0x50u + 0x34u, 0x80012340u);
    /* the device name must equal the "c" prefix for the real BIOS A(17h)
     * strcmp (pe_bios_string.c) to match */
    PE_StoreU8(PS_S + 0x300u, 'c'); PE_StoreU8(PS_S + 0x301u, 0u);
    (void)func_800727B4(name, 9);
    ASSERT(PE_LoadU32(0x800A32D0u) == 0x80012340u, "old hook saved");
    ASSERT(PE_LoadU32(tbl + 0x84u) == 0x80072950u, "hook patched");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == 0x80072A64u &&
           g_bootstrap_arg4_calls[0].arg0 == name &&
           g_bootstrap_arg4_calls[0].arg1 == 9u, "real compares, then open(name, arg)");

    /* func_80072950: *a0 0 -> 1, hook restored, saved hook called. */
    PS_Reset();
    PE_StoreU32(0x80000150u, tbl);
    PE_StoreU32(0x80000154u, 0x50u);
    PE_StoreU32(tbl, PS_S + 0x300u);
    PE_StoreU8(PS_S + 0x300u, 'c'); PE_StoreU8(PS_S + 0x301u, 0u);
    PE_StoreU8(0x800A32D8u, 'c'); PE_StoreU8(0x800A32D9u, 0u);
    PE_StoreU32(tbl + 0x34u, 0x80072950u);
    PE_StoreU32(0x800A32D0u, 0x80012340u);
    PE_StoreU32(PS_S + 0x500u, 0u);
    (void)func_80072950(PS_S + 0x500u, 4, 5);
    ASSERT(PE_LoadU32(PS_S + 0x500u) == 1u, "zero word forced to 1");
    ASSERT(PE_LoadU32(tbl + 0x34u) == 0x80012340u, "saved hook restored");
    ASSERT(PS_HasBoundary("func_80072950_saved_device_hook") &&
           g_bootstrap_arg4_calls[g_bootstrap_arg4_call_count - 1].target ==
               0x80012340u &&
           g_bootstrap_arg4_calls[g_bootstrap_arg4_call_count - 1].arg0 ==
               PS_S + 0x500u &&
           g_bootstrap_arg4_calls[g_bootstrap_arg4_call_count - 1].arg2 == 5u,
           "tail call through the saved hook with (a0,a1,a2)");
    PASS();
}

static void test_PA_sdk_jtb_slots(void)
{
    /* StopCallback / RestartCallback (0x80073D88 / 0x80073DB8) and their
     * jump-table targets are the clean-room pe_plat_stop/restart_callback.
     * They drive the registers directly: no code rewrites the I_STAT/I_MASK/
     * DPCR pointer words or the static jump-table slots +0x10/+0x18, so the
     * old RAM stand-ins and unknown-slot stop no longer apply
     * (docs/evidence/native-irq-callback-20261030). */
    TEST("PA_sdk_jtb_slots");
    PS_Reset();

    /* Restart while inactive: activate, restore mask/DPCR. */
    PE_StoreU16(0x800945E4u + 0x32u, 0x0009u);
    PE_StoreU32(0x800945E4u + 0x34u, 0x12345678u);
    ASSERT(func_80073DB8() == 0x800945E4u, "restart returns the state block");
    ASSERT(PE_LoadU16(0x800945E4u) == 1u, "state activated");
    ASSERT(PE_IRQ_GetMask() == 0x0009u && PE_GPU_ReadDPCR() == 0x12345678u,
           "mask and DPCR restored");

    /* Stop while active: save, clear, deactivate. */
    PE_GPU_WriteDPCR(0xFFFFFFFFu);
    ASSERT(func_80073D88() == 0x800945E4u, "stop returns the state block");
    ASSERT(PE_LoadU16(0x800945E4u) == 0u, "state deactivated");
    ASSERT(PE_LoadU16(0x800945E4u + 0x32u) == 0x0009u &&
           PE_LoadU32(0x800945E4u + 0x34u) == 0xFFFFFFFFu, "mask and DPCR saved");
    ASSERT(PE_IRQ_GetMask() == 0u && PE_GPU_ReadDPCR() == 0x77777777u, "DPCR masked");
    ASSERT(func_80073D88() == 0u, "second stop is inert");
    PASS();
}

static void test_PA_sdk_80073E28(void)
{
    TEST("PA_sdk_80073E28");
    PS_Reset();
    PE_StoreU16(0x800945E4u, 1u);
    ASSERT(func_80073E28() == 0u, "already reset -> 0");

    PS_Reset();
    PE_StoreU32(0x8009566Cu, 0x8009564Cu);
    PE_StoreU32(0x80095670u, 0x1F801070u);
    PE_StoreU32(0x80095674u, 0x1F801074u);
    PE_StoreU32(0x80095678u, 0x1F8010F0u);
    PE_StoreU32(0x800956B0u, 0x1F801114u);
    PE_StoreU32(0x800956BCu, 0x1F8010F4u);
    (void)PE_IRQ_ExchangeMask(0x0040u);
    PE_StoreU32(0x80094700u, 0xDEADBEEFu);       /* inside the 0x41A-word block */
    PE_StoreU32(0x8009568Cu + 8u, 0x80011111u);  /* a VBlank slot */
    PE_StoreU32(0x800956C0u + 4u, 0x80011111u);  /* a DMA slot */
    PE_StoreU32(0x800956ACu, 7u);

    ASSERT(func_80073E28() == 0x800945E4u, "returns the state block");
    ASSERT(PE_LoadU16(0x800945E4u) == 1u, "guard published");
    ASSERT(PE_LoadU32(0x80094700u) == 0u, "block cleared");
    /* The setjmp/HookEntryInt exception context at 0x8009461C is a BIOS
     * exception path the host does not have; the clean-room reset leaves it
     * alone (docs/evidence/native-irq-callback-20261030). */
    ASSERT(PE_GPU_ReadDPCR() == 0x33333333u, "DPCR");
    ASSERT(PE_LoadU32(0x80095660u) == 0x80074478u &&
           PE_LoadU32(0x80095650u) == 0x800746A0u, "jtb[5], jtb[1]");
    ASSERT(PE_LoadU32(0x800945E8u) == 0x8007440Cu &&
           PE_LoadU32(0x800945F4u) == 0x80074520u, "sources 0 and 3 registered");
    ASSERT(PE_IRQ_GetMask() == 0x0009u && PE_LoadU16(0x80094614u) == 0x0009u,
           "I_MASK cleared then sources 0|3");
    ASSERT(PE_LoadU32(0x8009568Cu + 8u) == 0u && PE_LoadU32(0x800956C0u + 4u) == 0u &&
           PE_LoadU32(0x800956ACu) == 0u, "slot tables and counter cleared");
    ASSERT(PE_Timer1_ReadMode() == 0x507u, "timer 1 mode 0x107 written");
    /* setjmp A(13h), HookEntryInt B(19h) and CdRemove A(72h) have no host
     * counterpart: the clean-room reset never reaches their veneers. */
    ASSERT(!PS_HasBoundary("func_80074354") && !PS_HasBoundary("func_800743A4") &&
           !PS_HasBoundary("func_8007436C"), "collapsed BIOS services not reached");
    PASS();
}

static void PS_SeedIrqPointers(void)
{
    PE_StoreU32(0x80095670u, 0x1F801070u);
    PE_StoreU32(0x80095674u, 0x1F801074u);
}

static void test_PA_sdk_80073F00(void)
{
    TEST("PA_sdk_80073F00");
    PS_Reset();
    PS_SeedIrqPointers();
    PE_StoreU16(0x800945E4u, 1u);
    PE_StoreU16(0x80094614u, 0x0001u);
    PE_StoreU32(0x800945E8u, 0x80012345u);      /* source 0 handler */
    PE_StoreU32(0x8009567Cu, 5u);
    (void)PE_IRQ_ExchangeMask(0x0001u);
    PE_IRQ_AssertSources(0x0001u);
    func_80073F00();
    /* 0x80073F00 is the clean-room pe_plat_irq_dispatch: the source is
     * acknowledged before its handler runs; an unresolved handler is a loud
     * stop (the dispatch returns at once, leaving the active flag set).
     * ReturnFromException B(17h) has no host counterpart and is not reached. */
    ASSERT(PE_IRQ_ReadStatus() == 0u, "source acknowledged (W0C)");
    ASSERT(PE_Port_ShouldStop() && PE_LoadU16(0x800945E6u) == 1u,
           "unresolved handler stops the dispatch");
    ASSERT(!PS_HasBoundary("func_80074384"), "ReturnFromException not reached");

    /* Unregistered pending source: watchdog counts, clears past 0x800. */
    PS_Reset();
    PS_SeedIrqPointers();
    PE_StoreU16(0x800945E4u, 1u);
    (void)PE_IRQ_ExchangeMask(0x0004u);
    PE_IRQ_AssertSources(0x0004u);
    PE_StoreU32(0x8009567Cu, 0x800u);
    func_80073F00();
    ASSERT(PE_LoadU32(0x8009567Cu) == 0x801u && PE_IRQ_ReadStatus() == 4u,
           "0x800 -> 0x801, no clear");
    func_80073F00();
    ASSERT(PE_LoadU32(0x8009567Cu) == 0u && PE_IRQ_ReadStatus() == 0u,
           "0x801 -> print, clear watchdog and I_STAT");
    {
        pe_addr_t f; uint32_t a[4];
        ASSERT(PE_Bios_PrintfLog(&f, a) == 1 && f == 0x8001175Cu && a[0] == 4u && a[1] == 4u,
               "watchdog print(I_STAT, I_MASK)");
    }

    /* Unset guard: diagnostic print of I_STAT. */
    PS_Reset();
    PS_SeedIrqPointers();
    PE_IRQ_AssertSources(0x0002u);
    func_80073F00();
    {
        pe_addr_t f; uint32_t a[4];
        ASSERT(PE_Bios_PrintfLog(&f, a) == 1 && f == 0x80011740u && a[0] == 2u,
               "unexpected-interrupt print");
    }
    PASS();
}

static void test_PA_sdk_callback_init(void)
{
    TEST("PA_sdk_callback_init");
    PS_Reset();
    PE_StoreU16(0x800945E4u, 1u);               /* registration enabled */
    PE_StoreU32(0x800956B0u, 0x1F801114u);
    PE_StoreU32(0x800956BCu, 0x1F8010F4u);
    PE_StoreU32(0x800956ACu, 9u);
    PE_StoreU32(0x8009568Cu + 0x1Cu, 0x80011111u);
    ASSERT(func_800743B4() == 0x80074478u, "returns func_80074478");
    ASSERT(PE_LoadU32(0x800956ACu) == 0u && PE_LoadU32(0x8009568Cu + 0x1Cu) == 0u,
           "counter and 8 slots cleared");
    ASSERT(PE_LoadU32(0x800945E8u) == 0x8007440Cu, "source 0 = func_8007440C");
    ASSERT(PE_Timer1_ReadMode() == 0x507u, "timer 1 mode");

    PE_StoreU32(0x800956C0u + 0x1Cu, 0x80011111u);
    ASSERT(func_800744D4() == 0x800746A0u, "returns func_800746A0");
    ASSERT(PE_LoadU32(0x800956C0u + 0x1Cu) == 0u, "8 DMA slots cleared");
    ASSERT(PE_LoadU32(0x800945F4u) == 0x80074520u, "source 3 = func_80074520");

    PE_StoreU32(0x800956ACu, 3u);
    func_8007440C();
    ASSERT(PE_LoadU32(0x800956ACu) == 4u, "dispatch counter incremented");
    PASS();
}

static void test_PA_sdk_80075B1C(void)
{
    /* GetODE is an independent host model of psx-spx GPUSTAT bit 31
     * ("drawing even/odd lines in interlace mode"), read from the GPU model
     * rather than through the libgpu jump table.  The model does not yet
     * produce odd interlaced fields, so both states read 0; the contract
     * checked is equality with GPUSTAT >> 31. */
    TEST("PA_sdk_80075B1C");
    PS_Reset();
    ASSERT(func_80075B1C() == (PE_GPU_ReadStatus() >> 31), "GetODE == GPUSTAT bit 31");
    ASSERT(PE_GPU_WriteGP1(0x08000024u), "interlace display mode");
    ASSERT(func_80075B1C() == (PE_GPU_ReadStatus() >> 31), "GetODE == GPUSTAT bit 31 (interlaced)");
    PASS();
}

static void test_PA_sdk_seq_ops(void)
{
    const pe_addr_t rec = PS_S + 0xC00u, data = PS_S + 0xE00u;

    TEST("PA_sdk_seq_ops");
    PS_Reset();
    PE_StoreU8(data + 0u, 0u);
    PE_StoreU8(data + 1u, 7u);
    PE_StoreU32(rec, data);
    /* the generated func_8009019C/800902AC translate D_8009D2C8 on entry */
    PE_StoreU32(0x8009D2C8u, PS_S + 0xF00u);
    PE_StoreU16(rec + 0x54u, 1u);                /* -> D_800BCD6C |= a1 */
    func_80090AEC(rec, 0x40u);
    ASSERT(PE_LoadU32(rec) == data + 1u, "cursor advanced");
    ASSERT(PE_LoadU16(rec + 0xBAu) == 0x101u, "zero byte -> 0x101");
    ASSERT(PE_LoadU32(0x800BCD6Cu) == 0x40u, "live a1 forwarded");
    ASSERT(PE_LoadU32(0x8009D2C4u) == 0x110u, "func_8009019C flags");

    PE_StoreU16(rec + 0x54u, 0u);
    func_80090B5C(rec, 0x8u);
    ASSERT(PE_LoadU32(rec) == data + 2u, "cursor advanced again");
    ASSERT(PE_LoadU16(rec + 0xBCu) == 8u, "byte 7 -> 8");
    ASSERT(PE_LoadU32(PS_S + 0xF00u + 15u * 4u) == 0x8u,
           "func_800902AC: D_8009D2C8[15] |= a1");
    PASS();
}

static void test_PA_sdk_all(void)
{
    test_PA_sdk_80038954();
    test_PA_sdk_8005CAEC();
    test_PA_sdk_device_hook();
    test_PA_sdk_jtb_slots();
    test_PA_sdk_80073E28();
    test_PA_sdk_80073F00();
    test_PA_sdk_callback_init();
    test_PA_sdk_80075B1C();
    test_PA_sdk_seq_ops();
}
