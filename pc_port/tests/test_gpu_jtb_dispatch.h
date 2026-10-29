/*
 * GPU jump-table dispatch through the guest-code registry (2026-10-26).
 *
 * Retail's libgpu wrappers call the D_80095744 slots with jalr.  The hand
 * wrappers keep their exact identity paths; any other slot value is called
 * through the guest-code registry with retail's a0..a3, as the immediate
 * wrappers in pe_libgpu.c and absent_sdk_port.c already do.  The tests plant
 * a registered native target, check the arguments it received and that
 * retail's code after the call ran, then plant an unregistered target and
 * check the STOP under the port's production boundary policy (PutDispEnv's
 * unregistered GP1 writer is covered by test_DAY1_dispenv).  LoadImage and
 * StoreImage keep an indirect boundary (their RECT is a host pointer with no
 * guest address) and must stop rather than answer 0.
 *
 * Witness targets: the BIOS printf vector 0x80071A74 (native; logs its
 * format and first four words) and the generated func_80074CC8 (DrawSync
 * callback setter: at debug level 0 it stores a0 in D_80095758).  No test
 * reads disc data: B54KR_SeedGpuStatic's display table only shapes
 * PutDispEnv's horizontal-range word, which no assertion checks.
 */

#define GJT_FMT        0x801F3000u   /* planted format string */
#define GJT_PACKET     0x801F3100u   /* planted DrawPrim packet */
#define GJT_UNREG      0x80012222u   /* no registry row covers it */
#define GJT_BAD_HOOK   0x80DEAD08u   /* unregistered print hook */
#define GJT_PRINTF     0x80071A74u
#define GJT_CB_SETTER  0x80074CC8u
#define GJT_CB_SLOT    0x80095758u

extern const char *PE_Decomp_BoundaryName(unsigned index);

static void GJT_PlantFormat(pe_addr_t at, const char *text)
{
    pe_addr_t i;

    for (i = 0u; text[i] != '\0'; i++)
        PE_StoreU8(at + i, (uint8_t)text[i]);
    PE_StoreU8(at + i, 0u);
}

/* Clear run-control and boundary evidence before the unregistered case. */
static void GJT_ResetEvidence(void)
{
    PE_Port_RunControlReset();
    Bootstrap_ResetArg4CallLog();
    PE_Decomp_ResetBoundaries();
    PE_Bios_PrintfLogReset();
}

/* Leave no boundary names, print log or GPU counters for later tests. */
static void GJT_Finish(void)
{
    PE_Decomp_ResetBoundaries();
    PE_Bios_PrintfLogReset();
    PE_GPU_Init();
}

/* The unregistered case's registry boundary: one name, one record carrying
 * the planted target and retail's first three argument words. */
static int GJT_RegistryStop(const char *site, pe_addr_t target, uint32_t a0,
                            uint32_t a1, uint32_t a2)
{
    const BootstrapArgCall4 *call;

    if (PE_Port_GetStopReason() != PE_PORT_STOP_UNRESOLVED_BOUNDARY ||
        PE_Decomp_BoundaryCount() != 1u ||
        strcmp(PE_Decomp_BoundaryName(0u), site) != 0 ||
        g_bootstrap_arg4_call_count != 1)
        return 0;
    call = &g_bootstrap_arg4_calls[0];
    return call->target == target && call->arg0 == a0 &&
           call->arg1 == a1 && call->arg2 == a2;
}

