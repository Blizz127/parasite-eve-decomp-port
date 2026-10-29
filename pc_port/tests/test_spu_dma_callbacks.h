/* func_8007D614's completion arms on the live D_8009B434, and the
 * func_80085174 wait adaptation, without abort().  No disc data or CPU
 * interpreter.  Retail reads the slot in the DMA4 IRQ, so the retail-shaped
 * fixture issues through func_800850F4 (identity installed) and changes the
 * slot before completion.  func_80085F44 (SpuSetTransferCallback, a
 * registered generated leaf) is the registry witness: it stores its $a0. */
#define SPUCB_SRC   0x80124000u
#define SPUCB_SPU   0x2600u
#define SPUCB_NONE  0x80012222u   /* no native code at this address */

static void SPUCB_Issue(uint8_t seed)
{
    ResetTestState();
    B48A_PrepareDma(SPUCB_SPU);
    B48A_Fill(SPUCB_SRC, 0x40u, seed);
    func_800850F4(SPUCB_SRC, 0x40u);
    g_stub_order_count = 0;
    Bootstrap_ResetArg4CallLog();
}

static void test_SPUCB_registry_callback(void)
{
    PeSpuDmaState state;
    TEST("SPUCB_registry_callback");
    SPUCB_Issue(0x21u);
    PE_StoreU32(0x8009B434u, 0x80085F44u);
    ASSERT(PE_SpuDma_Service() == 1, "completion did not run");
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count &&
           !g_bootstrap_arg4_call_count,
           "registered callback stopped or took a boundary");
    ASSERT(PE_LoadU32(0x8009B434u) == 0xF0000000u,
           "callee did not receive the jalr's $a0 (0xF0000000)");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u,
           "busy changed although the callee never clears it");
    ASSERT(PE_SpuRam_LoadU8(SPUCB_SPU) == 0x21u,
           "data did not move before the callback");
    PE_SpuDma_GetState(&state);
    ASSERT(state.pending == 0 && state.callback_order > state.data_order,
           "callback did not follow the data");
    PASS();
}

static void test_SPUCB_unknown_callback_stops(void)
{
    PeSpuDmaState state;
    TEST("SPUCB_unknown_callback_stops");
    SPUCB_Issue(0x31u);
    PE_StoreU32(0x8009B434u, SPUCB_NONE);
    ASSERT(PE_SpuDma_Service() == 1, "completion with a stopped IRQ tail");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "unknown callback did not stop");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == SPUCB_NONE &&
           g_bootstrap_arg4_calls[0].arg0 == 0xF0000000u &&
           CountOrderLog("D_8009B434") == 1,
           "boundary does not name the slot, the target and retail $a0");
    ASSERT(PE_SpuRam_LoadU8(SPUCB_SPU) == 0x31u,
           "data must move before the IRQ dispatch");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u &&
           PE_LoadU32(0x8009B434u) == SPUCB_NONE,
           "stop changed the busy flag or the slot");
    PE_SpuDma_GetState(&state);
    ASSERT(state.pending == 0 && state.callback_order == 0,
           "refused dispatch was counted as a callback");
    PASS();
}

static void test_SPUCB_null_callback_no_event(void)
{
    int handle;
    TEST("SPUCB_null_callback_no_event");
    /* No SpuInit: no SPU completion event exists.  Issue does not read the
     * slot (7D9F8), and DeliverEvent with no enabled event does nothing. */
    ResetTestState();
    PE_SpuDma_InstallIrq(PE_SPU_DMA_IRQ_HANDLER);
    B48A_Fill(0x80150000u, 0x40u, 0x41u);
    ASSERT(PE_SpuDma_Begin(0x80150000u, 0x3000u, 0x40u, 0),
           "issue refused a null callback slot");
    ASSERT(PE_SpuDma_Service() == 1 && !PE_Port_ShouldStop(),
           "DeliverEvent with no event was not a no-op");
    ASSERT(PE_SpuRam_LoadU8(0x3000u) == 0x41u && !PE_LoadU32(0x8009D24Cu),
           "no-op delivery lost the data or invented busy state");
    /* Retail-shaped: the identity issued, the live slot cleared, the event
     * disabled (SpuQuit's DisableEvent) before the IRQ. */
    SPUCB_Issue(0x51u);
    handle = (int)PE_LoadU32(0x8009B384u);
    PE_StoreU32(0x8009B434u, 0);
    PE_Event_Disable(handle);
    ASSERT(PE_SpuDma_Service() == 1 && !PE_Port_ShouldStop(),
           "disabled event delivery was not a no-op");
    PE_Event_Enable(handle);
    ASSERT(!PE_Event_ConsumeSpuDma(handle),
           "a disabled event recorded the delivery");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u && !g_stub_order_count,
           "no-op delivery cleared busy or took a boundary");
    PASS();
}

