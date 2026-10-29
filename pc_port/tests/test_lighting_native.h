/* Production-caller witnesses for replacement of the B97C lighting cut.
 * Expectations are retail side effects, independent of the generated TU. */
static void lighting_actor_fixture(unsigned parts)
{
    ResetTestState();
    PE_StoreU32(0x80130000u, 0x80131000u);
    PE_StoreU16(0x801300BAu, 1u);
    PE_StoreU16(0x8013009Cu, 1u); /* AF14 lighting + empty packet walks */
    PE_StoreU8(0x80131002u, parts);
    PE_StoreU8(0x80130088u, 0x24u);
    PE_StoreU32(0x8009CDA0u, 0x30808080u);
    PE_GTE_SetBK(1, 2, 3);
    PE_GTE_SetRGBC(0x12345678u);
    PE_StoreU32(0x1F800000u, 0xDEADBEEFu);
}

static void test_LIGHTING_AF14_empty_model_side_effects(void)
{
    TEST("LIGHTING_AF14_empty_model_side_effects");
    lighting_actor_fixture(0);
    ASSERT(func_8003AF14(0x80130000u, 0) == 0, "production tick");
    ASSERT(PE_LoadU32(0x1F800000u) == 0x30808080u, "retail saves base colour before part-count gate");
    ASSERT(PE_GTE_ReadCtrl(13) == 0x240u && PE_GTE_ReadCtrl(14) == 0x240u
           && PE_GTE_ReadCtrl(15) == 0x240u, "retail writes BK before part-count gate");
    ASSERT(PE_GTE_ReadData(6) == 0x12345678u, "empty model leaves RGBC intact");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

static void test_LIGHTING_AF14_visible_part_scratch(void)
{
    unsigned i;
    TEST("LIGHTING_AF14_visible_part_scratch");
    lighting_actor_fixture(1);
    PE_StoreU32(0x80130004u, 0x80132000u);
    PE_StoreU32(0x80130084u, 0x80135000u);
    PE_StoreU8(0x80132004u, 1u); /* visible part, no vertices */
    pe_btl6_store_identity_matrix(0x800BEA40u);
    for (i = 0; i < 9; ++i) {
        PE_StoreU16(0x80135000u + 2u*i, (uint16_t)(100u*(i+1u)));
        PE_StoreU16(0x1F800004u + 2u*i, 0xFFFFu);
    }
    ASSERT(func_8003AF14(0x80130000u, 0) == 0, "production tick");
    for (i = 0; i < 9; ++i)
        ASSERT(PE_LoadU16(0x1F800004u + 2u*i) == 100u*(i+1u), "retail writes transformed light matrix to guest scratchpad");
    ASSERT(PE_GTE_ReadData(6) == 0x12345678u, "zero vertices leave RGBC intact");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "no boundary");
    PASS();
}

static void test_LIGHTING_AF14_early_returns(void)
{
    TEST("LIGHTING_AF14_early_returns");
    lighting_actor_fixture(0);
    PE_StoreU16(0x801300BAu, 0u);
    func_8003AF14(0x80130000u, 0);
    ASSERT(PE_LoadU32(0x1F800000u) == 0xDEADBEEFu && PE_GTE_ReadCtrl(13) == 1u, "draw-count zero returns before lighting");
    PE_StoreU16(0x801300BAu, 1u);
    PE_StoreU32(0x80130000u, 0u);
    func_8003AF14(0x80130000u, 0);
    ASSERT(PE_LoadU32(0x1F800000u) == 0xDEADBEEFu, "null header returns before lighting");
    func_8003AF14(0, 0);
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_NONE, "null entity caller returns");
    PASS();
}
