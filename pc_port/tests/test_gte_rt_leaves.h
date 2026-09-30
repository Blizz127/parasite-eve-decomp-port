static void test_DAY1_gte_rt_leaves(void)
{
    TEST("DAY1_gte_rt_leaves");
    ResetTestState();
    PE_StoreU16(0x80140000u, 0x1000);
    PE_StoreU16(0x80140002u, 0);
    PE_StoreU16(0x80140004u, 0);
    PE_StoreU16(0x80140006u, 0);
    PE_StoreU16(0x80140008u, 0x1000);
    PE_StoreU16(0x8014000Au, 0);
    PE_StoreU16(0x8014000Cu, 0);
    PE_StoreU16(0x8014000Eu, 0);
    PE_StoreU16(0x80140010u, 0x1000);
    PE_StoreU32(0x80140014u, 10u);
    PE_StoreU32(0x80140018u, (uint32_t)-20);
    PE_StoreU32(0x8014001Cu, 400u);
    func_80078E04(0x80140000u);
    func_80078E94(0x80140000u);
    ASSERT(g_pe_gte.rt[0][0] == 0x1000 && g_pe_gte.rt[1][1] == 0x1000 &&
           g_pe_gte.rt[2][2] == 0x1000, "78E04 loads RT words 0..4");
    ASSERT(g_pe_gte.tr[0] == 10 && g_pe_gte.tr[1] == -20 &&
           g_pe_gte.tr[2] == 400, "78E94 loads TR words at +0x14");
    PE_StoreU32(0x800963E8u, 0u);
    func_80078A94();
    ASSERT(PE_LoadU32(0x800963E8u) == 0x20u, "78A94 depth += 0x20");
    ASSERT((int16_t)PE_LoadU16(0x800963ECu) == 0x1000, "78A94 cfc2 RT m00");
    ASSERT((int32_t)PE_LoadU32(0x800963ECu + 20u) == 10, "78A94 cfc2 TRX");
    g_pe_gte.rt[0][0] = 0;
    g_pe_gte.tr[0] = 0;
    func_80078B38();
    ASSERT(PE_LoadU32(0x800963E8u) == 0u, "78B38 depth -= 0x20");
    ASSERT(g_pe_gte.rt[0][0] == 0x1000 && g_pe_gte.tr[0] == 10,
           "78B38 restores RT/TR");
    func_80078B38();
    ASSERT(PE_LoadU32(0x800963E8u) == 0u, "78B38 empty stack is a no-op");
    PE_StoreU32(0x80141000u, 0x0000000Cu);
    ASSERT(func_8006EC6C(0x80141000u, 0) == 0x8014100Cu,
           "6EC6C a0+lw(a0+(int16)a1*4)");
    PE_StoreU32(0x80141000u + 12u, 0x20u);
    ASSERT(func_8006EC6C(0x80141000u, 3) == 0x80141020u,
           "6EC6C word index 3");
    PE_StoreU32(0x80141000u - 4u, 0x40u);
    ASSERT(func_8006EC6C(0x80141000u, -1) == 0x80141040u,
           "6EC6C signed word index");
    PE_StoreU32(0x801D0260u + 12u, 0x20u);
    PE_StoreU32(0x80141020u, 0u);
    PE_StoreU32(0x80141024u, 0u);
    PE_StoreU32(0x8014104Cu, 0u);
    PE_StoreU32(0x800BCFA4u, 0xA5A5A5A5u);
    PE_StoreU32(0x800BCFA8u, 0x5A5A5A5Au);
    PE_StoreU32(0x800963E8u, 0u);
    ASSERT(func_80191E30(0x123u) == -1, "91E30 empty package returns -1");
    ASSERT(PE_LoadU32(0x800BCFA4u) == 0xA5A5A5A5u &&
           PE_LoadU32(0x800BCFA8u) == 0x5A5A5A5Au, "91E30 restores camera globals");
    ASSERT(PE_LoadU32(0x800963E8u) == 0u, "91E30 Push/Pop is balanced");
    ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(), "GTE RT leaves execute natively");
    PASS();
}