static void test_GPUJTB_moveimage_registry_dispatch(void)
{
    RECT rect = { 4, 8, 16, 16 };
    pe_addr_t fmt;
    uint32_t args[4];
    int policy, result;
    TEST("GPUJTB_moveimage_registry_dispatch");
    ResetTestState();
    PE_GPU_Init();
    B54KR_SeedGpuStatic();
    PE_StoreU8(0x8009574Eu, 0u);                 /* validator silent */
    GJT_PlantFormat(GJT_FMT, "MoveImage jalr %08x %x %x\n");
    PE_StoreU32(0x8009570Cu, GJT_PRINTF);         /* jtb[2] */
    PE_StoreU32(0x8009571Cu, GJT_FMT);            /* jtb[6] = a0 */
    PE_Bios_PrintfLogReset();
    result = func_8007512C(&rect, 32, 48);
    ASSERT(!PE_Port_ShouldStop() && g_bootstrap_arg4_call_count == 0,
           "registered jtb[2] stopped or used a stub");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == GJT_FMT &&
           args[0] == 0x800957E4u && args[1] == 0x14u && args[2] == 0u,
           "jtb[2] did not receive (jtb[6], packet, 0x14, 0)");
    ASSERT(result == (int)strlen("MoveImage jalr 800957e4 14 0\n"),
           "jtb[2]'s result was not returned");
    ASSERT(PE_LoadU32(0x800957ECu) == ((8u << 16) | 4u) &&
           PE_LoadU32(0x800957F0u) == ((48u << 16) | 32u) &&
           PE_LoadU32(0x800957F4u) == ((16u << 16) | 16u),
           "packet words after the call differ");

    GJT_ResetEvidence();
    PE_StoreU32(0x8009570Cu, GJT_UNREG);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    result = func_8007512C(&rect, 32, 48);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(result == 0 && GJT_RegistryStop("func_8007512C", GJT_UNREG,
                                           GJT_FMT, 0x800957E4u, 0x14u),
           "unregistered jtb[2] was not the registry's STOP");
    GJT_Finish();
    PASS();
}

static void test_GPUJTB_drawotag_registry_dispatch(void)
{
    pe_addr_t fmt;
    uint32_t args[4];
    int policy, result;
    TEST("GPUJTB_drawotag_registry_dispatch");
    ResetTestState();
    PE_GPU_Init();
    DRW1_SeedDispatch();
    GJT_PlantFormat(GJT_FMT, "DrawOTag jalr %08x %x %x\n");
    PE_StoreU32(0x8009570Cu, GJT_PRINTF);         /* jtb[2] */
    PE_StoreU32(0x8009571Cu, GJT_FMT);            /* jtb[6] = a0 */
    PE_Bios_PrintfLogReset();
    result = func_800753B4(DRW1_OT_BASE);
    ASSERT(!PE_Port_ShouldStop() && g_bootstrap_arg4_call_count == 0,
           "registered jtb[2] stopped or used a stub");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == GJT_FMT &&
           args[0] == DRW1_OT_BASE && args[1] == 0u && args[2] == 0u,
           "jtb[2] did not receive (jtb[6], ot, 0, 0)");
    ASSERT(result == (int)strlen("DrawOTag jalr 801f1000 0 0\n"),
           "jtb[2]'s result was not returned");

    GJT_ResetEvidence();
    PE_StoreU32(0x8009570Cu, GJT_UNREG);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    result = func_800753B4(DRW1_OT_BASE);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(result == 0 && GJT_RegistryStop("func_800753B4", GJT_UNREG,
                                           GJT_FMT, DRW1_OT_BASE, 0u),
           "unregistered jtb[2] was not the registry's STOP");
    GJT_Finish();
    PASS();
}

static void test_GPUJTB_clearotagr_registry_dispatch(void)
{
    const pe_addr_t ot = OTC1_OT_BASE;
    pe_addr_t fmt;
    uint32_t args[4];
    int policy;
    TEST("GPUJTB_clearotagr_registry_dispatch");
    ResetTestState();
    OTC1_SeedDispatch();
    GJT_PlantFormat(ot, "ClearOTagR jalr %d\n");  /* a0 is the OT */
    PE_StoreU32(0x80095730u, GJT_PRINTF);         /* jtb[11] */
    PE_Bios_PrintfLogReset();
    func_800752AC(ot, 4);
    ASSERT(!PE_Port_ShouldStop() && g_bootstrap_arg4_call_count == 0,
           "registered jtb[11] stopped or used a stub");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == ot && args[0] == 4u,
           "jtb[11] did not receive (ot, n)");
    /* The tail's OT[0] link; its stub-node word is not pinned here. */
    ASSERT(PE_LoadU32(ot) == 0x0009580Cu,
           "retail's terminator tail did not follow the call");

    ResetTestState();
    OTC1_SeedDispatch();
    GJT_ResetEvidence();
    PE_StoreU32(0x80095730u, GJT_UNREG);
    PE_StoreU32(ot, 0xA5A5A5A5u);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    func_800752AC(ot, 4);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(GJT_RegistryStop("func_800752AC", GJT_UNREG, ot, 4u, 0u),
           "unregistered jtb[11] was not the registry's STOP");
    ASSERT(PE_LoadU32(ot) == 0xA5A5A5A5u, "the stopped call wrote the tail");
    GJT_Finish();
    PASS();
}

