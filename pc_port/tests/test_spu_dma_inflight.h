/* func_80085174's wait and func_800850F4's issue while a DMA4 transfer is
 * still in flight (docs/evidence/native-spuwait-20261026/REPORT.md).  The
 * port completes a transfer only when the host services it (the VSync pump,
 * the waits, func_8006CDA4's loop), so one can stay pending across a return:
 * func_800851A8 with count 0 returns right after its issue, as retail does.
 * Retail's DMA4 finishes in far less than a frame, and its _spu_t
 * (func_8007D778) never refuses an issue.  The game-over destination
 * func_8006A25C runs func_80085744, whose busy branch issues again and waits.
 * No disc data or CPU interpreter. */
#define SPUFL_A_SRC   0x80124000u   /* the in-flight transfer's source */
#define SPUFL_A_SPU   0x2600u
#define SPUFL_B_SRC   0x80125000u
#define SPUFL_B_SPU   0x3800u
#define SPUFL_UPLOAD  0x80126000u   /* func_800851A8 AKAO buffer */
#define SPUFL_B81C    0x8009B81Cu   /* func_80085744's 0x40-byte block */
#define SPUFL_C4C0    0x8009C4C0u   /* SpuClearReverbWorkArea's source */
#define SPUFL_NONE    0x80012222u   /* no native code at this address */

void func_80085744(void);           /* generated: src/func_80085744.c */

/* Run fn() with stderr captured (the test_port_absent.h pattern) and return
 * what it printed.  The stop report's detail keeps the first stop of the
 * whole process, an earlier test's in a full run, so a test reads the stop
 * line it caused from stderr instead. */
static int SPUFL_Captured(void (*fn)(void), char *out, size_t cap)
{
    FILE *capture = tmpfile();
    int saved;
    size_t n;

    if (!capture) return 0;
    fflush(stderr);
    saved = dup(fileno(stderr));
    if (saved < 0 || dup2(fileno(capture), fileno(stderr)) < 0) {
        if (saved >= 0) close(saved);
        fclose(capture);
        return 0;
    }
    fn();
    fflush(stderr);
    dup2(saved, fileno(stderr));
    close(saved);
    rewind(capture);
    n = fread(out, 1, cap - 1u, capture);
    out[n] = 0;
    fclose(capture);
    return 1;
}

static void SPUFL_CallWait(void)
{
    (void)PE_GuestCall("SPUFL wait", 0x80085174u, 0u, 0u, 0u, 0u, 0u);
}

static void SPUFL_CallIssueA(void)
{
    func_800850F4(SPUFL_A_SRC, 0x40u);
}

static void SPUFL_Reset(uint32_t spu_address)
{
    ResetTestState();
    PE_Decomp_ResetBoundaries();
    B48A_PrepareDma(spu_address);
    g_stub_order_count = 0;
    Bootstrap_ResetArg4CallLog();
}

/* No stop, no boundary of any kind, no bootstrap return. */
static int SPUFL_Clean(void)
{
    return !PE_Port_ShouldStop() && PE_Decomp_BoundaryCount() == 0 &&
           !g_stub_order_count && !g_bootstrap_arg4_call_count;
}

