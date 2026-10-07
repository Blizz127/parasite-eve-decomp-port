/*
 * port_absent lane round 2 (2026-09-24) — tests for the hi2 group
 * (pc_port/game/decomp_hand/absent_hi2_port.c).  Expectations are computed
 * from the matched leaf src/func_XXXXXXXX.c named in each block.
 * Guest scratch 0x80168000..0x8016BFFF.
 */
#define PA_HI2 0x80168000u

/* src/func_800878F0.c: flag groups, early return, final clear. */
static void test_PA_hi2_voice_regs(void)
{
    TEST("PA_hi2_800878F0_voice_regs");
    ResetTestState();
    const pe_addr_t r = PA_HI2;

    /* flags 0: nothing written. */
    PE_StoreU32(r + 4u, 0u);
    PE_SpuRegister_StoreU16(0x34u, 0x1234u);
    func_800878F0(3, r);
    ASSERT(PE_SpuRegister_LoadU16(0x34u) == 0x1234u, "878F0 flags 0 is a no-op");

    /* only 0x10: pitch (+0x1C -> reg +4), flags cleared, early return
     * leaves the 0x9000 group untouched. */
    PE_StoreU32(r + 4u, 0x10u);
    PE_StoreU16(r + 0x1Cu, 0x0ABCu);
    PE_SpuRegister_StoreU16(0x38u, 0xFFFFu);
    func_800878F0(3, r);
    ASSERT(PE_SpuRegister_LoadU16(0x34u) == 0x0ABCu, "878F0 0x10 -> voice pitch");
    ASSERT(PE_LoadU32(r + 4u) == 0u, "878F0 0x10 flag cleared");
    ASSERT(PE_SpuRegister_LoadU16(0x38u) == 0xFFFFu, "878F0 returned early");

    /* 3: signed volume pair (+0x28/+0x2A, masked 0x7FFF). */
    PE_StoreU32(r + 4u, 3u | 0x80u);
    PE_StoreU16(r + 0x28u, 0x8123u);
    PE_StoreU16(r + 0x2Au, 0x0456u);
    PE_StoreU32(r + 8u, 0x80u);          /* start address -> reg +6 = 0x10 */
    func_800878F0(2, r);
    ASSERT(PE_SpuRegister_LoadU16(0x20u) == 0x0123u &&
           PE_SpuRegister_LoadU16(0x22u) == 0x0456u, "878F0 volume pair");
    ASSERT(PE_SpuRegister_LoadU16(0x26u) == 0x10u, "878F0 0x80 -> addr >> 3");
    ASSERT(PE_LoadU32(r + 4u) == 0u, "878F0 both groups cleared");

    /* 0x9000 only: +0x20 into bits 4..7, +0x22 into bits 0..3 of reg +8,
     * then flags = 0. */
    PE_StoreU32(r + 4u, 0x1000u);
    PE_StoreU16(r + 0x20u, 0x5u);
    PE_StoreU16(r + 0x22u, 0x9u);
    PE_SpuRegister_StoreU16(0x18u, 0xAB00u);
    func_800878F0(1, r);
    ASSERT(PE_SpuRegister_LoadU16(0x18u) == 0xAB59u, "878F0 0x9000 group");
    ASSERT(PE_LoadU32(r + 4u) == 0u, "878F0 final flags = 0");
    PASS();
}