static void test_GPUJTB_drawprim_registry_dispatch(void)
{
    uint32_t command = 0xE1000018u;
    PeGpuState state;
    pe_addr_t fmt;
    uint32_t args[4];
    int policy, result;
    TEST("GPUJTB_drawprim_registry_dispatch");
    ResetTestState();
    PE_GPU_Init();
    HostFB_Init();
    B54KR_SeedGpuStatic();
    /* Slot 15 (DrawSync) -> func_80074CC8(0); slot 5 -> printf with the
     * packet's words as its format: retail passes (packet + 4, length). */
    PE_StoreU32(0x80095740u, GJT_CB_SETTER);
    PE_StoreU32(GJT_CB_SLOT, 0x1234u);
    PE_StoreU32(0x80095718u, GJT_PRINTF);
    PE_StoreU32(GJT_PACKET, 0x02FFFFFFu);        /* length byte 2 */
    GJT_PlantFormat(GJT_PACKET + 4u, "Prim %x\n");
    PE_Bios_PrintfLogReset();
    result = func_80075358(GJT_PACKET);
    ASSERT(result == 0 && !PE_Port_ShouldStop() &&
           g_bootstrap_arg4_call_count == 0,
           "registered DrawPrim slots stopped or used a stub");
    ASSERT(PE_LoadU32(GJT_CB_SLOT) == 0u, "slot 15 was not called with 0");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == GJT_PACKET + 4u &&
           args[0] == 2u, "slot 5 did not receive (packet + 4, length)");

    /* An unregistered slot 15 stops before slot 5 runs. */
    GJT_ResetEvidence();
    PE_StoreU32(0x80095740u, GJT_UNREG);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    result = func_80075358(GJT_PACKET);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(result == -1 &&
           GJT_RegistryStop("func_80075358", GJT_UNREG, 0u, 0u, 0u),
           "unregistered slot 15 was not the registry's STOP");
    ASSERT(PE_Bios_PrintfLog(NULL, NULL) == 0u,
           "slot 5 ran after the stopped slot-15 call");

    /* An unregistered slot 5 for a guest packet: slot 15 ran first. */
    GJT_ResetEvidence();
    PE_StoreU32(0x80095740u, GJT_CB_SETTER);
    PE_StoreU32(GJT_CB_SLOT, 0x1234u);
    PE_StoreU32(0x80095718u, GJT_UNREG);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    result = func_80075358(GJT_PACKET);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(result == -1 && PE_LoadU32(GJT_CB_SLOT) == 0u &&
           GJT_RegistryStop("func_80075358", GJT_UNREG,
                            GJT_PACKET + 4u, 2u, 0u),
           "unregistered slot 5 was not the registry's STOP");

    /* The host-words transient caller: slot 15 still goes through the
     * registry; its retail slot 5 writes the command word to GP0. */
    GJT_ResetEvidence();
    PE_GPU_Init();
    PE_StoreU32(0x80095718u, 0x80076B58u);
    PE_StoreU32(GJT_CB_SLOT, 0x1234u);
    ASSERT(PE_func_80075358_Transient(&command, 1u) == 0 &&
           !PE_Port_ShouldStop() && PE_LoadU32(GJT_CB_SLOT) == 0u,
           "transient caller's registered slot 15 did not run");
    PE_GPU_GetState(&state);
    ASSERT(state.draw_mode == command && state.draw_mode_count == 1u,
           "transient caller's slot 5 did not write the command word");

    GJT_ResetEvidence();
    PE_StoreU32(0x80095740u, GJT_UNREG);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    result = PE_func_80075358_Transient(&command, 1u);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(result == -1 &&
           GJT_RegistryStop("func_80190660", GJT_UNREG, 0u, 0u, 0u),
           "transient caller's unregistered slot 15 was not the registry's STOP");
    GJT_Finish();
    PASS();
}

