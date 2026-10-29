/* Host drive tray for the retail disc-swap screen (platform/pe_drive_tray.h):
 * the lid opens PE_TRAY_OPEN_DELAY frames after the screen starts waiting
 * for it (status 0x10 latched, CdReady lane 2), closes PE_TRAY_CLOSE_DELAY
 * frames later on the spare disc (lane 1, status clear), and the disc that
 * came out becomes the spare.  The host CdlStop publishes a completed
 * command that func_8007F418 reads back with status 2.  No spare for the
 * wanted disc: the tray does nothing (the screen is a STOP instead). */
static void test_drive_tray_swap(void)
{
    static unsigned char fake1, fake2;
    PE_Disc *d1 = (PE_Disc *)(void *)&fake1, *d2 = (PE_Disc *)(void *)&fake2;
    PE_Disc *before = PE_Disc_GetActive();
    unsigned f;
    uint32_t seq;
    TEST("drive_tray_swap");
    ResetTestState();
    PE_Tray_Reset();
    ASSERT(!PE_Tray_CanServe(2), "empty tray claims a disc");
    PE_Disc_SetActive(d1);
    PE_Tray_SetSpare(d2, 2);
    ASSERT(PE_Tray_CanServe(2) && !PE_Tray_CanServe(1), "spare disc number");
    PE_StoreU32(0x8009B574u, 1u);
    PE_StoreU8(0x8009B56Cu, 0x02u);
    PE_Tray_Begin(2);
    for (f = 0; f < 10u; f++) ASSERT(!PE_Tray_Frame(0), "tray acted before the lid prompt");
    for (f = 1; f < PE_TRAY_OPEN_DELAY; f++) ASSERT(!PE_Tray_Frame(1), "lid opened early");
    ASSERT(!PE_Tray_Frame(1) && (PE_LoadU8(0x8009B56Cu) & 0x10u) &&
           PE_LoadU32(0x8009B574u) == 2u && PE_Disc_GetActive() == d1,
           "lid open state differs");
    for (f = 1; f < PE_TRAY_CLOSE_DELAY; f++) ASSERT(!PE_Tray_Frame(0), "lid closed early");
    ASSERT(PE_Tray_Frame(0) && PE_Disc_GetActive() == d2 &&
           !(PE_LoadU8(0x8009B56Cu) & 0x10u) && PE_LoadU32(0x8009B574u) == 1u &&
           PE_Tray_ActiveNumber() == 2 && PE_Tray_CanServe(1) && PE_Tray_Swaps() == 1u,
           "lid close / swap state differs");
    /* Insert-now (dev warp harness) swaps back. */
    ASSERT(PE_Tray_InsertNow(1) && PE_Disc_GetActive() == d1 && PE_Tray_CanServe(2),
           "insert-now differs");
    /* Host CdlStop: a completed command (status 2) found by F418. */
    PE_StoreU32(0x8009B53Cu, 41u);
    PE_StoreU32(0x800A3690u, 3u);
    seq = PE_Tray_HostStop();
    ASSERT(seq == 42u && PE_LoadU32(0x800A3610u + 3u * 16u) == 42u &&
           PE_LoadU32(0x800A3690u) == 4u, "host stop record differs");
    ASSERT(func_8007F418(seq, 0x80140000u) == 2, "F418 did not read the stop as complete");
    PE_Tray_Reset();
    PE_Disc_SetActive(before);
    PASS();
}