/* src/func_8008900C.c: voice claim paths. */
static void test_PA_hi2_voice_claim(void)
{
    TEST("PA_hi2_8008900C_voice_claim");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    const pe_addr_t v = PA_HI2 + 0x1000u, trk = PA_HI2 + 0x3000u,
                    out = PA_HI2 + 0x3100u;
    unsigned k;

    PE_StoreU32(0x8009D2C8u, trk);
    PE_StoreU32(trk + 0x10u, 0x3u);           /* bits 0,1 claimable */
    PE_StoreU32(0x8009D2B8u, 0x2u);            /* bit 1 clears +0x118/+0x11A */
    for (k = 0; k < 24; k++)
        PE_StoreU16(0x800B002Cu + k * 8u, k < 5 ? 1u : 0u);   /* slot 5 free */
    /* voice 0: live flags, a2 bit 0 set -> takes voice i = 0. */
    PE_StoreU32(v + 0xF4u, 1u);
    /* voice 1: live flags, a2 bit 1 clear -> claims free slot 5. */
    PE_StoreU32(v + 0x11Cu + 0xF4u, 1u);
    PE_StoreU16(v + 0x11Cu + 0x118u, 7u);
    PE_StoreU16(v + 0x11Cu + 0x11Au, 7u);
    /* voice 2: flags 0 -> only the boundary runs. */
    PE_StoreU32(v + 2u * 0x11Cu + 0xF4u, 0u);
    PE_StoreU32(v + 2u * 0x11Cu + 0xF0u, 0x55u);
    PE_StoreU32(out, 0u);

    func_8008900C(v, 0x7u, 0x1u, out);
    /* func_80088344 is a real port now (port7_port.c; retail oracle case
     * set P7 entry 12): the claim loop runs without a boundary. */
    ASSERT(PE_Decomp_BoundaryCount() == 0 && !PE_Port_ShouldStop(),
           "8900C runs func_80088344 natively");
    ASSERT(PE_LoadU32(v + 0xF0u) == 0u, "8900C voice 0 takes voice 0");
    ASSERT(PE_LoadU32(v + 0x11Cu + 0xF0u) == 5u, "8900C voice 1 claims slot 5");
    ASSERT(PE_LoadU16(0x800B002Cu + 5u * 8u) == 0x7FFFu, "8900C slot marked 0x7FFF");
    ASSERT((PE_LoadU32(0x8009D2C4u) & 0x100u) != 0, "8900C D_8009D2C4 |= 0x100");
    ASSERT(PE_LoadU32(out) == ((1u << 0) | (1u << 5)), "8900C *a3 bits");
    ASSERT(PE_LoadU16(v + 0x11Cu + 0x118u) == 0u &&
           PE_LoadU16(v + 0x11Cu + 0x11Au) == 0u, "8900C D_8009D2B8 bit clears");
    ASSERT(PE_LoadU32(v + 2u * 0x11Cu + 0xF0u) == 0x55u, "8900C voice 2 untouched");
    /* func_800878F0 ran on voice 0/1 regs (+0xF0 record): flags +0xF4 had
     * 0x1FF93 OR-ed in and the programming loop ends with flags = 0. */
    ASSERT(PE_LoadU32(v + 0xF4u) == 0u && PE_LoadU32(v + 0x11Cu + 0xF4u) == 0u,
           "8900C voices programmed via func_800878F0");

    /* No free slot: +0xF0 = 24 and *D_8009D2C8 |= 1. */
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    PE_StoreU32(0x8009D2C8u, trk);
    PE_StoreU32(trk, 0x10u);
    PE_StoreU32(trk + 0x10u, 0x1u);
    for (k = 0; k < 24; k++)
        PE_StoreU16(0x800B002Cu + k * 8u, 1u);
    PE_StoreU32(v + 0xF4u, 1u);
    PE_StoreU32(out, 0u);
    func_8008900C(v, 0x1u, 0u, out);
    ASSERT(PE_LoadU32(v + 0xF0u) == 24u && PE_LoadU32(trk) == 0x11u,
           "8900C no free slot");
    ASSERT(PE_LoadU32(v + 0xF4u) == 1u && PE_LoadU32(out) == 0u,
           "8900C no claim, no programming (voice >= 24)");
    PASS();
}

/* src/func_80089784.c / 80089980 / 80089B48 / 80089D10. */
static void hi2_seed_banks(pe_addr_t trk)
{
    PE_StoreU32(0x8009D2C8u, trk);
    PE_StoreU32(0x800BCD50u, 0x8u);        /* voice 3 masked out */
    PE_StoreU32(0x800BCD60u, 0u);
    /* bank-1 records D_800BA560: record k's +0xF0 = voice 10 + k */
    for (unsigned k = 0; k < 4; k++) {
        PE_StoreU32(0x800BA560u + k * 0x11Cu + 0xF0u, 10u + k);
        PE_StoreU32(0x800B8AC0u + k * 0x11Cu + 0xF0u, k);
    }
}