static void test_GPUJTB_putdispenv_registry_dispatch(void)
{
    const pe_addr_t env = 0x80150000u;
    PeGpuState before, after;
    pe_addr_t fmt;
    uint32_t args[4];
    int policy;
    TEST("GPUJTB_putdispenv_registry_dispatch");
    ResetTestState();
    PE_GPU_Init();
    DAY1_SeedDisplayDispatch();
    PE_Fill(env, 20u, 0u);
    PE_StoreU16(env + 4u, 320u);
    PE_StoreU16(env + 6u, 240u);
    PE_Fill(0x800A3348u, 256u, 0u);
    PE_StoreU32(0x80095714u, GJT_CB_SETTER);     /* jtb[4] */
    PE_StoreU32(GJT_CB_SLOT, 0x1234u);
    PE_GPU_GetState(&before);
    PE_Bios_PrintfLogReset();
    ASSERT(func_800755F0(env) == env && !PE_Port_ShouldStop() &&
           g_bootstrap_arg4_call_count == 0,
           "registered jtb[4] stopped or used a stub");
    PE_GPU_GetState(&after);
    ASSERT(PE_LoadU32(GJT_CB_SLOT) == 0x07040010u,
           "jtb[4] did not receive the last GP1 word (vertical range)");
    ASSERT(after.display_command_count == before.display_command_count &&
           PE_LoadU8(0x800A3348u + 7u) == 0u &&
           PE_LoadU8(0x800A3348u + 8u) == 0u,
           "the identity GP1 writer ran instead of jtb[4]");
    ASSERT(PE_LoadU16(0x800957BCu) == 320u,
           "retail's environment cache did not follow the calls");
    ASSERT(PE_Bios_PrintfLog(NULL, NULL) == 0u,
           "PutDispEnv printed at debug level 0");

    /* Level-2 debug print: retail's jalr D_80095748(fmt, env). */
    ResetTestState();
    PE_GPU_Init();
    DAY1_SeedDisplayDispatch();
    PE_Fill(env, 20u, 0u);
    PE_StoreU8(0x8009574Eu, 2u);
    PE_Bios_PrintfLogReset();
    ASSERT(func_800755F0(env) == env && !PE_Port_ShouldStop() &&
           g_bootstrap_arg4_call_count == 0,
           "level-2 PutDispEnv stopped or used a stub");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == 0x80011970u &&
           args[0] == env, "level-2 print did not use the guest hook");

    /* An unregistered print hook stops before any GP1 word or cache copy. */
    ResetTestState();
    PE_GPU_Init();
    DAY1_SeedDisplayDispatch();
    GJT_ResetEvidence();
    PE_Fill(env, 20u, 0u);
    PE_Fill(0x800957B8u, 20u, 0xA5u);
    PE_StoreU8(0x8009574Eu, 2u);
    PE_StoreU32(0x80095748u, GJT_BAD_HOOK);
    PE_GPU_GetState(&before);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    (void)func_800755F0(env);
    PE_Decomp_SetBoundaryStop(policy);
    PE_GPU_GetState(&after);
    ASSERT(GJT_RegistryStop("func_800755F0", GJT_BAD_HOOK, 0x80011970u,
                            env, 0u) &&
           after.display_command_count == before.display_command_count &&
           PE_LoadU32(0x800957B8u) == 0xA5A5A5A5u,
           "an unregistered print hook was not the registry's STOP");
    GJT_Finish();
    PASS();
}

