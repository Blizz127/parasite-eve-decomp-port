/* Fixed-address hook words dispatched like retail's jalr: an installed
 * identity runs directly, any other value through the guest-code registry
 * (PE_GuestHook_Call), and a value with no C stops at a named boundary.
 * No disc data or CPU interpreter.  func_80085F44 (SpuSetTransferCallback,
 * a registered generated leaf) is the witness: it stores its $a0 into
 * D_8009B434 when that differs, so every positive case preseeds a sentinel.
 * Registers are asserted only where retail's are established; elsewhere the
 * host passes 0 and the tests check only that the callee ran. */
#define HOOKREG_WITNESS  0x80085F44u   /* func_80085F44: D_8009B434 = $a0 */
#define HOOKREG_SLOT     0x8009B434u
#define HOOKREG_SENTINEL 0x5A5A5A5Au
#define HOOKREG_NONE     0x80012222u   /* no native code at this address */
#define HOOKREG_DICR_PTR 0x800956BCu   /* .data word, retail 0x1F8010F4 */

/* DMA channels 0 and `later` pending, slot 0 = `first`, and channel 3's
 * identity func_8007C214 primed so its run is visible. */
static void HOOKREG_SeedDma(pe_addr_t first, uint32_t later, pe_addr_t later_cb)
{
    ResetTestState();
    PE_StoreU32(HOOKREG_DICR_PTR, 0x1F8010F4u);
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
    PE_StoreU32(0x800C0DC8u, 0x801F0000u);
    PE_StoreU32(0x800BE9E4u, 0u);
    PE_StoreU32(0x800BE998u, 0u);
    PE_StoreU32(0x800B89F4u, 0xA5A5A5A5u);
    PE_GPU_WriteDICR(PE_GPU_DICR_MASTER | 0x00010000u | (0x00010000u << later));
    (void)PE_GPU_LatchDMACompletionFlag(0u);
    (void)PE_GPU_LatchDMACompletionFlag(later);
    (void)PE_GPU_TakeDICRRisingEdge();
    PE_StoreU32(PE_DMA_CallbackSlotAddress(0u), first);
    PE_StoreU32(PE_DMA_CallbackSlotAddress(later), later_cb);
    PE_IRQ_DeliveryTraceReset();
}

static void test_HOOKREG_dma_registry_continues(void)
{
    PeIrqDeliveryTrace trace;
    TEST("HOOKREG_dma_registry_continues");
    HOOKREG_SeedDma(HOOKREG_WITNESS, 3u, 0x8007C214u);
    ASSERT(PE_func_80074520_Dispatch() == PE_IRQ_SERVICE_RETURNED &&
           !PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "registered DMA callback stopped or took a boundary");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == 0x1F8010F4u,
           "callee did not receive the jalr's $a0 (D_800956BC)");
    PE_IRQ_GetDeliveryTrace(&trace);
    ASSERT(trace.dma_ack_count == 2u && trace.dma_ack_channels[0] == 0u &&
           trace.dma_ack_channels[1] == 3u,
           "scan did not continue to channel 3");
    ASSERT(PE_LoadU16(0x801F0000u) == 2u && PE_LoadU32(0x800B89F4u) == 0u,
           "channel 3's identity did not run after the registry callback");
    PASS();
}

static void test_HOOKREG_dma_unregistered_stops(void)
{
    TEST("HOOKREG_dma_unregistered_stops");
    HOOKREG_SeedDma(HOOKREG_NONE, 3u, 0x8007C214u);
    ASSERT(PE_func_80074520_Dispatch() == PE_IRQ_SERVICE_BOUNDARY &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "unknown DMA callback did not stop");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE &&
           g_bootstrap_arg4_calls[0].arg0 == 0x1F8010F4u &&
           CountOrderLog("func_80074520_dma_indirect_call") == 1,
           "boundary does not name the slot, the target and retail $a0");
    ASSERT(PE_LoadU32(0x800B89F4u) == 0xA5A5A5A5u,
           "scan continued past the stop");
    PASS();
}

/* SpuInit's DMA4 IRQ tail and DsRead's DMA3 completion have C, but host
 * models own their state, so the DMA dispatcher refuses them by name. */