static void test_PA_hi2_collectors(void)
{
    TEST("PA_hi2_89784_family_collectors");
    ResetTestState();
    const pe_addr_t trk = PA_HI2 + 0x2000u, b1 = trk + 0x68u;

    /* func_80089784: bank 1 selects f6C & f80 (+0x80), with the f70 part
     * first; bank 0 selects f4 & f18. */
    hi2_seed_banks(trk);
    PE_StoreU32(trk + 0x6Cu, 0x3u);
    PE_StoreU32(trk + 0x80u, 0x3u);
    PE_StoreU32(trk + 0x70u, 0x1u);          /* bank-1 bit 0 first */
    PE_StoreU32(b1 + 0x8u, 0x1u);
    PE_StoreU32(b1 + 0x18u, 0xFu);
    PE_StoreU32(trk + 0x4u, 0xCu);
    PE_StoreU32(trk + 0x18u, 0xFu);
    PE_StoreU32(trk + 0x8u, 0u);
    PE_StoreU32(0x800BCD5Cu, 0x100000u);
    func_80089784();
    ASSERT(PE_LoadU32(0x8009D2C8u) == trk, "89784 restores D_8009D2C8");
    ASSERT(PE_LoadU32(0x800BCD5Cu) == 0u, "89784 consumes D_800BCD5C");
    /* acc = voices 10, 11 (bank 1), 2 (bank 0; voice 3 masked) | 0x100000 */
    ASSERT(PE_LoadU32(PE_HAND_HI2_STACK) == ((1u << 10) | (1u << 11) | (1u << 2) | 0x100000u),
           "89784 key-off mask");
    ASSERT(PE_SpuRegister_LoadU16(0x18Cu) == (uint16_t)((1u << 10) | (1u << 11) | (1u << 2)) &&
           PE_SpuRegister_LoadU16(0x18Eu) == 0x10u, "89784 func_80087728(acc)");
    ASSERT(PE_LoadU32(b1 + 0x18u) == 0u && PE_LoadU32(trk + 0x18u) == 0u,
           "89784 clears both banks' +0x18");

    /* func_80089980: +0x9C / +0x34, D_800C0DD4 = acc | D_800BCD6C. */
    ResetTestState();
    hi2_seed_banks(trk);
    PE_StoreU32(trk + 0x6Cu, 0x1u);
    PE_StoreU32(trk + 0x9Cu, 0x1u);
    PE_StoreU32(trk + 0x4u, 0x1u);
    PE_StoreU32(trk + 0x34u, 0x1u);
    PE_StoreU32(0x800BCD6Cu, 0x800000u);
    func_80089980();
    ASSERT(PE_LoadU32(0x800C0DD4u) == ((1u << 10) | (1u << 0) | 0x800000u),
           "89980 D_800C0DD4");
    ASSERT((PE_LoadU32(0x8009D2C4u) & 0x100u) != 0, "89980 D_8009D2C4 |= 0x100");
    ASSERT(PE_LoadU32(0x8009D2C8u) == trk, "89980 restores D_8009D2C8");

    /* func_80089B48: +0xA0 / +0x38 -> D_800C0DD0 | D_800BCD70. */
    ResetTestState();
    hi2_seed_banks(trk);
    PE_StoreU32(trk + 0x6Cu, 0x2u);
    PE_StoreU32(trk + 0xA0u, 0x2u);
    PE_StoreU32(trk + 0x9Cu, 0x1u);          /* wrong select word: ignored */
    PE_StoreU32(0x800BCD70u, 0x1000000u);
    func_80089B48();
    ASSERT(PE_LoadU32(0x800C0DD0u) == ((1u << 11) | 0x1000000u), "89B48 D_800C0DD0");

    /* func_80089D10: +0xA4 / +0x3C -> D_800C0DD8 | D_800BCD74. */
    ResetTestState();
    hi2_seed_banks(trk);
    PE_StoreU32(trk + 0x4u, 0x4u);
    PE_StoreU32(trk + 0x3Cu, 0x4u);
    PE_StoreU32(0x800BCD74u, 0u);
    func_80089D10();
    ASSERT(PE_LoadU32(0x800C0DD8u) == (1u << 2), "89D10 D_800C0DD8");
    PASS();
}