static void test_SPUFL_shutdown_with_transfer_in_flight(void)
{
    PeSpuDmaState state;
    int i, ok;
    TEST("SPUFL_shutdown_with_transfer_in_flight");
    SPUFL_Reset(0);
    /* func_800851A8(buffer, 0): AKAO magic, SPU address 0x2600, 0x80 bytes
     * of DMA payload at buffer+0x80 (param2 0, param3 1: one 64-byte block
     * of words copied to D_800B2900 first). */
    PE_StoreU32(SPUFL_UPLOAD, 0x4F414B41u);
    PE_StoreU32(SPUFL_UPLOAD + 0x10u, SPUFL_A_SPU);
    PE_StoreU32(SPUFL_UPLOAD + 0x14u, 0x80u);
    PE_StoreU32(SPUFL_UPLOAD + 0x18u, 0u);
    PE_StoreU32(SPUFL_UPLOAD + 0x1Cu, 1u);
    B48A_Fill(SPUFL_UPLOAD + 0x80u, 0x80u, 0x10u);
    B48A_Fill(SPUFL_B81C, 0x40u, 0xC0u);
    ASSERT(func_800851A8(SPUFL_UPLOAD, 0) == 0, "upload refused");
    PE_SpuDma_GetState(&state);
    ASSERT(state.pending == 1 && PE_LoadU32(0x8009D24Cu) == 1u &&
           PE_LoadU32(0x8009B434u) == PE_SPU_DMA_CALLBACK,
           "count-0 upload did not return with its transfer in flight");

    func_80085744();

    PE_SpuDma_GetState(&state);
    ASSERT(SPUFL_Clean(), "shutdown stopped, aborted or took a boundary");
    ASSERT(state.pending == 0 && state.event_count == 2,
           "both transfers were not delivered");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 0u, "the wait returned busy");
    /* 85744 issues at the unchanged transfer address: its 0x40 bytes land
     * last, over the first half of the in-flight transfer, whose second
     * half shows it completed first. */
    ok = 1;
    for (i = 0; i < 0x40; i++)
        ok &= PE_SpuRam_LoadU8(SPUFL_A_SPU + (uint32_t)i) == (uint8_t)(0xC0u + i);
    for (i = 0x40; i < 0x80; i++)
        ok &= PE_SpuRam_LoadU8(SPUFL_A_SPU + (uint32_t)i) == (uint8_t)(0x10u + i);
    ASSERT(ok, "SPU RAM does not show the in-flight transfer, then 85744's");
    ASSERT(PE_LoadU32(0x8009B3ECu) == 0u && PE_LoadU32(0x8009B434u) == 0u,
           "SpuQuit did not run after the wait");
    PASS();
}

static void SPUFL_IssueA(uint8_t seed)
{
    SPUFL_Reset(SPUFL_A_SPU);
    B48A_Fill(SPUFL_A_SRC, 0x40u, seed);
    func_800850F4(SPUFL_A_SRC, 0x40u);
}

static void test_SPUFL_registry_wait_services_in_flight(void)
{
    PeSpuDmaState state;
    TEST("SPUFL_registry_wait_services_in_flight");
    SPUFL_IssueA(0x21u);
    /* Retail's jal/jalr into func_80085174 through the guest-code registry. */
    ASSERT(PE_GuestCode_Resolve(0x80085174u) != NULL,
           "func_80085174 has no registry row");
    (void)PE_GuestCall("SPUFL wait", 0x80085174u, 0u, 0u, 0u, 0u, 0u);
    PE_SpuDma_GetState(&state);
    ASSERT(SPUFL_Clean(), "wait stopped or took a boundary");
    ASSERT(state.pending == 0 && state.event_count == 1 &&
           PE_LoadU32(0x8009D24Cu) == 0u,
           "wait did not service the transfer in flight");
    ASSERT(PE_SpuRam_LoadU8(SPUFL_A_SPU) == 0x21u &&
           PE_SpuRam_LoadU8(SPUFL_A_SPU + 0x3Fu) == (uint8_t)(0x21u + 0x3Fu),
           "transfer data missing after the wait");
    PASS();
}

static void test_SPUFL_registry_wait_stall_stops(void)
{
    char err[512];
    TEST("SPUFL_registry_wait_stall_stops");
    SPUFL_Reset(SPUFL_A_SPU);
    PE_StoreU32(0x8009D24Cu, 1u);             /* busy, nothing in flight */
    ASSERT(SPUFL_Captured(SPUFL_CallWait, err, sizeof err),
           "cannot capture stderr");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "busy with nothing in flight did not stop");
    ASSERT(strstr(err, "SPU_DMA/STALL func_80085174") != NULL,
           "stop line does not name the stall");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u, "stall changed busy");
    PASS();
}

