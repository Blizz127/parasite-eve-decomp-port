/* Worker and dynamic callback arms dispatched like retail's jalr: a value
 * other than the hand-ported identities runs through the guest-code registry
 * (PE_GuestHook_Call), and a value with no C stops at a named boundary.
 *   - func_80076C34's direct issue: jalr s3 at 0x80076D38, a0 = argument,
 *     a1 = auxiliary;
 *   - the queue pump's ring worker: jalr v0 at 0x80077034, a0/a1 = the
 *     entry's +0x04/+0x08 words;
 *   - the m28/m32 movement update: 801917FC/801917EC jalr the slot+12
 *     callback with a0 = slot.
 * No disc data or CPU interpreter.  Registered generated leaves are the
 * witnesses: func_800528C4 stores a1*60 at D_800A76A4[a0*3], so two small
 * integers pin both registers; func_80085F44 stores its a0 (HOOKREG_SLOT);
 * func_80042BD8 sets D_800A1820 = 1 (a DrawSync callback that leaves
 * HOOKREG_SLOT alone).  func_8007C214 with an unregistered D_800B0CC8 is a
 * registered callee that stops by its own named refusal. */
#define WKREG_PAIR      0x800528C4u   /* func_800528C4(a0, a1) */
#define WKREG_PAIR_A0   2u
#define WKREG_PAIR_A1   5u
#define WKREG_PAIR_HIT  (0x800A76A4u + WKREG_PAIR_A0 * 12u)
#define WKREG_PAIR_SWAP (0x800A76A4u + WKREG_PAIR_A1 * 12u)
#define WKREG_PAIR_ZERO 0x800A76A4u   /* where a0 = 0 would store */
#define WKREG_STOPPER   0x8007C214u
#define WKREG_DRAWSYNC  0x80042BD8u   /* func_80042BD8: D_800A1820 = 1 */
#define WKREG_DRAWSYNC_FLAG 0x800A1820u
#define WKREG_PUMP_SAVED_IMASK 0x80095880u   /* written only by the pump */
#define WKREG_M28_SLOT  0x80150000u

static int WKREG_Registered(pe_addr_t fn)
{
    const PeGuestCodeEntry *e = PE_GuestCode_Resolve(fn);
    return e != NULL && e->thunk != NULL;
}

static void WKREG_SeedPair(void)
{
    PE_StoreU32(WKREG_PAIR_HIT, HOOKREG_SENTINEL);
    PE_StoreU32(WKREG_PAIR_SWAP, HOOKREG_SENTINEL);
    PE_StoreU32(WKREG_PAIR_ZERO, HOOKREG_SENTINEL);
}

/* HOOKREG_SeedChain's state with an unregistered chain hook. */
static void WKREG_ArmStopper(void)
{
    PE_StoreU32(0x800C0DC8u, 0x801F0000u);
    PE_StoreU32(0x800BE9E4u, 0u);
    PE_StoreU32(0x800BE998u, 0u);
    PE_StoreU32(0x800B0CC8u, HOOKREG_NONE);
    PE_StoreU32(0x800B89F4u, 1u);
}

/* Direct-issue state: initialized, empty ring, idle DMA2, no DrawSync
 * callback, prior I_MASK 0xBEEF. */
static void WKREG_SeedDirect(void)
{
    ResetTestState();
    PE_StoreU8(B53D_INIT_BYTE, 1u);
    (void)PE_IRQ_ExchangeMask(0xBEEFu);
}

/* The direct arm ran, not the enqueue and the pump (which a queued worker
 * would also reach): the ring never moved and the pump never saved a mask. */
static int WKREG_TookDirectArm(void)
{
    return PE_LoadU32(B53C_PRODUCER) == 0u && PE_LoadU32(B53C_CONSUMER) == 0u &&
           PE_LoadU32(WKREG_PUMP_SAVED_IMASK) == 0u;
}