static void test_HOOKREG_dma_model_owned_refused(void)
{
    static const pe_addr_t owned[] = {0x8007D614u, 0x80081110u};
    TEST("HOOKREG_dma_model_owned_refused");
    for (unsigned i = 0; i < 2u; i++) {
        ASSERT(PE_GuestCode_Resolve(owned[i]) != NULL,
               "fixture: the refused callback must have C in the registry");
        HOOKREG_SeedDma(0u, 3u, owned[i]);
        PE_StoreU32(HOOKREG_SLOT, HOOKREG_WITNESS);
        ASSERT(PE_func_80074520_Dispatch() == PE_IRQ_SERVICE_BOUNDARY &&
               PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
               "model-owned DMA callback did not stop");
        ASSERT(CountOrderLog("func_80074520_model_owned_callback") == 1 &&
               g_bootstrap_arg4_call_count == 1 &&
               g_bootstrap_arg4_calls[0].target == owned[i] &&
               g_bootstrap_arg4_calls[0].arg0 == 0x1F8010F4u,
               "model-owned refusal is not named");
        ASSERT(PE_LoadU32(HOOKREG_SLOT) == HOOKREG_WITNESS,
               "the refused callback's body ran");
    }
    PASS();
}

static void test_HOOKREG_cpu_registry_continues(void)
{
    PeIrqDeliveryTrace trace;
    PeIrqGeneration generation;
    TEST("HOOKREG_cpu_registry_continues");
    ResetTestState();
    func_80073C94();
    generation = PE_IRQ_Generation();
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
    PE_StoreU32(B53I_IRQ_SOURCE0_SLOT, HOOKREG_WITNESS);
    PE_IRQ_DeliveryTraceReset();
    PE_IRQ_AssertSources(0x0009u);
    ASSERT(PE_IRQ_ServicePendingForGeneration(generation) ==
               PE_IRQ_SERVICE_RETURNED &&
           !PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "registered CPU callback stopped or took a boundary");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == 0x0009u,
           "the pass's first handler did not receive the I_STAT read in $a0");
    PE_IRQ_GetDeliveryTrace(&trace);
    ASSERT(trace.cpu_ack_count == 2u && trace.cpu_ack_sources[0] == 0u &&
           trace.cpu_ack_sources[1] == 3u && trace.dma_dispatch_entries == 1u,
           "scan did not continue to source 3's identity");
    ASSERT(PE_IRQ_ReadStatus() == 0u &&
           PE_LoadU16(B53I_IRQ_DISPATCH_ACTIVE) == 0u,
           "normal terminal path was not reached");
    PASS();
}

static void test_HOOKREG_cpu_unregistered_stops(void)
{
    PeIrqGeneration generation;
    TEST("HOOKREG_cpu_unregistered_stops");
    ResetTestState();
    func_80073C94();
    generation = PE_IRQ_Generation();
    PE_StoreU32(B53I_IRQ_SOURCE3_SLOT, HOOKREG_NONE);
    PE_IRQ_AssertSources(0x0008u);
    ASSERT(PE_IRQ_ServicePendingForGeneration(generation) ==
               PE_IRQ_SERVICE_BOUNDARY &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "unknown CPU callback did not stop");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE &&
           g_bootstrap_arg4_calls[0].arg0 == 0x0008u &&
           CountOrderLog("func_80073F00_cpu_indirect_call") == 1,
           "boundary does not name the slot, the target and retail $a0");
    ASSERT(PE_LoadU16(B53I_IRQ_DISPATCH_ACTIVE) == 1u,
           "stopped scan reached the terminal path");
    PASS();
}

/* Lane 3 with nothing pending: the tail after the callback issues command 1
 * through func_8007FCFC (B558 live chain, as test_B558_7fcfc_*). */
static void HOOKREG_SeedVblank(pe_addr_t callback)
{
    ResetTestState();
    B558_PlantPointers();
    PE_StoreU32(0x8009AFC0u, 0u);
    PE_StoreU16(0x800945E6u, 0u);
    PE_StoreU32(0x8009B574u, 3u);
    PE_StoreU32(0x8009B554u, 1u);
    PE_StoreU32(0x800A36A0u, callback);
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
}