static void test_SPUFL_issue_delivers_pending_first(void)
{
    PeSpuDmaState state;
    TEST("SPUFL_issue_delivers_pending_first");
    SPUFL_IssueA(0x31u);
    B48A_Fill(SPUFL_B_SRC, 0x40u, 0x71u);
    ASSERT(func_80085EB4(SPUFL_B_SPU) == SPUFL_B_SPU, "second address setup");
    func_800850F4(SPUFL_B_SRC, 0x40u);
    PE_SpuDma_GetState(&state);
    ASSERT(SPUFL_Clean(), "issue stopped, aborted or took a boundary");
    /* The in-flight transfer completed with the callback its issuer left
     * (func_80085098: busy 0, slot 0), then 850C0 and the new issue ran. */
    ASSERT(state.event_count == 1 && state.pending == 1 &&
           state.source == SPUFL_B_SRC && state.destination == SPUFL_B_SPU,
           "the new issue is not the one in flight");
    ASSERT(PE_SpuRam_LoadU8(SPUFL_A_SPU) == 0x31u &&
           PE_SpuRam_LoadU8(SPUFL_B_SPU) == 0u,
           "first transfer did not complete before the second");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u &&
           PE_LoadU32(0x8009B434u) == PE_SPU_DMA_CALLBACK,
           "850C0 did not set busy and the identity for the new issue");
    ASSERT(PE_SpuDma_Service() == 1 && PE_LoadU32(0x8009D24Cu) == 0u &&
           PE_SpuRam_LoadU8(SPUFL_B_SPU) == 0x71u,
           "second transfer did not complete");
    PASS();
}

static void test_SPUFL_pending_unknown_callback_stops_issue(void)
{
    PeSpuDmaState state;
    TEST("SPUFL_pending_unknown_callback_stops_issue");
    SPUFL_IssueA(0x41u);
    PE_StoreU32(0x8009B434u, SPUFL_NONE);
    B48A_Fill(SPUFL_B_SRC, 0x40u, 0x81u);
    func_800850F4(SPUFL_B_SRC, 0x40u);
    PE_SpuDma_GetState(&state);
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == SPUFL_NONE,
           "the in-flight transfer's unknown callback did not stop");
    /* No 850C0 store and no issue after the stop. */
    ASSERT(PE_LoadU32(0x8009B434u) == SPUFL_NONE &&
           PE_LoadU32(0x8009D24Cu) == 1u,
           "850C0 ran after the stop");
    ASSERT(state.pending == 0 && state.event_count == 1 &&
           PE_SpuRam_LoadU8(SPUFL_A_SPU) == 0x41u,
           "the stop issued a second transfer or lost the first");
    PASS();
}

static void test_SPUFL_reverb_clear_delivers_pending(void)
{
    PeSpuDmaState state;
    int i, ok;
    TEST("SPUFL_reverb_clear_delivers_pending");
    SPUFL_IssueA(0x51u);
    B48A_Fill(SPUFL_C4C0, 0x80u, 0x90u);
    /* Mode 0: 0x80 bytes from D_8009C4C0 to 0xFFF0 << 3, one DMA chunk,
     * consumed through SpuInit's event. */
    ASSERT(PE_SpuReverb_ClearWorkArea(0) == 0, "reverb clear failed");
    PE_SpuDma_GetState(&state);
    ASSERT(SPUFL_Clean(), "reverb clear stopped or took a boundary");
    ASSERT(state.pending == 0 && state.event_count == 2,
           "in-flight transfer and clear were not both delivered");
    ASSERT(PE_SpuRam_LoadU8(SPUFL_A_SPU) == 0x51u &&
           PE_LoadU32(0x8009D24Cu) == 0u,
           "in-flight transfer did not complete with its identity callback");
    ok = 1;
    for (i = 0; i < 0x80; i++)
        ok &= PE_SpuRam_LoadU8(0x7FF80u + (uint32_t)i) == (uint8_t)(0x90u + i);
    ASSERT(ok, "reverb work area not written");
    /* The slot is read after the delivery cleared it: nothing to restore. */
    ASSERT(PE_LoadU32(0x8009B434u) == 0u && PE_LoadU32(0x8009B418u) == 0u,
           "callback slot or transfer mode not restored as read");
    PASS();
}

