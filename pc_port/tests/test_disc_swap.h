/* func_80069B08 (retail disc-swap screen) has no native port.  The main loop
 * calls it when persist[74] needs the other disc or when New Game init
 * (0xAA108448) runs without disc 2; a silent return left D_800B0DCD unchanged
 * and func_8001220C asked again forever (the m0050i warp-probe hang).  It
 * must be a CPU_BOUNDARY STOP (unresolved-boundary) and must not fake the
 * swap (D_800B0DCD stays as the mount found it). */
static void test_disc_swap_boundary_stops(void)
{
    TEST("disc_swap_boundary_stops");
    ResetTestState();
    PE_Port_RunControlReset();
    PE_StoreU32(0x800A7918u, 0x2F4u);
    PE_StoreU8(0x800B0DCDu, 1u);
    ASSERT(!PE_Port_ShouldStop(), "run control not reset");
    PE_Port_DiscSwapBoundary(2);
    ASSERT(PE_Port_ShouldStop() &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "disc swap did not stop with unresolved-boundary");
    ASSERT(PE_LoadU8(0x800B0DCDu) == 1u, "disc swap faked the disc-2 flag");
    PE_Port_RunControlReset();
    PASS();
}
