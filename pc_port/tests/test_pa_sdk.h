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
}

static void test_PA_sdk_80038954(void)
{
    TEST("PA_sdk_80038954");
    PS_Reset();
    PE_StoreU32(0x80091A2Cu + 4u, 0x80010F00u);    /* D_80091A2C[1] */
    func_80038954(PS_S, PS_S + 0x10u, PS_S + 0x20u, 0x101); /* (u8)a3 == 1 */
    ASSERT(g_bootstrap_arg4_call_count == 2, "two printf calls");
    ASSERT(g_bootstrap_arg4_calls[0].target == 0x80071A74u &&
           g_bootstrap_arg4_calls[0].arg0 == 0x80010EB0u &&
           g_bootstrap_arg4_calls[0].arg1 == 0x80010F00u &&
           g_bootstrap_arg4_calls[0].arg2 == PS_S, "first print args");
    ASSERT(g_bootstrap_arg4_calls[1].arg0 == 0x80010EC8u &&
           g_bootstrap_arg4_calls[1].arg1 == PS_S + 0x10u &&
           g_bootstrap_arg4_calls[1].arg2 == PS_S + 0x20u, "second print args");
    ASSERT(PE_Port_ShouldStop(), "low byte 1 is the retail halt");

    PS_Reset();
    func_80038954(PS_S, 0, 0, 2);
    ASSERT(!PE_Port_ShouldStop() && g_bootstrap_arg4_call_count == 2,
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
    TEST("PA_sdk_jtb_slots");
    PS_Reset();
    PE_StoreU32(0x8009566Cu, 0x8009564Cu);
    PE_StoreU32(0x8009565Cu, 0x80074218u);       /* +0x10 */
    PE_StoreU32(0x80095664u, 0x800742B8u);       /* +0x18 */
    PE_StoreU32(0x80095670u, PS_S + 0x10u);      /* RAM stand-ins for the */
    PE_StoreU32(0x80095674u, PS_S + 0x14u);      /* I_STAT/I_MASK/DPCR    */
    PE_StoreU32(0x80095678u, PS_S + 0x18u);      /* pointer globals       */

    /* +0x18 = func_800742B8: inactive -> activate, restore mask/DPCR. */
    PE_StoreU16(0x800945E4u + 0x32u, 0x0009u);
    PE_StoreU32(0x800945E4u + 0x34u, 0x12345678u);
    func_80073DB8();
    ASSERT(PE_LoadU16(0x800945E4u) == 1u, "state activated");
    ASSERT(PE_LoadU16(PS_S + 0x14u) == 0x0009u &&
           PE_LoadU32(PS_S + 0x18u) == 0x12345678u, "mask and DPCR restored");
    ASSERT(PS_HasBoundary("func_800743A4"), "HookEntryInt boundary");

    /* +0x10 = func_80074218: active -> saved and deactivated. */
    PE_StoreU32(PS_S + 0x18u, 0xFFFFFFFFu);
    func_80073D88();
    ASSERT(PE_LoadU16(0x800945E4u) == 0u, "state deactivated");
    ASSERT(PE_LoadU32(PS_S + 0x18u) == 0x77777777u, "DPCR masked");

    /* An unproven identity is a loud boundary. */
    PS_Reset();
    PE_StoreU32(0x8009566Cu, 0x8009564Cu);
    PE_StoreU32(0x8009565Cu, 0x80011111u);
    func_80073D88();
    ASSERT(PS_HasBoundary("func_80073D88_jtb_0x10"), "unknown slot boundary");
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
    ASSERT(PE_LoadU32(0x80094620u) == 0x80094620u + 0xFDCu, "stack top word");
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
    ASSERT(PS_HasBoundary("func_80074354") && PS_HasBoundary("func_800743A4") &&
           PS_HasBoundary("func_8007436C"), "BIOS boundaries");
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
    ASSERT(PE_IRQ_ReadStatus() == 0u, "source acknowledged (W0C)");
    ASSERT(PS_HasBoundary("func_80073F00_cpu_source") &&
           g_bootstrap_arg4_call_count == 0, "zero-argument handler call");
    ASSERT(PE_LoadU32(0x8009567Cu) == 0u, "no pending -> watchdog cleared");
    ASSERT(PE_LoadU16(0x800945E6u) == 0u, "active cleared");
    ASSERT(PS_HasBoundary("func_80074384"), "ReturnFromException");

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
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].arg0 == 0x8001175Cu &&
           g_bootstrap_arg4_calls[0].arg1 == 4u &&
           g_bootstrap_arg4_calls[0].arg2 == 4u, "watchdog print(I_STAT, I_MASK)");

    /* Unset guard: diagnostic print of I_STAT. */
    PS_Reset();
    PS_SeedIrqPointers();
    PE_IRQ_AssertSources(0x0002u);
    func_80073F00();
    ASSERT(g_bootstrap_arg4_calls[0].arg0 == 0x80011740u &&
           g_bootstrap_arg4_calls[0].arg1 == 2u, "unexpected-interrupt print");
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

