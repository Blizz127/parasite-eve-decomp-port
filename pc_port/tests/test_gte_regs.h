/* E21b: register-level COP2 access used by lowered inline GTE asm
 * (PE_GTE_ReadData/WriteData/ReadCtrl/WriteCtrl/Command, pe_gte.c), checked
 * against psx-spx "GTE Registers":
 *  - ctc2 $0-$4 pack RT two halves per register, RT33 alone and read back
 *    sign-extended; TR full 32-bit; H reads back sign-extended (quirk);
 *  - VXY0/VZ0 halves, IR writes/reads sign-extended, SXYP pushes the FIFO,
 *    IRGB -> IR*0x80 and ORGB saturation, LZCS -> LZCR;
 *  - the gte_SetRotMatrix / gte_ldv0 / rtv0 / gte_stlvnl register sequence
 *    (what the generator emits) equals PE_GTE_MVMVA on the same state, and
 *    NCLIP through PE_GTE_Command fills MAC0. */
static void test_GTE_regs_pack(void)
{
    TEST("GTE_regs_pack");
    ResetTestState();
    memset(&g_pe_gte, 0, sizeof g_pe_gte);
    PE_GTE_WriteCtrl(0u, 0xF0001000u);
    PE_GTE_WriteCtrl(1u, 0x00230FFFu);
    PE_GTE_WriteCtrl(4u, 0x1234FF80u);
    PE_GTE_WriteCtrl(5u, 0xFFFFFFFEu);
    ASSERT(g_pe_gte.rt[0][0] == 0x1000 && g_pe_gte.rt[0][1] == (int16_t)0xF000, "ctc2 $0 packing");
    ASSERT(g_pe_gte.rt[0][2] == 0x0FFF && g_pe_gte.rt[1][0] == 0x0023, "ctc2 $1 packing");
    ASSERT(g_pe_gte.rt[2][2] == -128 && PE_GTE_ReadCtrl(4u) == 0xFFFFFF80u, "RT33 sign-extended");
    ASSERT(g_pe_gte.tr[0] == -2 && PE_GTE_ReadCtrl(5u) == 0xFFFFFFFEu, "TRX full word");
    ASSERT(PE_GTE_ReadCtrl(1u) == 0x00230FFFu, "cfc2 $1 round trip");
    PE_GTE_WriteCtrl(26u, 0x0001F000u);
    ASSERT(g_pe_gte.h == 0xF000 && PE_GTE_ReadCtrl(26u) == 0xFFFFF000u, "H write u16, read sign-extended");
    PE_GTE_WriteData(0u, 0x8001FFFEu);
    PE_GTE_WriteData(1u, 0x00017FFFu);
    ASSERT(g_pe_gte.v0[0] == -2 && g_pe_gte.v0[1] == (int16_t)0x8001 && g_pe_gte.v0[2] == 0x7FFF, "VXY0/VZ0");
    ASSERT(PE_GTE_ReadData(0u) == 0x8001FFFEu && PE_GTE_ReadData(1u) == 0x7FFFu, "VXY0/VZ0 read");
    PE_GTE_WriteData(9u, 0x0001FFF0u);
    ASSERT(g_pe_gte.ir[0] == -16 && PE_GTE_ReadData(9u) == 0xFFFFFFF0u, "IR1 sign-extended");
    PE_GTE_WriteData(12u, 1u); PE_GTE_WriteData(13u, 2u); PE_GTE_WriteData(14u, 3u);
    PE_GTE_WriteData(15u, 4u);
    ASSERT(g_pe_gte.sxy[0] == 2u && g_pe_gte.sxy[1] == 3u && g_pe_gte.sxy[2] == 4u, "SXYP push");
    PE_GTE_WriteData(28u, (3u << 10) | (0x1Fu << 5) | 1u);
    ASSERT(g_pe_gte.ir[0] == 0x80 && g_pe_gte.ir[1] == 0xF80 && g_pe_gte.ir[2] == 0x180, "IRGB");
    g_pe_gte.ir[1] = 0x7FFF; g_pe_gte.ir[2] = -5;
    ASSERT(PE_GTE_ReadData(29u) == ((0u << 10) | (0x1Fu << 5) | 1u), "ORGB saturation");
    PE_GTE_WriteData(30u, 0xFFFF0000u);
    ASSERT(PE_GTE_ReadData(31u) == 16u, "LZCS -> LZCR");
    PASS();
}

static void test_GTE_regs_rtv0_sequence(void)
{
    static const int16_t R[9] = { 0x0E00, -0x123, 0x0400, 0x0011, 0x1000, -0x7FF, -0x0F00, 0x0222, 0x0B00 };
    static const int32_t T[3] = { 1000, -70000, 3 };
    static const int16_t V[3] = { -300, 32000, 4095 };
    const pe_addr_t m = 0x80190000u, v = 0x80190040u, o = 0x80190080u;
    int32_t want_mac[3], want_ir[3];
    TEST("GTE_regs_rtv0_sequence");
    ResetTestState();
    for (unsigned i = 0; i < 9u; i++) PE_StoreU16(m + i * 2u, (uint16_t)R[i]);
    for (unsigned i = 0; i < 3u; i++) PE_StoreU32(m + 20u + i * 4u, (uint32_t)T[i]);
    for (unsigned i = 0; i < 3u; i++) PE_StoreU16(v + i * 2u, (uint16_t)V[i]);
    for (unsigned w = 0; w < 2u; w++) {
        const uint32_t cmd = w ? 0x480012u : 0x486012u;     /* rtv0tr / rtv0 */
        memset(&g_pe_gte, 0, sizeof g_pe_gte);
        for (unsigned k = 0; k < 9u; k++) g_pe_gte.rt[k / 3u][k % 3u] = R[k];
        for (unsigned k = 0; k < 3u; k++) g_pe_gte.tr[k] = T[k];
        PE_GTE_SetV0(V[0], V[1], V[2]);
        PE_GTE_MVMVA(cmd);
        memcpy(want_mac, g_pe_gte.mac, sizeof want_mac); memcpy(want_ir, g_pe_gte.ir, sizeof want_ir);
        memset(&g_pe_gte, 0, sizeof g_pe_gte);
        for (unsigned r = 0; r < 5u; r++) PE_GTE_WriteCtrl(r, PE_LoadU32(m + r * 4u));   /* lw/ctc2 */
        for (unsigned r = 5u; r < 8u; r++) PE_GTE_WriteCtrl(r, PE_LoadU32(m + r * 4u));
        PE_GTE_WriteData(0u, PE_LoadU32(v)); PE_GTE_WriteData(1u, PE_LoadU32(v + 4u)); /* lwc2 */
        PE_GTE_Command(cmd);
        for (unsigned r = 0; r < 3u; r++) PE_StoreU32(o + r * 4u, PE_GTE_ReadData(25u + r)); /* swc2 */
        for (unsigned r = 0; r < 3u; r++) {
            ASSERT((int32_t)PE_LoadU32(o + r * 4u) == want_mac[r], "MAC via register sequence");
            ASSERT((int32_t)PE_GTE_ReadData(9u + r) == want_ir[r], "IR via register sequence");
        }
    }
    g_pe_gte.sxy[0] = 0x00000000u; g_pe_gte.sxy[1] = 0x00000010u; g_pe_gte.sxy[2] = 0x00100000u;
    PE_GTE_Command(0x1400006u);
    ASSERT((int32_t)PE_GTE_ReadData(24u) == 256, "NCLIP via PE_GTE_Command -> MAC0");
    PASS();
}