/* src/func_8008A400.c */
static void test_PA_hi2_seq_stop(void)
{
    TEST("PA_hi2_8008A400_seq_stop");
    const pe_addr_t base = 0x800BC000u;
    unsigned k;

    /* id 0xFFFF: nothing. */
    ResetTestState();
    PE_StoreU32(0x800BCD50u, 0xFFFFFFFFu);
    func_8008A400(0x1FFFFu, 0u);
    ASSERT(PE_LoadU32(0x8009D2C4u) == 0u, "8A400 id 0xFFFF returns");

    /* default: active records whose +0x28 == id; a held one (0x100000)
     * gets 0x200000 instead. */
    ResetTestState();
    PE_StoreU32(0x800BCD50u, 0x7000u);       /* records 0..2 active */
    for (k = 0; k < 4; k++) {
        PE_StoreU32(base + k * 0x11Cu + 0x28u, 5u);
        PE_StoreU32(base + k * 0x11Cu + 0x38u, 0x40u);
    }
    PE_StoreU32(base + 0x11Cu + 0x38u, 0x100000u);
    func_8008A400(0x10005u, 0u);
    ASSERT(PE_LoadU32(base + 0x38u) == 0u && PE_LoadU32(base + 0x28u) == 0u,
           "8A400 record 0 stopped (func_8008F1B0 clears +0x28)");
    ASSERT(PE_LoadU32(base + 0x11Cu + 0x38u) == 0x300000u, "8A400 held record flagged");
    ASSERT(PE_LoadU32(base + 3u * 0x11Cu + 0x38u) == 0x40u, "8A400 inactive record kept");
    ASSERT(PE_LoadU32(0x800BCD5Cu) == 0x5000u, "8A400 D_800BCD5C bits");
    ASSERT(PE_LoadU32(0x800BCD50u) == 0x2000u, "8A400 func_8008F1B0 clears active bits");
    ASSERT(PE_LoadU32(0x8009D2C4u) == 0x110u, "8A400 D_8009D2C4 |= 0x10 then 0x100");

    /* a1 & 0x0FFFFFFF: +0x2C & a1. */
    ResetTestState();
    PE_StoreU32(0x800BCD50u, 0x3000u);
    PE_StoreU32(base + 0x2Cu, 0x4u);
    PE_StoreU32(base + 0x11Cu + 0x2Cu, 0x8u);
    func_8008A400(1u, 0x4u);
    ASSERT(PE_LoadU32(0x800BCD5Cu) == 0x1000u, "8A400 +0x2C selector");

    /* a1 & 0x40000000: among active records with +0x2C == 0, the maximum
     * +0x50 (signed; record 2's negative is below the 0 floor). */
    ResetTestState();
    PE_StoreU32(0x800BCD50u, 0xF000u);
    PE_StoreU32(base + 0x50u, 7u);
    PE_StoreU32(base + 0x2Cu, 1u);           /* excluded by +0x2C */
    PE_StoreU32(base + 0x11Cu + 0x50u, 3u);
    PE_StoreU32(base + 2u * 0x11Cu + 0x50u, 0xFFFFFFFFu);
    PE_StoreU32(base + 3u * 0x11Cu + 0x50u, 3u);
    func_8008A400(1u, 0x40000000u);
    ASSERT(PE_LoadU32(0x800BCD5Cu) == (0x2000u | 0x8000u), "8A400 max-priority selector");

    /* a1 < 0: recurse on records id and id + 1, no tail. */
    ResetTestState();
    PE_StoreU32(0x800BCD50u, 0x6000u | 0x1000u);   /* records 0,1,2 */
    PE_StoreU32(base + 1u * 0x11Cu + 0x28u, 9u);
    PE_StoreU32(base + 2u * 0x11Cu + 0x28u, 9u);
    func_8008A400(1u, 0x80000000u);
    /* id 1: record 1 active -> recurse(9): stops records 1 and 2 (both
     * +0x28 == 9; func_8008F1B0 zeroes their +0x28).  Record 2 was active in
     * the caller's captured `act`, so recurse(record 2 +0x28 == 0) follows
     * and stops record 0 (+0x28 == 0).  No tail calls on this path. */
    ASSERT(PE_LoadU32(0x800BCD5Cu) == 0x7000u, "8A400 recursion on id and id + 1");
    ASSERT(PE_LoadU32(0x8009D2C4u) == 0x110u, "8A400 tail only in the recursive calls");
    PASS();
}

