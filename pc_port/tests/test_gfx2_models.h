/* gfx2 lane: model attachment modes of func_8003A088 (src/func_8003A088.c),
 * the room lighting-state setters func_8003F758/func_8003F798, and the
 * func_80068B94 projection tail.  Included by test_native.c. */
#include "game/decomp_hand/hand_lo_protos.h"

static void gfx2_store_matrix(pe_addr_t m, int16_t d, int32_t tx, int32_t ty, int32_t tz)
{
    unsigned k;
    for (k = 0; k < 9u; k++) PE_StoreU16(m + k * 2u, 0u);
    PE_StoreU16(m + 0u, (uint16_t)d);
    PE_StoreU16(m + 8u, (uint16_t)d);
    PE_StoreU16(m + 16u, (uint16_t)d);
    PE_StoreU32(m + 20u, (uint32_t)tx);
    PE_StoreU32(m + 24u, (uint32_t)ty);
    PE_StoreU32(m + 28u, (uint32_t)tz);
}

/* Mode 3 = rigid attachment (Aya's weapon: func_8006CC68 publishes parent
 * = Aya +0x1B4, mode 3, bone 0x12).  The parent's bone matrix lands in
 * joints 0 and 1; retail returns before the walk. */
static void test_GFX2_3A088_mode3_attach(void)
{
    pe_addr_t weapon = 0x80130000u, parent = 0x80131000u;
    pe_addr_t wj = 0x80132000u, pj = 0x80133000u;

    TEST("GFX2_3A088_mode3_attach");
    ResetTestState();
    PE_StoreU32(weapon + 0x84u, wj);
    PE_StoreU32(parent + 0x84u, pj);
    PE_StoreU32(weapon + 0x24u, parent);
    PE_StoreU16(weapon + 0x28u, 3u);
    PE_StoreU16(weapon + 0x2Au, 2u);
    PE_StoreU16(weapon + 0x9Cu, 0u);
    gfx2_store_matrix(pj + 2u * 32u, 4096, 11, -22, 33);
    func_8003A088_mode0_walk_cut(weapon);
    ASSERT(PE_LoadU16(wj + 0u) == 4096u && PE_LoadU16(wj + 16u) == 4096u, "joint0 rotation from parent bone 2");
    ASSERT((int32_t)PE_LoadU32(wj + 24u) == -22, "joint0 translation");
    ASSERT(PE_LoadU16(wj + 0x20u) == 4096u && (int32_t)PE_LoadU32(wj + 0x3Cu) == 33, "joint1 copy");

    /* +0x9C bit 0x400: the model matrix at +0x34 goes to joint 1 instead. */
    PE_StoreU16(weapon + 0x9Cu, 0x400u);
    gfx2_store_matrix(weapon + 0x34u, 2048, 5, 6, 7);
    func_8003A088_mode0_walk_cut(weapon);
    ASSERT(PE_LoadU16(wj + 0x20u) == 2048u && (int32_t)PE_LoadU32(wj + 0x38u) == 6, "0x400 model-matrix copy");
    PASS();
}

/* Mode 4 = parent-relative: +0x34 rotation and +0x48 translation are
 * transformed by the parent's bone matrix before the walk. */
static void test_GFX2_3A088_mode4_parent_relative(void)
{
    pe_addr_t child = 0x80130000u, parent = 0x80131000u;
    pe_addr_t pj = 0x80133000u, obj = 0x80134000u;

    TEST("GFX2_3A088_mode4_parent_relative");
    ResetTestState();
    PE_StoreU32(parent + 0x84u, pj);
    PE_StoreU32(child + 0x24u, parent);
    PE_StoreU32(child + 0x00u, obj);       /* part count 0: no walk */
    PE_StoreU16(obj + 0x18u, 0u);
    PE_StoreU16(child + 0x28u, 4u);
    PE_StoreU16(child + 0x2Au, 1u);
    gfx2_store_matrix(pj + 32u, 2048, 100, 200, 300);   /* half scale + T */
    gfx2_store_matrix(child + 0x34u, 4096, 40, 80, -120);
    func_8003A088_mode0_walk_cut(child);
    ASSERT(PE_LoadU16(child + 0x34u) == 2048u && PE_LoadU16(child + 0x44u) == 2048u, "R = Rp * R");
    ASSERT((int32_t)PE_LoadU32(child + 0x48u) == 120 && (int32_t)PE_LoadU32(child + 0x4Cu) == 240
           && (int32_t)PE_LoadU32(child + 0x50u) == 240, "T = Rp * T + Tp");
    PASS();
}

static void test_GFX2_3F758_3F798_light_state(void)
{
    pe_addr_t s = 0x80130000u;
    unsigned k;

    TEST("GFX2_3F758_3F798_light_state");
    ResetTestState();
    for (k = 0; k < 9u; k++) PE_StoreU16(s + k * 2u, 0x1234u);
    func_8003F758(s, 0x28, 0x28, 0x28);
    ASSERT(g_pe_gte.bk[0] == 0x280 && g_pe_gte.bk[2] == 0x280, "BK = rgb << 4");
    ASSERT(PE_LoadU16(s + 0u) == 0u && PE_LoadU16(s + 16u) == 0u, "light matrix zeroed");
    func_8003F798(s, 0, 0xFF, 0xFF, 0xFF);
    func_8003F798(s, 1, 0x80, 0x80, 0x80);
    func_8003F798(s, 2, 0x60, 0x60, 0x60);
    ASSERT(g_pe_gte.lcm[0][0] == 0xFF && g_pe_gte.lcm[0][1] == 0x80 && g_pe_gte.lcm[2][2] == 0x60, "LCM columns");
    ASSERT(PE_LoadU8(s + 0x44u) == 0x80 && PE_LoadU8(s + 0x4Au) == 0x60, "byte triples at +0x40");
    PASS();
}

static void test_GFX2_models(void)
{
    test_GFX2_3A088_mode3_attach();
    test_GFX2_3A088_mode4_parent_relative();
    test_GFX2_3F758_3F798_light_state();
}
