/* BIOS interrupt-chain services SysEnqIntRP C(02h) / SysDeqIntRP C(03h)
 * (platform/pe_plat_bios_intrp.c), contract from psx-spx kernelbios
 * "Priority Chains": four chains, insertion at the begin of the chain
 * (record.next = old head), removal unlinks the record, no return value
 * (the host model returns 0).  Also the stop-policy test: under the
 * native-only STOP policy the two retail entries no longer reach the decomp
 * boundary primitive (they did before they were bound), and the libpad
 * start wrapper 0x80082534 issues them without stopping the run. */
#define BI_R0 0x800A6000u
#define BI_R1 0x800A6010u
#define BI_R2 0x800A6020u

static void BI_Rec(uint32_t a, uint32_t f2, uint32_t f1)
{
    PE_StoreU32(a, 0xDEADBEEFu);   /* next: filled in by the service */
    PE_StoreU32(a + 4u, f2);
    PE_StoreU32(a + 8u, f1);
    PE_StoreU32(a + 12u, 0u);
}

static void test_bios_intrp_chain(void)
{
    TEST("BIOS_intrp_chain");
    ResetTestState();
    pe_plat_bios_intrp_reset();
    ASSERT(pe_plat_bios_intrp_head(0) == 0u && pe_plat_bios_intrp_head(3) == 0u, "chains start empty");

    /* enqueue two records at priority 2: insertion at the begin */
    BI_Rec(BI_R0, 0x11u, 0x12u);
    BI_Rec(BI_R1, 0x21u, 0x22u);
    ASSERT(pe_plat_bios_sys_enq_int_rp(2u, BI_R0) == 0, "enqueue first return");
    ASSERT(pe_plat_bios_intrp_head(2u) == BI_R0 && PE_LoadU32(BI_R0) == 0u, "first record is the head, next 0");
    ASSERT(pe_plat_bios_sys_enq_int_rp(2u, BI_R1) == 0, "enqueue second return");
    ASSERT(pe_plat_bios_intrp_head(2u) == BI_R1 && PE_LoadU32(BI_R1) == BI_R0, "second record is the new head, next = first");
    ASSERT(PE_LoadU32(BI_R1 + 4u) == 0x21u && PE_LoadU32(BI_R1 + 8u) == 0x22u &&
           PE_LoadU32(BI_R0 + 4u) == 0x11u && PE_LoadU32(BI_R0 + 8u) == 0x12u,
           "function words untouched");
    ASSERT(pe_plat_bios_intrp_head(0) == 0u && pe_plat_bios_intrp_head(1) == 0u && pe_plat_bios_intrp_head(3) == 0u,
           "other chains untouched");

    /* dequeue the first-enqueued (tail) record: unlinked from the middle of the walk */
    ASSERT(pe_plat_bios_sys_deq_int_rp(2u, BI_R0) == 0, "dequeue return");
    ASSERT(pe_plat_bios_intrp_head(2u) == BI_R1 && PE_LoadU32(BI_R1) == 0u, "chain is now just the second record");
    /* dequeue the head */
    BI_Rec(BI_R2, 0x31u, 0x32u);
    ASSERT(pe_plat_bios_sys_enq_int_rp(2u, BI_R2) == 0 && PE_LoadU32(BI_R2) == BI_R1, "third record on top");
    ASSERT(pe_plat_bios_sys_deq_int_rp(2u, BI_R2) == 0, "dequeue head return");
    ASSERT(pe_plat_bios_intrp_head(2u) == BI_R1, "head dequeue exposes the previous record");
    /* dequeue from the middle of a three-record chain */
    ASSERT(pe_plat_bios_sys_enq_int_rp(2u, BI_R0) == 0 && pe_plat_bios_sys_enq_int_rp(2u, BI_R2) == 0, "rebuild");
    ASSERT(pe_plat_bios_sys_deq_int_rp(2u, BI_R0) == 0 &&
           pe_plat_bios_intrp_head(2u) == BI_R2 && PE_LoadU32(BI_R2) == BI_R1, "middle record unlinked");

    /* a record that is not on the chain, a wrong priority and bad arguments change nothing */
    ASSERT(pe_plat_bios_sys_deq_int_rp(1u, BI_R2) == 0 && pe_plat_bios_intrp_head(2u) == BI_R2, "wrong chain");
    ASSERT(pe_plat_bios_sys_deq_int_rp(2u, BI_R0) == 0 && pe_plat_bios_intrp_head(2u) == BI_R2, "record not on chain");
    ASSERT(pe_plat_bios_sys_enq_int_rp(4u, BI_R0) == 0 && pe_plat_bios_intrp_head(4u) == 0u, "priority 4 ignored");
    ASSERT(pe_plat_bios_sys_enq_int_rp(3u, 0u) == 0 && pe_plat_bios_intrp_head(3u) == 0u, "null record ignored");
    ASSERT(pe_plat_bios_sys_enq_int_rp(3u, 0x80200000u) == 0 && pe_plat_bios_intrp_head(3u) == 0u, "record outside RAM ignored");
    ASSERT(pe_plat_bios_sys_deq_int_rp(9u, BI_R2) == 0 && pe_plat_bios_intrp_head(2u) == BI_R2, "priority 9 ignored");

    /* empty-chain dequeue, then drain */
    ASSERT(pe_plat_bios_sys_deq_int_rp(2u, BI_R2) == 0 && pe_plat_bios_sys_deq_int_rp(2u, BI_R1) == 0 &&
           pe_plat_bios_intrp_head(2u) == 0u, "drained");
    ASSERT(pe_plat_bios_sys_deq_int_rp(2u, BI_R1) == 0 && pe_plat_bios_intrp_head(2u) == 0u, "dequeue on empty chain");

    /* a cyclic chain in guest RAM cannot hang the walk */
    ASSERT(pe_plat_bios_sys_enq_int_rp(0u, BI_R0) == 0 && pe_plat_bios_sys_enq_int_rp(0u, BI_R1) == 0, "cycle build");
    PE_StoreU32(BI_R0, BI_R1);               /* R1 -> R0 -> R1 -> ... */
    ASSERT(pe_plat_bios_sys_deq_int_rp(0u, BI_R2) == 0, "cyclic chain walk terminates");

    /* a RAM reset clears the host-held chain heads */
    PE_RamReset();
    ASSERT(pe_plat_bios_intrp_head(0u) == 0u, "RAM reset empties the chains");
    PASS();
}