static void test_SPUCB_wait_stall_stops(void)
{
    TEST("SPUCB_wait_stall_stops");
    ResetTestState();
    B48A_PrepareDma(SPUCB_SPU);
    PE_StoreU32(0x8009D24Cu, 1u);             /* busy, nothing in flight */
    PE_SpuDma_WaitForCompletion();
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "busy flag with no transfer did not stop");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u && !g_bootstrap_arg4_call_count,
           "stall changed busy or logged a callback boundary");
    PASS();
}

static void test_SPUCB_wait_returns_on_callback_stop(void)
{
    TEST("SPUCB_wait_returns_on_callback_stop");
    SPUCB_Issue(0x61u);
    PE_StoreU32(0x8009B434u, SPUCB_NONE);
    PE_SpuDma_WaitForCompletion();
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           PE_LoadU32(0x8009D24Cu) == 1u,
           "wait did not return on the callback's stop");
    ASSERT(g_bootstrap_arg4_call_count == 1 && CountOrderLog("D_8009B434") == 1,
           "wait serviced again or reported a stall after the stop");
    PASS();
}

static void test_SPUCB_upload_stop_skips_guest_writes(void)
{
    PeSpuDmaState state;
    TEST("SPUCB_upload_stop_skips_guest_writes");
    ResetTestState();
    B48A_Seed851A8(0x80150000u);              /* valid header and payload */
    B48A_Fill(SPUCB_SRC, 0x40u, 0x71u);
    func_800850F4(SPUCB_SRC, 0x40u);           /* a transfer in flight */
    PE_StoreU32(0x8009B434u, SPUCB_NONE);
    PE_StoreU8(0x800B2900u, 0xCCu);
    ASSERT(func_800851A8(0x80150000u, 1) == 0,
           "stopped upload did not return retail's success value");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "first barrier did not stop");
    ASSERT(PE_LoadU8(0x800B2900u) == 0xCCu,
           "payload copy ran after the stop");
    PE_SpuDma_GetState(&state);
    ASSERT(state.pending == 0 && state.event_count == 1 &&
           state.source == SPUCB_SRC,
           "a second transfer was issued after the stop");
    ASSERT(PE_LoadU32(0x8009D24Cu) == 1u, "busy changed after the stop");
    PASS();
}

static void test_SPUCB_stream_bringup_stop(void)
{
    TEST("SPUCB_stream_bringup_stop");
    ResetTestState();
    for (uint32_t i = 0; i < 0x40u; i++)
        PE_StoreU8(0x8009B7FCu + i, (uint8_t)(0x40u + i));
    PE_Port_RequestStop(PE_PORT_STOP_EXPLICIT);
    func_80085644();
    /* Since 299fdc38e a pre-existing stop prevents the issue itself. */
    ASSERT(PE_LoadU32(0x8009D24Cu) == 0u,
           "bring-up issued a transfer after a pre-existing stop");
    ASSERT(PE_LoadU32(0x800BCD64u) == 0 && !(PE_LoadU32(0x8009D2C4u) & 0x80u) &&
           PE_LoadU32(0x8009CDE0u) == 0,
           "state init or the event tail ran after the stop");
    PASS();
}

static void test_SPUCB_cda4_stop_after_service(void)
{
    int result;
    TEST("SPUCB_cda4_stop_after_service");
    /* State 10 sees the flag clear, moves to state 7 with nothing left, and
     * then services a transfer whose live callback has no native code.
     * Without the stop check the loop runs state 7 again (f0 = 0, return 0). */
    ResetTestState();
    B48A_PrepareDma(SPUCB_SPU);
    ASSERT(PE_SpuDma_Begin(SPUCB_SRC, SPUCB_SPU, 0x40u, 0),
           "fixture transfer was refused");
    PE_StoreU32(0x8009B434u, SPUCB_NONE);
    PE_StoreU8(0x800B0CD8u + 0xF0u, 10u);
    PE_StoreU32(0x8009D178u, 5u);
    PE_StoreU32(0x8009D17Cu, 5u);
    result = func_8006CDA4(1, 0, 0, 0x80140000u, 0x10, 1);
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "service did not stop");
    ASSERT(result == 1 && PE_LoadU8(0x800B0CD8u + 0xF0u) == 7u &&
           PE_LoadU32(0x8009D178u) == 0,
           "loop ran another state after the stop");
    PASS();
}