static void test_PA_sdk_gpu_setters(void)
{
    const pe_addr_t jtb = PS_S + 0x600u;

    TEST("PA_sdk_gpu_setters");
    PS_Reset();
    PE_StoreU32(0x80095744u, jtb);
    PE_StoreU32(jtb + 13u * 4u, 0x80011110u);
    PE_StoreU32(0x80095748u, 0x80071A74u);
    PE_StoreU8(0x8009574Du, 3u);
    PE_StoreU8(0x8009574Eu, 2u);                 /* debug level 2 */
    ASSERT(func_80074C14(5) == 3, "returns the old byte");
    ASSERT(PE_LoadU8(0x8009574Du) == 5u, "new value stored");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           g_bootstrap_arg4_calls[0].target == 0x80071A74u &&
           g_bootstrap_arg4_calls[0].arg0 == 0x80011840u &&
           g_bootstrap_arg4_calls[0].arg1 == 5u &&
           g_bootstrap_arg4_calls[1].target == 0x80011110u &&
           g_bootstrap_arg4_calls[1].arg0 == 1u, "print, then jtb[13](1)");
    Bootstrap_ResetArg4CallLog();
    PE_StoreU8(0x8009574Eu, 1u);
    ASSERT(func_80074C14(5) == 5 && g_bootstrap_arg4_call_count == 0,
           "unchanged value: no print (level 1), no jtb call");

    PS_Reset();
    PE_StoreU32(0x80095758u, 0x1234u);
    ASSERT(func_80074CC8(0x55) == 0x1234 && PE_LoadU32(0x80095758u) == 0x55u,
           "D_80095758 exchanged");
    PASS();
}

static void test_PA_sdk_80074FD4(void)
{
    const pe_addr_t jtb = PS_S + 0x600u, rect = PS_S + 0x700u;

    TEST("PA_sdk_80074FD4");
    PS_Reset();
    PE_StoreU32(0x80095744u, jtb);
    PE_StoreU32(jtb + 8u, 0x80012222u);          /* unproven jtb[2] */
    PE_StoreU32(jtb + 12u, 0x80076434u);
    func_80074FD4((int)rect, 0x11, 0x22, 0x33);
    ASSERT(PS_HasBoundary("func_80074FD4_jtb2") &&
           g_bootstrap_arg4_calls[0].target == 0x80012222u &&
           g_bootstrap_arg4_calls[0].arg0 == 0x80076434u &&
           g_bootstrap_arg4_calls[0].arg1 == rect &&
           g_bootstrap_arg4_calls[0].arg2 == 8u &&
           g_bootstrap_arg4_calls[0].arg3 == 0x80332211u,
           "jtb[2](jtb[3], rect, 8, 0x80|b3|b2|b1)");
    PASS();
}

static void test_PA_sdk_800751E4(void)
{
    const pe_addr_t ot = PS_S + 0x800u;

    TEST("PA_sdk_800751E4");
    PS_Reset();
    PE_StoreU32(ot + 0u, 0xABCDEF01u);
    PE_StoreU32(ot + 4u, 0xABCDEF01u);
    PE_StoreU32(ot + 8u, 0xABCDEF01u);
    ASSERT(func_800751E4(ot, 3) == ot + 8u, "returns the last word");
    ASSERT(PE_LoadU32(ot + 0u) == ((ot + 4u) & 0xFFFFFFu), "byte 3 zeroed, link");
    ASSERT(PE_LoadU32(ot + 4u) == ((ot + 8u) & 0xFFFFFFu), "second link");
    ASSERT(PE_LoadU32(ot + 8u) == 0x0009580Cu, "terminator -> D_8009580C");
    ASSERT(PE_LoadU32(0x8009580Cu) == 0x040957F8u, "D_8009580C seed");
    ASSERT(g_bootstrap_arg4_call_count == 0, "no print at level 0");

    PS_Reset();
    PE_StoreU8(0x8009574Eu, 2u);
    PE_StoreU32(0x80095748u, 0x80071A74u);
    PE_StoreU32(ot, 0xFFFFFFFFu);
    ASSERT(func_800751E4(ot, 1) == ot && PE_LoadU32(ot) == 0x0009580Cu,
           "count 1: terminator only");
    ASSERT(g_bootstrap_arg4_calls[0].arg0 == 0x800118F8u &&
           g_bootstrap_arg4_calls[0].arg1 == ot &&
           g_bootstrap_arg4_calls[0].arg2 == 1u, "debug print (fmt, a0, a1)");
    PASS();
}