static void test_WKREG_direct_registry_restores(void)
{
    unsigned epoch;
    TEST("WKREG_direct_registry_restores");
    ASSERT(WKREG_Registered(WKREG_PAIR),
           "fixture: func_800528C4 must have a register thunk");
    WKREG_SeedDirect();
    WKREG_SeedPair();
    ASSERT(func_80076C34(WKREG_PAIR, WKREG_PAIR_A0, 0, WKREG_PAIR_A1) == 0,
           "direct issue did not return retail zero");
    ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "registered worker stopped or took a boundary");
    ASSERT(PE_LoadU32(WKREG_PAIR_HIT) == WKREG_PAIR_A1 * 60u &&
           PE_LoadU32(WKREG_PAIR_SWAP) == HOOKREG_SENTINEL,
           "worker did not receive a0 = argument and a1 = auxiliary");
    ASSERT(PE_LoadU32(B53D_SAVED_IMASK) == 0xBEEFu &&
           PE_IRQ_GetMask() == 0xBEEFu,
           "the saved I_MASK was not restored after the worker");
    ASSERT(WKREG_TookDirectArm(), "the worker was queued, not issued directly");

    /* An unrelated stop latched earlier must not suppress the tail. */
    WKREG_SeedDirect();
    WKREG_SeedPair();
    PE_Port_RequestStop(PE_PORT_STOP_FRAME_LIMIT);
    epoch = PE_Port_StopEpoch();
    ASSERT(func_80076C34(WKREG_PAIR, WKREG_PAIR_A0, 0, WKREG_PAIR_A1) == 0 &&
           PE_Port_StopEpoch() == epoch &&
           PE_Port_GetStopReason() == PE_PORT_STOP_FRAME_LIMIT,
           "a latched frame stop fabricated a new stop");
    ASSERT(PE_LoadU32(WKREG_PAIR_HIT) == WKREG_PAIR_A1 * 60u &&
           PE_IRQ_GetMask() == 0xBEEFu && WKREG_TookDirectArm(),
           "a latched frame stop suppressed the worker or the mask restore");
    PASS();
}

static void test_WKREG_direct_callee_stop_skips_restore(void)
{
    TEST("WKREG_direct_callee_stop_skips_restore");
    ASSERT(WKREG_Registered(WKREG_STOPPER),
           "fixture: func_8007C214 must have a register thunk");
    WKREG_SeedDirect();
    WKREG_ArmStopper();
    ASSERT(func_80076C34(WKREG_STOPPER, 0u, 0, 0u) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "the worker's stop was lost");
    ASSERT(CountOrderLog("func_8007C214_B0CC8_chain") == 1 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE,
           "the stop is not the worker's own named refusal");
    ASSERT(PE_IRQ_GetMask() == 0u && PE_LoadU32(0x800B89F4u) == 1u,
           "the worker's tail or the mask restore ran after the stop");
    ASSERT(WKREG_TookDirectArm(), "the worker was queued, not issued directly");
    PASS();
}

/* Control: behaviour the parent already had, kept.  No C at the worker
 * (including zero) is a named stop with retail a0/a1, and an inline8
 * caller's worker is refused even when it has C (its retail a0 is a
 * caller-stack RECT).  None restores the mask. */
static void test_WKREG_direct_stops_unchanged(void)
{
    static const pe_addr_t none[] = {HOOKREG_NONE, 0u};
    static const char *const name[] = {"func_80012222", "func_00000000"};
    TEST("WKREG_direct_stops_unchanged");
    for (unsigned i = 0; i < 2u; i++) {
        WKREG_SeedDirect();
        ASSERT(func_80076C34(none[i], 0x80100000u, 0, 0x80110000u) == 0 &&
               PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
               "unknown direct worker did not stop");
        ASSERT(g_bootstrap_arg4_call_count == 1 &&
               g_bootstrap_arg4_calls[0].target == none[i] &&
               g_bootstrap_arg4_calls[0].arg0 == 0x80100000u &&
               g_bootstrap_arg4_calls[0].arg1 == 0x80110000u &&
               strcmp(g_bootstrap_arg4_calls[0].caller, "func_80076C34") == 0 &&
               CountOrderLog(name[i]) == 1,
               "boundary does not name the worker and retail a0/a1");
        ASSERT(PE_IRQ_GetMask() == 0u, "stopped direct issue restored the mask");
    }

    WKREG_SeedDirect();
    WKREG_SeedPair();
    ASSERT(PE_func_80076C34_Inline8(WKREG_PAIR, 0x00100020u, 0x00080010u,
                                    WKREG_PAIR_A1) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "an inline8 caller's worker with C did not stop");
    ASSERT(g_bootstrap_arg4_call_count == 2 &&
           g_bootstrap_arg4_calls[1].target == WKREG_PAIR &&
           g_bootstrap_arg4_calls[1].arg1 == WKREG_PAIR_A1 &&
           strcmp(g_bootstrap_arg4_calls[1].caller, "func_80076C34") == 0,
           "inline8 refusal does not name the worker");
    ASSERT(PE_LoadU32(WKREG_PAIR_ZERO) == HOOKREG_SENTINEL &&
           PE_LoadU32(WKREG_PAIR_HIT) == HOOKREG_SENTINEL &&
           PE_IRQ_GetMask() == 0u,
           "the refused worker ran or the mask was restored");
    PASS();
}