/* src/func_8008CF70.c */
static void test_PA_hi2_reverb(void)
{
    TEST("PA_hi2_8008CF70_reverb");
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    const pe_addr_t regs = PA_HI2 + 0x3800u;
    unsigned k;

    PE_StoreU32(0x8009B3FCu, regs);        /* RAM stand-in register block */
    PE_StoreU32(0x8009B464u, 0u);           /* func_80085BB4: no allocations */
    PE_StoreU32(0x8009B46Cu + 7u * 4u, 0x1234u);
    for (k = 0; k < 68; k++)
        PE_StoreU8(0x8009C8C0u + 7u * 68u + k, (uint8_t)(k + 1));
    PE_StoreU16(regs + 0x1AAu, 0x80u | 0x1u);
    PE_StoreU16(regs + 0x184u, 0x55u);
    PE_StoreU16(regs + 0x186u, 0x66u);

    ASSERT(func_8008CF70(10) == -1, "8CF70 a0 >= 10 fails");
    ASSERT(func_8008CF70(7 | 0x100) == 0, "8CF70 preset 7 ok");
    ASSERT(PE_LoadU32(0x8009B39Cu + 4u) == 7u && PE_LoadU32(0x8009B398u) == 0x1234u,
           "8CF70 mode / start address");
    ASSERT(PE_LoadU32(0x8009B39Cu + 0xCu) == 0x7Fu &&
           PE_LoadU32(0x8009B39Cu + 0x10u) == 0x7Fu, "8CF70 preset 7 delay/feedback");
    ASSERT(PE_LoadU32(PE_HAND_HI2_STACK + 0x10u) == 0u &&
           PE_LoadU8(PE_HAND_HI2_STACK + 0x10u + 67u) == 68u, "8CF70 entry copied, mask 0");
    /* entry.mask == 0 -> func_8008D140 writes all 32 attribute halfwords
     * (+4.. of entry) to +0x1C0.. */
    ASSERT(PE_LoadU16(regs + 0x1C0u) == (uint16_t)(5u | (6u << 8)), "8CF70 func_8008D140(&entry)");
    ASSERT(PE_LoadU16(regs + 0x184u) == 0u && PE_LoadU16(regs + 0x186u) == 0u,
           "8CF70 depth registers zeroed");
    ASSERT(PE_LoadU16(regs + 0xD1u * 2u) == 0x1234u, "8CF70 func_8007DAE0(0xD1, start, 0)");
    /* func_8008D610 now runs natively (pe_stream.c): like retail
     * func_8007D778(1) it leaves SPUCNT in DMA-write mode (bits 4-5 = 2)
     * before bit 7 is restored. */
    ASSERT(PE_LoadU16(regs + 0x1AAu) == 0xA1u, "8CF70 SPUCNT bit 7 restored, DMA-write mode");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "8CF70 clear -> native func_8008D610 (no boundary)");

    /* preset 8 without clear and with bit 7 off. */
    PE_Decomp_ResetBoundaries();
    PE_StoreU16(regs + 0x1AAu, 0u);
    ASSERT(func_8008CF70(8) == 0, "8CF70 preset 8 ok");
    ASSERT(PE_LoadU32(0x8009B39Cu + 0xCu) == 0x7Fu &&
           PE_LoadU32(0x8009B39Cu + 0x10u) == 0u, "8CF70 preset 8 delay/feedback");
    ASSERT(PE_LoadU16(regs + 0x1AAu) == 0u, "8CF70 SPUCNT stays off");
    ASSERT(PE_Decomp_BoundaryCount() == 0, "8CF70 no clear boundary");

    /* allocation overlap -> -1 before any state change. */
    PE_StoreU32(0x8009B464u, PA_HI2 + 0x3C00u);
    PE_StoreU32(0x8009B424u, 0u);
    PE_StoreU32(PA_HI2 + 0x3C00u, 0x0u);       /* start 0 <= x: overlap */
    PE_StoreU32(PA_HI2 + 0x3C04u, 0x100u);
    PE_StoreU32(0x8009B39Cu + 4u, 0xAAu);
    ASSERT(func_8008CF70(3) == -1, "8CF70 allocated area fails");
    ASSERT(PE_LoadU32(0x8009B39Cu + 4u) == 0xAAu, "8CF70 fail leaves state");
    PASS();
}

static void test_PA_hi2_all(void)
{
    test_PA_hi2_voice_regs();
    test_PA_hi2_voice_claim();
    test_PA_hi2_collectors();
    test_PA_hi2_seq_stop();
    test_PA_hi2_reverb();
}