static void test_HOOKREG_cd_vblank_registry_tail(void)
{
    TEST("HOOKREG_cd_vblank_registry_tail");
    HOOKREG_SeedVblank(HOOKREG_WITNESS);
    func_8007FE24();
    ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "registered VBlank callback stopped or took a boundary");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) != HOOKREG_SENTINEL,
           "registered VBlank callback did not run");
    ASSERT(PE_LoadU32(0x8009B570u) == 33u && PE_LoadU8(0x8009B558u) == 1u &&
           PE_LoadU32(0x8009B598u) == 0x1Eu,
           "lane tail did not follow the callback");

    HOOKREG_SeedVblank(HOOKREG_NONE);
    func_8007FE24();
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE &&
           CountOrderLog("CD_vblank_callback") == 1,
           "unknown VBlank callback did not stop by name");
    ASSERT(PE_LoadU32(0x8009B570u) == 0u, "lane tail ran after the stop");
    PASS();
}

/* Stream oracle cases 50 and 54 reach the D_800B0CCC jalr at 0x8007CA90
 * (status 7 follows) and 0x8007CB34 (status 8, $a0 = D_800A34A0). */
static void test_HOOKREG_cd_aux_registry_continues(void)
{
    static const struct { unsigned k; uint32_t status; int rec_a0; } c[] = {
        {50u, 7u, 0}, {54u, 8u, 1}};
    TEST("HOOKREG_cd_aux_registry_continues");
    for (unsigned i = 0; i < 2u; i++) {
        pe_addr_t rec;
        uint32_t advance;
        ASSERT(CDSTREAM_cases[c[i].k].boundary == 4u &&
               (CDSTREAM_cases[c[i].k].init[14] != 0u) == !c[i].rec_a0,
               "fixture: case is not the expected auxiliary-hook site");
        CDSTREAM_SeedCase(c[i].k);
        PE_StoreU32(0x800B0CCCu, HOOKREG_WITNESS);
        PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
        rec = PE_LoadU32(0x800C0DC8u) + (PE_LoadU32(0x800BE998u) << 5);
        advance = PE_LoadU32(0x800BCD7Cu) + 1u;
        func_8007C564();
        ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
               "registered auxiliary callback stopped or took a boundary");
        ASSERT(PE_LoadU32(0x8009B374u) == c[i].status &&
               PE_LoadU32(0x800BCD7Cu) == advance,
               "stream advance and status did not follow the callback");
        if (c[i].rec_a0)
            ASSERT(PE_LoadU32(HOOKREG_SLOT) == rec,
                   "callee did not receive the jalr's $a0 (D_800A34A0)");
        else
            ASSERT(PE_LoadU32(HOOKREG_SLOT) != HOOKREG_SENTINEL,
                   "registered auxiliary callback did not run");
    }
    PASS();
}

static void HOOKREG_SeedChain(pe_addr_t callback)
{
    ResetTestState();
    PE_StoreU32(0x800C0DC8u, 0x801F0000u);
    PE_StoreU32(0x800BE9E4u, 0u);
    PE_StoreU32(0x800BE998u, 0u);
    PE_StoreU32(0x800B0CC8u, callback);
    PE_StoreU32(0x800B89F4u, 1u);
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
}

static void test_HOOKREG_b0cc8_chain_then_clear(void)
{
    TEST("HOOKREG_b0cc8_chain_then_clear");
    HOOKREG_SeedChain(HOOKREG_WITNESS);
    func_8007C214();
    ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "registered chain callback stopped or took a boundary");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == HOOKREG_WITNESS,
           "callee did not receive the jalr's $a0 (the callback itself)");
    ASSERT(PE_LoadU32(0x800B89F4u) == 0u && PE_LoadU16(0x801F0000u) == 2u,
           "D_800B89F4 clear did not follow the callback");

    HOOKREG_SeedChain(HOOKREG_NONE);
    func_8007C214();
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE &&
           g_bootstrap_arg4_calls[0].arg0 == HOOKREG_NONE &&
           g_bootstrap_arg4_calls[0].arg2 == 0x800A3490u &&
           CountOrderLog("func_8007C214_B0CC8_chain") == 1,
           "unknown chain callback did not stop with retail $a0/$a2");
    ASSERT(PE_LoadU32(0x800B89F4u) == 1u, "clear ran after the stop");
    PASS();
}