/* PutDrawEnv and DrawOTagEnv share the jtb[2](jtb[6], dr, 0x40, 0) call. */
static void test_GPUJTB_drawenv_registry_dispatch(void)
{
    const pe_addr_t env = 0x80110000u;
    const pe_addr_t dr = env + 0x1Cu;
    pe_addr_t fmt;
    uint32_t args[4];
    int policy;
    TEST("GPUJTB_drawenv_registry_dispatch");
    ResetTestState();
    PE_GPU_Init();
    B54KR_SeedGpuStatic();
    memset(PE_Translate(env, 0x5Cu), 0, 0x5Cu);
    (void)func_80074924(env, 0, 240, 320, 240);
    GJT_PlantFormat(GJT_FMT, "DrawEnv jalr %08x %x %x\n");
    PE_StoreU32(0x8009570Cu, GJT_PRINTF);         /* jtb[2] */
    PE_StoreU32(0x8009571Cu, GJT_FMT);            /* jtb[6] = a0 */
    PE_Bios_PrintfLogReset();
    (void)func_80075424(env);
    ASSERT(!PE_Port_ShouldStop() && g_bootstrap_arg4_call_count == 0,
           "PutDrawEnv's registered jtb[2] stopped or used a stub");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == GJT_FMT &&
           args[0] == dr && args[1] == 0x40u && args[2] == 0u,
           "PutDrawEnv's jtb[2] did not receive (jtb[6], dr, 0x40, 0)");
    ASSERT(memcmp(PE_TranslateConst(0x8009575Cu, 0x5Cu),
                  PE_TranslateConst(env, 0x5Cu), 0x5Cu) == 0,
           "PutDrawEnv's cache copy did not follow the call");

    PE_Fill(0x8009575Cu, 0x5Cu, 0xA5u);
    PE_Bios_PrintfLogReset();
    func_800754E4(DRW1_OT_BASE, env);
    ASSERT(!PE_Port_ShouldStop() && g_bootstrap_arg4_call_count == 0,
           "DrawOTagEnv's registered jtb[2] stopped or used a stub");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == GJT_FMT &&
           args[0] == dr && args[1] == 0x40u && args[2] == 0u,
           "DrawOTagEnv's jtb[2] did not receive (jtb[6], dr, 0x40, 0)");
    ASSERT((PE_LoadU32(dr) & 0x00FFFFFFu) == (DRW1_OT_BASE & 0x00FFFFFFu) &&
           memcmp(PE_TranslateConst(0x8009575Cu, 0x5Cu),
                  PE_TranslateConst(env, 0x5Cu), 0x5Cu) == 0,
           "DrawOTagEnv's splice or cache copy differs");

    GJT_ResetEvidence();
    PE_StoreU32(0x8009570Cu, GJT_UNREG);
    PE_StoreU32(0x8009575Cu, 0xA5A5A5A5u);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    (void)func_80075424(env);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(GJT_RegistryStop("func_80075424", GJT_UNREG, GJT_FMT, dr, 0x40u) &&
           PE_LoadU32(0x8009575Cu) == 0xA5A5A5A5u,
           "PutDrawEnv's unregistered jtb[2] was not the registry's STOP");

    GJT_ResetEvidence();
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    func_800754E4(DRW1_OT_BASE, env);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(GJT_RegistryStop("func_800754E4", GJT_UNREG, GJT_FMT, dr, 0x40u) &&
           PE_LoadU32(0x8009575Cu) == 0xA5A5A5A5u,
           "DrawOTagEnv's unregistered jtb[2] was not the registry's STOP");

    /* PutDrawEnv's level-2 debug print: retail's jalr D_80095748(fmt, env). */
    ResetTestState();
    PE_GPU_Init();
    B54KR_SeedGpuStatic();
    memset(PE_Translate(env, 0x5Cu), 0, 0x5Cu);
    (void)func_80074924(env, 0, 240, 320, 240);
    PE_StoreU8(0x8009574Eu, 2u);
    PE_Bios_PrintfLogReset();
    ASSERT(func_80075424(env) == env && !PE_Port_ShouldStop(),
           "level-2 PutDrawEnv stopped");
    ASSERT(PE_Bios_PrintfLog(&fmt, args) == 1u && fmt == 0x8001193Cu &&
           args[0] == env, "level-2 PutDrawEnv print did not use the guest hook");

    /* An unregistered print hook stops before the packet or cache copy. */
    ResetTestState();
    PE_GPU_Init();
    B54KR_SeedGpuStatic();
    GJT_ResetEvidence();
    memset(PE_Translate(env, 0x5Cu), 0, 0x5Cu);
    (void)func_80074924(env, 0, 240, 320, 240);
    PE_StoreU8(0x8009574Eu, 2u);
    PE_StoreU32(0x80095748u, GJT_BAD_HOOK);
    PE_StoreU32(0x8009575Cu, 0xA5A5A5A5u);
    PE_StoreU32(dr, 0xA5A5A5A5u);
    policy = PE_Decomp_BoundaryStopEnabled();
    PE_Decomp_SetBoundaryStop(1);
    (void)func_80075424(env);
    PE_Decomp_SetBoundaryStop(policy);
    ASSERT(GJT_RegistryStop("func_80075424", GJT_BAD_HOOK, 0x8001193Cu,
                            env, 0u) &&
           PE_LoadU32(dr) == 0xA5A5A5A5u &&
           PE_LoadU32(0x8009575Cu) == 0xA5A5A5A5u,
           "PutDrawEnv's unregistered print hook was not the registry's STOP");
    GJT_Finish();
    PASS();
}