static void test_PA_sdk_80075B1C(void)
{
    const pe_addr_t jtb = PS_S + 0x600u;

    TEST("PA_sdk_80075B1C");
    PS_Reset();
    PE_StoreU32(0x80095744u, jtb);
    PE_StoreU32(jtb + 0x38u, 0x8007633Cu);       /* jtb[14] */
    PE_StoreU32(0x80095854u, PS_S + 0x900u);     /* func_8007633C: *D_80095854 */
    PE_StoreU32(PS_S + 0x900u, 0x80000001u);
    ASSERT(func_80075B1C() == 1u, "sign bit of jtb[14]()");
    PE_StoreU32(PS_S + 0x900u, 0x7FFFFFFFu);
    ASSERT(func_80075B1C() == 0u, "clear sign bit");
    PASS();
}

static void test_PA_sdk_80075CE8(void)
{
    const pe_addr_t dr = PS_S + 0xA00u, env = PS_S + 0xB00u;

    TEST("PA_sdk_80075CE8");
    PS_Reset();
    PE_StoreU16(0x80095750u, 0x400u);
    PE_StoreU16(0x80095752u, 0x200u);
    PE_StoreU16(env + 0u, 10u);  PE_StoreU16(env + 2u, 20u);
    PE_StoreU16(env + 4u, 100u); PE_StoreU16(env + 6u, 50u);
    PE_StoreU16(env + 8u, 3u);   PE_StoreU16(env + 0xAu, 4u);
    PE_StoreU16(env + 0xCu, 8u); PE_StoreU16(env + 0xEu, 16u);
    PE_StoreU16(env + 0x10u, 8u); PE_StoreU16(env + 0x12u, 8u);
    PE_StoreU16(env + 0x14u, 0x1234u);
    PE_StoreU8(env + 0x16u, 1u); PE_StoreU8(env + 0x17u, 0u);
    PE_StoreU8(env + 0x18u, 1u);
    PE_StoreU8(env + 0x19u, 0x11u); PE_StoreU8(env + 0x1Au, 0x22u);
    PE_StoreU8(env + 0x1Bu, 0x33u);
    PE_StoreU32(dr, 0x00ABCDEFu);
    func_80075CE8(dr, env);
    ASSERT(PE_LoadU32(dr) == 0x09ABCDEFu, "length 9 in byte 3 only");
    ASSERT(PE_LoadU32(dr + 4u) == 0xE300500Au, "draw area top-left");
    ASSERT(PE_LoadU32(dr + 8u) == 0xE401146Du, "draw area bottom-right (109,69)");
    ASSERT(PE_LoadU32(dr + 12u) == 0xE5002003u, "offset (3,4)");
    ASSERT(PE_LoadU32(dr + 16u) == 0xE1000234u, "texpage: dtd 0x200, tpage&0x9FF");
    ASSERT(PE_LoadU32(dr + 20u) == 0xE20107FFu, "texture window");
    ASSERT(PE_LoadU32(dr + 24u) == 0xE6000000u, "mask word");
    ASSERT(PE_LoadU32(dr + 28u) == 0x60332211u, "fill rect command");
    ASSERT(PE_LoadU32(dr + 32u) == 0x00100007u, "rect x-ox, y-oy");
    ASSERT(PE_LoadU32(dr + 36u) == 0x00320064u, "rect w, h");

    /* clamp: w beyond the display -> limit-1; negative h -> 0 */
    PE_StoreU16(env + 4u, 0x500u);
    PE_StoreU16(env + 6u, 0x8000u);
    func_80075CE8(dr, env);
    ASSERT(PE_LoadU32(dr + 36u) == 0x000003FFu, "clamped w, zero h");

    /* no background: 7 words, length 6 */
    PE_StoreU8(env + 0x18u, 0u);
    PE_StoreU32(dr + 28u, 0xCAFEF00Du);
    func_80075CE8(dr, env);
    ASSERT(PE_LoadU8(dr + 3u) == 6u && PE_LoadU32(dr + 28u) == 0xCAFEF00Du,
           "no fill words");
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
    test_PA_sdk_gpu_setters();
    test_PA_sdk_80074FD4();
    test_PA_sdk_800751E4();
    test_PA_sdk_80075B1C();
    test_PA_sdk_80075CE8();
    test_PA_sdk_seq_ops();
}
