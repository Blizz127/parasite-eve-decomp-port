/* Split-brain scalars: one guest-memory authority per retail global.
 *
 * D_8009D018 (gp+0x2A8) and D_800A7918 (story word persist[74]) used to be
 * host variables while other code used their guest words:
 *   - the hand writers (func_80051CC4 / func_800512AC) assign D_8009D018 by
 *     name, but the generated reader func_80051E58 loads guest 0x8009D018;
 *   - every story writer stores guest 0x800A7918, but the main-loop volume
 *     gate in func_8001220C reads D_800A7918 by name.
 * Each check exercises exactly those two access forms and fails on the split
 * layout (the reader sees 0). */
int func_80051E58(void);

static void test_SPLIT_d8009d018_writer_reader(void)
{
    TEST("SPLIT_d8009d018_writer_reader");
    ResetTestState();
    D_8009D018 = 4u;                        /* func_800512AC case 12 form */
    ASSERT(func_80051E58() == 4, "generated func_80051E58 sees the hand writer's value");
    D_8009D018 = 1u << (10u - 8u);          /* func_80051CC4 command-10 form */
    ASSERT(func_80051E58() == 4 && PE_LoadU32(0x8009D018u) == 4u, "one storage for gp+0x2A8");
    D_8009D018 = 0u;
    PASS();
}

static void test_SPLIT_d800a7918_writer_reader(void)
{
    TEST("SPLIT_d800a7918_writer_reader");
    ResetTestState();
    PE_StoreU32(0x800A7918u, 0x258u);       /* story writers store the guest word */
    ASSERT(D_800A7918 == 0x258u, "main-loop gate (by name) sees the stored story word");
    D_800A7918 = 0x80u;                     /* and a by-name write reaches the guest word */
    ASSERT(PE_LoadU32(0x800A7918u) == 0x80u, "one storage for persist[74]");
    PE_StoreU32(0x800A7918u, 0u);
    PASS();
}

static void test_SPLIT_all(void)
{
    test_SPLIT_d8009d018_writer_reader();
    test_SPLIT_d800a7918_writer_reader();
}