static void test_WKREG_pump_registry_advances(void)
{
    PeGpuPumpTrace trace;
    pe_addr_t entry;
    unsigned epoch;
    int returned = 0;
    TEST("WKREG_pump_registry_advances");
    for (unsigned latched = 0; latched < 2u; latched++) {
        ResetTestState();
        entry = B53I_C_SeedIdleEntry(0u, 1u, WKREG_PAIR, 1, 0u);
        PE_StoreU32(entry + 4u, WKREG_PAIR_A0);
        PE_StoreU32(entry + 8u, WKREG_PAIR_A1);
        WKREG_SeedPair();
        (void)PE_IRQ_ExchangeMask(0xBEEFu);
        /* Second pass: an unrelated stop latched earlier. */
        if (latched) PE_Port_RequestStop(PE_PORT_STOP_FRAME_LIMIT);
        epoch = PE_Port_StopEpoch();
        PE_Pump_TraceReset();
        returned = 0;
        ASSERT(PE_func_80076EE4_Pump(&returned) == 0 && returned == 1,
               "registered ring worker did not return through the pump");
        ASSERT(PE_Port_StopEpoch() == epoch && !g_bootstrap_arg4_call_count &&
               PE_Port_GetStopReason() ==
                   (latched ? PE_PORT_STOP_FRAME_LIMIT : PE_PORT_STOP_NONE),
               "registered ring worker stopped or took a boundary");
        ASSERT(PE_LoadU32(WKREG_PAIR_HIT) == WKREG_PAIR_A1 * 60u &&
               PE_LoadU32(WKREG_PAIR_SWAP) == HOOKREG_SENTINEL,
               "worker did not receive the entry's +4/+8 words in a0/a1");
        PE_Pump_GetTrace(&trace);
        ASSERT(trace.worker_returned == 1 && PE_LoadU32(B53C_CONSUMER) == 1u &&
               trace.worker_call_order < trace.consumer_advance_order &&
               trace.consumer_advance_order < trace.mask_restore_order &&
               PE_IRQ_GetMask() == 0xBEEFu,
               "consumer advance and mask restore did not follow the worker");
    }
    PASS();
}

/* Retail has not returned from a worker that stopped, so the consumer must
 * not move and the mask must not be restored. */
static void test_WKREG_pump_callee_stop_holds_consumer(void)
{
    PeGpuPumpTrace trace;
    int returned = 1;
    TEST("WKREG_pump_callee_stop_holds_consumer");
    ResetTestState();
    (void)B53I_C_SeedIdleEntry(0u, 1u, WKREG_STOPPER, 1, 0u);
    WKREG_ArmStopper();
    (void)PE_IRQ_ExchangeMask(0xBEEFu);
    PE_Pump_TraceReset();
    ASSERT(PE_func_80076EE4_Pump(&returned) == 0 && returned == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "the worker's stop was lost");
    ASSERT(CountOrderLog("func_8007C214_B0CC8_chain") == 1 &&
           g_bootstrap_arg4_call_count == 1 &&
           g_bootstrap_arg4_calls[0].target == HOOKREG_NONE,
           "the stop is not the worker's own named refusal");
    PE_Pump_GetTrace(&trace);
    ASSERT(PE_LoadU32(B53C_CONSUMER) == 0u && !trace.worker_returned &&
           trace.consumer_advance_order == 0u &&
           trace.mask_restore_order == 0u && PE_IRQ_GetMask() == 0u,
           "the pump advanced or restored after the worker stopped");
    PASS();
}

/* A worker queued by func_80076C34 (a DrawSync callback forces the enqueue)
 * and drained by the pump at once (DMA2 idle).  The inline8 form shows the
 * queued payload is a guest copy, so the pump's worker gets its address. */
static void test_WKREG_queued_registry_drains(void)
{
    PeGpuPumpTrace trace;
    TEST("WKREG_queued_registry_drains");
    ASSERT(WKREG_Registered(WKREG_DRAWSYNC),
           "fixture: func_80042BD8 must have a register thunk");
    WKREG_SeedDirect();
    PE_StoreU32(B53D_DRAWSYNC_CB, WKREG_DRAWSYNC);
    PE_StoreU32(WKREG_DRAWSYNC_FLAG, 0u);
    WKREG_SeedPair();
    PE_Pump_TraceReset();
    ASSERT(func_80076C34(WKREG_PAIR, WKREG_PAIR_A0, 0, WKREG_PAIR_A1) == 0 &&
           !PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count,
           "queued registered worker stopped or took a boundary");
    ASSERT(PE_LoadU32(B53C_RING_BASE) == WKREG_PAIR &&
           PE_LoadU32(B53C_PRODUCER) == 1u && PE_LoadU32(B53C_CONSUMER) == 1u,
           "the worker was not queued and then drained");
    ASSERT(PE_LoadU32(WKREG_PAIR_HIT) == WKREG_PAIR_A1 * 60u &&
           PE_LoadU32(WKREG_PAIR_SWAP) == HOOKREG_SENTINEL,
           "the pump did not pass the queued a0/a1");
    PE_Pump_GetTrace(&trace);
    ASSERT(trace.worker_calls == 1u && trace.worker_returned == 1 &&
           PE_LoadU32(WKREG_DRAWSYNC_FLAG) == 1u && PE_IRQ_GetMask() == 0xBEEFu,
           "drain, DrawSync cleanup or mask restore did not follow");

    WKREG_SeedDirect();
    PE_StoreU32(B53D_DRAWSYNC_CB, WKREG_DRAWSYNC);
    PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
    ASSERT(PE_func_80076C34_Inline8(HOOKREG_WITNESS, 0x00100020u, 0x00080010u,
                                    0u) == 0 && !PE_Port_ShouldStop(),
           "an inline8 worker on the queued path stopped");
    ASSERT(g_bootstrap_arg4_call_count == 1 &&
           strcmp(g_bootstrap_arg4_calls[0].caller, "func_8007506C") == 0,
           "the queued inline8 worker took a boundary");
    ASSERT(PE_LoadU32(HOOKREG_SLOT) == B53C_RING_BASE + 0x0Cu &&
           PE_LoadU32(B53C_RING_BASE + 0x0Cu) == 0x00100020u &&
           PE_LoadU32(B53C_RING_BASE + 0x10u) == 0x00080010u,
           "the queued worker did not get the ring payload's guest address");
    PASS();
}