/* Stop policy: with the native-only STOP policy on, the retail entries are
 * bound host services -- no boundary note, no stop request -- and the libpad
 * start wrapper 0x80082534 and PadStartCom 0x80082CF0 issue them in
 * production order (dequeue then enqueue) with the retail record. */
static void test_bios_intrp_no_boundary(void)
{
    int policy = PE_Decomp_BoundaryStopEnabled();
    TEST("BIOS_intrp_no_boundary");
    ResetTestState();
    pe_plat_bios_intrp_reset();
    PE_Decomp_ResetBoundaries();
    PE_Decomp_SetBoundaryStop(1);
    (void)func_8007E1E4(2u, 0x800A5AB0u);      /* SysEnqIntRP */
    (void)func_8007E1F4(2u, 0x800A5AB0u);      /* SysDeqIntRP */
    ASSERT(PE_Decomp_BoundaryCount() == 0u && !PE_Port_ShouldStop(), "entries reached the boundary primitive");
    ASSERT(pe_plat_bios_intrp_head(2u) == 0u, "enqueue then dequeue empties the chain");
    /* wrapper 0x80082534 with an empty card slot ends right after the RCnt/I_MASK work */
    PE_StoreU32(0x800A5AB0u, 0xFFFFFFFFu);
    func_80082534();
    ASSERT(PE_Decomp_BoundaryCount() == 0u && !PE_Port_ShouldStop(), "0x80082534 reached a boundary");
    ASSERT(pe_plat_bios_intrp_head(2u) == 0x800A5AB0u && PE_LoadU32(0x800A5AB0u) == 0u,
           "0x80082534 left the record as the head of chain 2");
    /* a second start dequeues the record first, so the chain never holds it twice */
    func_80082534();
    ASSERT(pe_plat_bios_intrp_head(2u) == 0x800A5AB0u && PE_LoadU32(0x800A5AB0u) == 0u,
           "second start did not link the record to itself");
    ASSERT(PE_Decomp_BoundaryCount() == 0u && !PE_Port_ShouldStop(), "second start reached a boundary");
    PE_Decomp_SetBoundaryStop(policy);
    PASS();
}