/* The retail dispatcher identity with another jtb[6]: the hand dispatcher
 * owns worker identities.  It runs its prefix (deadline, polls, work
 * marker), then its unknown-worker branch is the boundary that stops, and
 * the wrapper copies no environment after it.  Direct-issue state (empty
 * ring, idle DMA2, no DrawSync callback); with a busy ring or DMA the worker
 * is queued and the pump judges it later, as for DrawOTag and MoveImage. */
static void test_GPUJTB_drawenv_unknown_worker_in_dispatcher(void)
{
    const pe_addr_t env = 0x80110000u;
    const pe_addr_t dr = env + 0x1Cu;
    const BootstrapArgCall4 *call;
    int pass;
    TEST("GPUJTB_drawenv_unknown_worker_in_dispatcher");
    for (pass = 0; pass < 2; pass++) {
        ResetTestState();
        PE_GPU_Init();
        B54KR_SeedGpuStatic();
        memset(PE_Translate(env, 0x5Cu), 0, 0x5Cu);
        (void)func_80074924(env, 0, 240, 320, 240);
        PE_StoreU32(0x8009571Cu, GJT_UNREG);     /* jtb[6]; jtb[2] retail */
        PE_StoreU32(0x8009575Cu, 0xA5A5A5A5u);
        if (pass == 0)
            (void)func_80075424(env);
        else
            func_800754E4(DRW1_OT_BASE, env);
        ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
               PE_LoadU32(0x8009575Cu) == 0xA5A5A5A5u &&
               g_bootstrap_arg4_call_count == 1,
               "an unknown worker did not stop before the environment cache copy");
        call = &g_bootstrap_arg4_calls[0];
        ASSERT(strcmp(call->caller, "func_80076C34") == 0 &&
               call->target == GJT_UNREG && call->arg0 == dr,
               "the wrapper, not the dispatcher, judged the worker identity");
        ASSERT(PE_LoadU32(0x80095888u) == 0xF0u &&
               PE_LoadU32(0x8009588Cu) == 0u &&
               PE_LoadU32(0x80095754u) == 1u,
               "the dispatcher prefix did not run before the worker check");
    }
    GJT_Finish();
    PASS();
}

static void test_GPUJTB_load_store_image_dirty_jtb_stop(void)
{
    RECT rc = { 0, 0, 4, 4 };
    const BootstrapArgCall4 *call;
    TEST("GPUJTB_load_store_image_dirty_jtb_stop");
    ResetTestState();
    B52_SeedGpuDispatch();
    PE_StoreU32(B52_JTB + 8u, GJT_UNREG);
    PE_StoreU32(B52_JTB + 0x1Cu, 0x800768A0u);   /* StoreImage worker */
    ASSERT(func_8007506C(&rc, 0x80100000u) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           g_bootstrap_arg4_call_count == 1,
           "LoadImage's dirty jtb[2] answered 0 without a stop");
    call = &g_bootstrap_arg4_calls[0];
    ASSERT(call->target == GJT_UNREG && call->arg0 == B52_WORKER &&
           call->arg2 == 8u && call->arg3 == 0x80100000u,
           "LoadImage boundary record differs");

    PE_Port_RunControlReset();
    Bootstrap_ResetArg4CallLog();
    ASSERT(func_800750CC(&rc, 0x80100000u) == 0 &&
           PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY &&
           g_bootstrap_arg4_call_count == 1,
           "StoreImage's dirty jtb[2] answered 0 without a stop");
    call = &g_bootstrap_arg4_calls[0];
    ASSERT(call->target == GJT_UNREG && call->arg0 == 0x800768A0u &&
           call->arg2 == 8u && call->arg3 == 0x80100000u,
           "StoreImage boundary record differs");
    GJT_Finish();
    PASS();
}

static void test_GPUJTB_all(void)
{
    test_GPUJTB_moveimage_registry_dispatch();
    test_GPUJTB_drawotag_registry_dispatch();
    test_GPUJTB_clearotagr_registry_dispatch();
    test_GPUJTB_drawprim_registry_dispatch();
    test_GPUJTB_putdispenv_registry_dispatch();
    test_GPUJTB_drawenv_registry_dispatch();
    test_GPUJTB_drawenv_unknown_worker_in_dispatcher();
    test_GPUJTB_load_store_image_dirty_jtb_stop();
}