static void test_HOOKREG_drawsync_registry_returns(void)
{
    PeGpuPumpTrace trace;
    pe_addr_t entry;
    int returned = 0;
    TEST("HOOKREG_drawsync_registry_returns");
    ResetTestState();
    entry = B53I_C_SeedIdleEntry(0u, 1u, 0x80076664u, 1, HOOKREG_WITNESS);
    PE_StoreU32(entry + 4u, 0u); /* ordinary -1; no DMA becomes busy */
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
    PE_Pump_TraceReset();
    ASSERT(PE_func_80076EE4_Pump(&returned) == 0 && returned == 1,
           "registered DrawSync callback did not return through the pump");
    ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "registered DrawSync callback stopped or took a boundary");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == HOOKREG_WITNESS,
           "callee did not receive the jalr's $a0 (the callback itself)");
    PE_Pump_GetTrace(&trace);
    ASSERT(trace.drawsync_calls == 1u &&
           trace.drawsync_clear_order < trace.drawsync_call_order &&
           PE_LoadU32(B53D_WORK_MARKER) == 0u,
           "work-marker clear did not precede the callback");
    PASS();
}

/* func_80050020 (registered generated getter) returns D_800A1888[$a0]: a
 * record hook other than the identity. */
static void test_HOOKREG_libpad_registry_hooks(void)
{
    TEST("HOOKREG_libpad_registry_hooks");
    ResetTestState();
    PE_StoreU32(0x8009B738u, 0x80050020u);
    PE_StoreU32(0x800A1888u + 4u, 0x80150000u);
    PE_StoreU32(0x80150000u + 0x34u, 0x10000u);
    PE_StoreU8(0x80150000u + 0x49u, 6u);
    ASSERT(func_800825C0(1u) == 4 && !PE_Port_ShouldStop() &&
           !g_bootstrap_arg4_call_count,
           "caller did not read through the registry hook's record");

    ResetTestState();
    PE_StoreU32(0x8009B738u, 0x80084B20u);
    PE_StoreU32(0x8009B740u, HOOKREG_WITNESS);   /* returns 0: not busy */
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
    ASSERT(func_800828F4(0u, 0x80160000u) == 1 && !PE_Port_ShouldStop() &&
           !g_bootstrap_arg4_call_count,
           "not-busy registry hook did not let the request run");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == 0x800A5B70u,
           "busy hook did not receive the record in $a0");
    ASSERT(PE_LoadU8(0x800A5B70u + 0x46u) == 1u &&
           PE_LoadU32(0x800A5B70u + 0x20u) == 0x80160000u,
           "request fields were not written after the not-busy return");
    PASS();
}

static void test_HOOKREG_libpad_stops_are_named(void)
{
    TEST("HOOKREG_libpad_stops_are_named");
    /* A registry hook's NULL record has no retail value to continue with. */
    ResetTestState();
    PE_StoreU32(0x8009B738u, HOOKREG_WITNESS);
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
    ASSERT(func_800825C0(5u) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "NULL record from a registry hook did not stop");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == 5u,
           "record hook did not receive the port in $a0");
    ASSERT(CountOrderLog("D_8009B738_null_record") == 1 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_WITNESS &&
           g_bootstrap_arg4_calls[0].arg0 == 5u,
           "NULL-record stop is not named");

    ResetTestState();
    PE_StoreU32(0x8009B738u, HOOKREG_NONE);
    ASSERT(func_800825C0(2u) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           CountOrderLog("D_8009B738_record_hook") == 1 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE &&
           g_bootstrap_arg4_calls[0].arg0 == 2u,
           "unknown record hook stop does not name the target and port");

    ResetTestState();
    PE_StoreU32(0x8009B738u, 0x80084B20u);
    PE_StoreU32(0x8009B740u, HOOKREG_NONE);
    PE_StoreU32(0x800A5B84u, 0x12345678u);
    ASSERT(func_800828F4(0u, 0x80160000u) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           CountOrderLog("D_8009B740_busy_hook") == 1 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE &&
           g_bootstrap_arg4_calls[0].arg0 == 0x800A5B70u,
           "unknown busy hook stop does not name the target and record");
    ASSERT(PE_LoadU32(0x800A5B84u) == 0x12345678u,
           "stopped busy check wrote the request");
    PASS();
}