static void test_SPUFL_reverb_stops_after_pending_callback(void)
{
    unsigned direct;
    TEST("SPUFL_reverb_stops_after_pending_callback");
    for (direct = 0; direct < 2u; direct++) {
        SPUFL_IssueA(0x51u);
        PE_StoreU32(0x8009B434u, SPUFL_NONE);
        PE_StoreU32(0x8009B464u, 0u);
        PE_StoreU32(0x8009B46Cu, 0x4321u);
        PE_StoreU32(0x8009B3A0u, 1u);
        PE_SpuRegister_StoreU16(0x1A2u, 0xABCDu);
        if (direct) (void)func_8008CF70(0x100);
        else func_8008CB54(0u);
        ASSERT(PE_Port_ShouldStop() && PE_SpuRegister_LoadU16(0x1A2u) == 0xABCDu,
               "reverb address changed after a pending callback stopped");
        ASSERT(!(PE_SpuRegister_LoadU16(0x1AAu) & 0x80u),
               "reverb re-enabled after a pending callback stopped");
    }
    PASS();
}

static void test_SPUFL_io_mode_issue_writes(void)
{
    PeSpuDmaState state;
    char err[512];
    TEST("SPUFL_io_mode_issue_writes");
    SPUFL_Reset(SPUFL_A_SPU);
    PE_StoreU32(0x8009B418u, 1u);           /* SpuSetTransferMode(1) */
    PE_StoreU32(0x8009B3FCu, 0x1F801C00u);  /* real SPU register base for I/O */
    B48A_Fill(SPUFL_A_SRC, 0x40u, 0x61u);
    ASSERT(SPUFL_Captured(SPUFL_CallIssueA, err, sizeof err),
           "cannot capture stderr");
    PE_SpuDma_GetState(&state);
    /* Programmed I/O runs natively since 48f28e22e (black-box observations,
     * docs/evidence/native-spu-pio-write-20261028): data, no DMA4. */
    ASSERT(!PE_Port_ShouldStop(), "programmed-I/O transfer stopped");
    ASSERT(state.pending == 0, "an I/O-mode issue started DMA4");
    for (uint32_t i = 0; i < 0x40u; i++)
        ASSERT(PE_SpuRam_LoadU8(SPUFL_A_SPU + i) == (uint8_t)(0x61u + i),
               "programmed-I/O data did not reach SPU RAM");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u &&
           PE_LoadU32(0x8009B434u) == PE_SPU_DMA_CALLBACK,
           "850C0 did not run before the issue, as in retail");
    PASS();
}

static void test_SPUFL_bad_shift_issue_stops(void)
{
    PeSpuDmaState state;
    char err[512];
    TEST("SPUFL_bad_shift_issue_stops");
    SPUFL_Reset(SPUFL_A_SPU);
    PE_StoreU32(0x8009B424u, 32u);
    ASSERT(SPUFL_Captured(SPUFL_CallIssueA, err, sizeof err),
           "cannot capture stderr");
    PE_SpuDma_GetState(&state);
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "an unrepresentable transfer-address shift did not stop");
    ASSERT(strstr(err, "SPU_DMA/REFUSED func_8007D9F8: transfer-address shift 32") != NULL,
           "stop line does not name the shift");
    ASSERT(state.pending == 0, "a refused issue started DMA4");
    PASS();
}