/* The m28 or m32 predicate words, so PE_EffectCallback's own selector
 * reaches the hand-ported update (m32's graph sits 16 bytes lower). */
static void WKREG_PlantMovementPredicate(uint32_t relocation)
{
    PE_StoreU32(0x8019151Cu + relocation, 0x24020001u);
    PE_StoreU32(0x801915ACu + relocation, 0x2CC2001Au);
    PE_StoreU32(0x80191894u + relocation, 0x27BDFFD8u);
    PE_StoreU32(0x801920D8u + relocation, 0xA0800003u);
}

/* Variants: 0 m28, 1 m32, 2 m28 through the selector, 3 m32 through it. */
static int WKREG_M28Update(unsigned variant)
{
    if (variant >= 2u) {
        const uint32_t relocation = variant == 3u ? 0xFFFFFFF0u : 0u;
        WKREG_PlantMovementPredicate(relocation);
        return PE_EffectCallback(0x801917FCu + relocation, 1, WKREG_M28_SLOT, 0u);
    }
    return variant ? PE_M32MovementUpdate(WKREG_M28_SLOT)
                   : PE_M28MovementUpdate(WKREG_M28_SLOT);
}

static void test_WKREG_m28_update_registry(void)
{
    static const char *const caller[] = {
        "func_801917FC", "func_801917EC", "func_801917FC", "func_801917EC"};
    static const pe_addr_t idle[] = {
        0x80191D10u, 0x80191D00u, 0x80191D10u, 0x80191D00u};
    static const pe_addr_t none[] = {HOOKREG_NONE, 0u};
    const pe_addr_t slot = WKREG_M28_SLOT;
    TEST("WKREG_m28_update_registry");
    for (unsigned v = 0; v < 4u; v++) {
        ResetTestState();
        PE_StoreU32(slot + 12u, HOOKREG_WITNESS);
        PE_StoreU32(HOOKREG_SLOT, HOOKREG_SENTINEL);
        ASSERT(WKREG_M28Update(v) == 0 && !PE_Port_ShouldStop() &&
               !g_bootstrap_arg4_call_count,
               "registered movement callback stopped or took a boundary");
        ASSERT(PE_LoadU32(HOOKREG_SLOT) == slot &&
               PE_LoadU32(slot + 12u) == HOOKREG_WITNESS,
               "callback did not receive a0 = slot, or the word changed");

        /* No C (including zero): a named stop. */
        for (unsigned n = 0; n < 2u; n++) {
            ResetTestState();
            PE_StoreU32(slot + 12u, none[n]);
            ASSERT(WKREG_M28Update(v) == 0 &&
                   PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
                   "unknown movement callback did not stop");
            ASSERT(g_bootstrap_arg4_call_count == 1 &&
                   g_bootstrap_arg4_calls[0].target == none[n] &&
                   g_bootstrap_arg4_calls[0].arg0 == slot &&
                   strcmp(g_bootstrap_arg4_calls[0].caller, caller[v]) == 0 &&
                   CountOrderLog("M28_movement_callback") == 1,
                   "boundary does not name the update, the target and a0 = slot");
        }

        /* The idle identity, a bare `jr ra` in retail, stays a no-op. */
        ResetTestState();
        PE_StoreU32(slot + 12u, idle[v]);
        ASSERT(WKREG_M28Update(v) == 0 && !PE_Port_ShouldStop() &&
               !g_bootstrap_arg4_call_count && !g_stub_order_count,
               "the idle identity made a call");
    }
    PASS();
}
