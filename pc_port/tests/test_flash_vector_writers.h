/* The m0023i flash's borrowed six bytes (F710 sp+48, S-112..S-107 below the
 * 69594 dispatch sp S): writers bound from retail frames.  Each case drives
 * the real weapon update/draw ports (CD8F0 -> C251C/C2758, CD8C8 -> C2414)
 * on a zeroed slot and checks the modeled bytes the F710 arm later reads.
 *   - CDE90 and CDD0C at the weapon-draw depth write none of the bytes;
 *   - C2758's C2B90 call leaves the return address low half 0x2858 in Z
 *     and C2758's s4 (unknown here) in X/Y;
 *   - the frameless update leaves of the three weapon tables (CA4A8,
 *     CA4B4, CA540, CDF40, CDF4C, CE464, CE470) keep them unless a record
 *     store could reach crt0's stack reservation;
 *   - CDFE0, whose frame at the C251C dispatch depth stays above them,
 *     keeps them too;
 *   - other callbacks and direct C2B90 calls keep forgetting. */
#define FVW_SLOT 0x801F0000u
#define FVW_PROG (FVW_SLOT+0xE00u)
/* Each case runs on a snapshot: the native callees also write packets,
 * the packet cursor and scratch, and later tests must not see them. */
static uint8_t *fvw_ram,fvw_scratch[0x400];

static int fvw_save(int restore)
{
    if (!fvw_ram && !(fvw_ram=malloc(PE_RAM_SIZE))) return 0;
    if (restore) {
        memcpy(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),fvw_ram,PE_RAM_SIZE);
        memcpy(PE_Translate(0x1F800000u,sizeof fvw_scratch),fvw_scratch,sizeof fvw_scratch);
    } else {
        memcpy(fvw_ram,PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),PE_RAM_SIZE);
        memcpy(fvw_scratch,PE_TranslateConst(0x1F800000u,sizeof fvw_scratch),sizeof fvw_scratch);
    }
    return 1;
}

/* Zeroed slot (owner 0, so C6CE0 returns 0 and C2B90 creates nothing),
 * one bytecode op then stop, record 0 with code 0 bound to the given
 * draw/update callbacks (0 = inactive). */
static void fvw_slot(uint32_t op,pe_addr_t draw_fn,pe_addr_t update_fn)
{
    for (pe_addr_t a=FVW_SLOT;a<FVW_SLOT+0x1000u;a+=4u) PE_StoreU32(a,0u);
    PE_StoreU32(FVW_SLOT+120u,FVW_PROG);
    PE_StoreU32(FVW_PROG,op);PE_StoreU32(FVW_PROG+4u,0xFFFFFFFFu);
    PE_StoreU32(0x800E0F28u,draw_fn);PE_StoreU32(0x800E0F38u,update_fn);
    if (draw_fn || update_fn) PE_StoreU8(FVW_SLOT+129u,1u);
}

static const char *fvw_cause(unsigned known,pe_addr_t reset_fn,const char *text)
{
    static char buf[256];
    PeEffectStackCause cause;char describe[160];
    PE_EffectStackCause(&cause);PE_EffectStackDescribe(&cause,describe,sizeof describe);
    if (PE_Port_ShouldStop()) return "weapon ports stopped at a boundary";
    if (cause.known!=known || cause.reset_fn!=reset_fn || !strstr(describe,text)) {
        snprintf(buf,sizeof buf,"known %u reset %08X \"%s\"",cause.known,(unsigned)cause.reset_fn,describe);
        return buf;
    }
    return NULL;
}

/* Update pass: C2758 runs op 0x00010000 (create code 0) through C2B90. */
static const char *fvw_spawn_update(void)
{
    int16_t position[3];
    const char *error;
    fvw_slot(0x00010000u,0,0);
    PE_EffectStackBegin();
    (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_SLOT);
    if ((error=fvw_cause(4u,0x800C2B90u,"unknown XY, last cleared by callback 800C2B90 in context weapon-update")))
        return error;
    PE_EffectStackPosition(position);
    return position[2]==0x2858?NULL:"Z is not the C2B90 return-address low half";
}

static const char *fvw_case_spawn_binds_z(void)
{
    const char *error=fvw_spawn_update();
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_cde90_preserves_known_z(void)
{
    int16_t position[3];
    const char *error=fvw_spawn_update();
    if (!error) {
        PE_StoreU32(0x800E0F28u,0x800CDE90u);PE_StoreU8(FVW_SLOT+129u,1u);
        (void)PE_EffectStackWeaponDraw(0x800CD8C8u,FVW_SLOT);
        error=fvw_cause(4u,0x800C2B90u,"last cleared by callback 800C2B90 in context weapon-update");
        PE_EffectStackPosition(position);
        if (!error && position[2]!=0x2858) error="CDE90 changed the retained Z";
    }
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_cde90_keeps_unknown_z(void)
{
    const char *error;
    fvw_slot(0xFFFFFFFFu,0x800CDE90u,0);
    PE_EffectStackBegin();PE_EffectStackInvalidate();
    (void)PE_EffectStackWeaponDraw(0x800CD8C8u,FVW_SLOT);
    error=fvw_cause(0u,PE_EFFECT_STACK_RESET_INVALIDATE,"unknown XYZ, last cleared by invalidate");
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_other_callback_forgets(void)
{
    const char *error=fvw_spawn_update();
    if (!error) {
        PE_StoreU32(0x800E0F28u,0x800CDC24u);PE_StoreU8(FVW_SLOT+129u,1u);
        (void)PE_EffectStackWeaponDraw(0x800CD8C8u,FVW_SLOT);
        error=fvw_cause(0u,0x800CDC24u,"unknown XYZ, last cleared by callback 800CDC24 in context weapon-draw");
    }
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_cdd0c_preserves_known_z(void)
{
    int16_t position[3];
    const char *error=fvw_spawn_update();
    if (!error) {
        PE_StoreU32(0x800E0F28u,0x800CDD0Cu);PE_StoreU8(FVW_SLOT+129u,1u);
        (void)PE_EffectStackWeaponDraw(0x800CD8C8u,FVW_SLOT);
        error=fvw_cause(4u,0x800C2B90u,"last cleared by callback 800C2B90 in context weapon-update");
        PE_EffectStackPosition(position);
        if (!error && position[2]!=0x2858) error="CDD0C changed the retained Z";
    }
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_cdd0c_keeps_unknown_z(void)
{
    const char *error;
    fvw_slot(0xFFFFFFFFu,0x800CDD0Cu,0);
    PE_EffectStackBegin();PE_EffectStackInvalidate();
    (void)PE_EffectStackWeaponDraw(0x800CD8C8u,FVW_SLOT);
    error=fvw_cause(0u,PE_EFFECT_STACK_RESET_INVALIDATE,"unknown XYZ, last cleared by invalidate");
    PE_EffectStackEnd();return error;
}

/* CA540 as record 0's update callback after the spawn pass bound Z: C251C
 * dispatches it (the bytecode cursor stays on its stop op, so C2758 does
 * not bind again). `offset` is rec[4], the record's data offset from the
 * slot's arena base slot+512. Returns the cause check, NULL on success. */
static const char *fvw_ca540_update(pe_addr_t slot,uint16_t offset,unsigned known,pe_addr_t reset_fn,
                                    const char *text)
{
    pe_addr_t data=slot+512u+(pe_addr_t)(int32_t)(int16_t)offset;
    PE_StoreU32(0x800E0F38u,0x800CA540u);
    PE_StoreU8(slot+128u,0u);PE_StoreU8(slot+129u,1u);PE_StoreU16(slot+132u,offset);
    PE_StoreU16(data+4u,100u);
    (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,slot);
    /* Retail CA540: data+4 -= 20; below 20 it zeroes it and retires rec. */
    if (PE_LoadU16(data+4u)!=80u || PE_LoadU8(slot+129u)!=1u) return "CA540 did not run its retail update";
    return fvw_cause(known,reset_fn,text);
}

static const char *fvw_case_ca540_preserves_known_z(void)
{
    int16_t position[3];
    const char *error=fvw_spawn_update();
    if (!error) error=fvw_ca540_update(FVW_SLOT,0u,4u,0x800C2B90u,
        "unknown XY, last cleared by callback 800C2B90 in context weapon-update");
    PE_EffectStackPosition(position);
    if (!error && position[2]!=0x2858) error="CA540 changed the retained Z";
    PE_EffectStackEnd();return error;
}

/* The same two passes on a slot whose arena record reaches crt0's stack
 * reservation (>= 0x801FDFF8): there the leaf's sh could land on the
 * borrowed bytes, so the bytes are forgotten. Just below it they are kept. */
#define FVW_HIGH_SLOT 0x801FC000u
static const char *fvw_ca540_high(uint16_t offset,unsigned known,pe_addr_t reset_fn,const char *text)
{
    const char *error;
    for (pe_addr_t a=FVW_HIGH_SLOT;a<FVW_HIGH_SLOT+0x200u;a+=4u) PE_StoreU32(a,0u);
    PE_StoreU32(FVW_HIGH_SLOT+120u,FVW_PROG);
    PE_StoreU32(FVW_PROG,0x00010000u);PE_StoreU32(FVW_PROG+4u,0xFFFFFFFFu);
    PE_StoreU32(0x800E0F28u,0u);PE_StoreU32(0x800E0F38u,0u);
    PE_EffectStackBegin();
    (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_HIGH_SLOT);
    if ((error=fvw_cause(4u,0x800C2B90u,"last cleared by callback 800C2B90 in context weapon-update")))
        return error;
    return fvw_ca540_update(FVW_HIGH_SLOT,offset,known,reset_fn,text);
}

static const char *fvw_case_ca540_stack_record_forgets(void)
{
    /* data = 0x801FC200 + 0x1DF4 = 0x801FDFF4; the sh lands on 0x801FDFF8. */
    const char *error=fvw_ca540_high(0x1DF4u,0u,0x800CA540u,
        "unknown XYZ, last cleared by callback 800CA540 in context weapon-update");
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_ca540_below_stack_preserves(void)
{
    /* data+4 = 0x801FDFF6..F7, the last halfword below the reservation. */
    const char *error=fvw_ca540_high(0x1DF2u,4u,0x800C2B90u,
        "last cleared by callback 800C2B90 in context weapon-update");
    PE_EffectStackEnd();return error;
}

/* CA4B4 after the spawn pass: retail's moving-particle update leaf. */
static const char *fvw_case_ca4b4_update_preserves_known_z(void)
{
    int16_t position[3];
    pe_addr_t data=FVW_SLOT+512u;
    const char *error=fvw_spawn_update();
    if (!error) {
        PE_StoreU32(0x800E0F38u,0x800CA4B4u);
        PE_StoreU8(FVW_SLOT+128u,0u);PE_StoreU8(FVW_SLOT+129u,1u);PE_StoreU16(FVW_SLOT+132u,0u);
        PE_StoreU8(data+1u,7u);PE_StoreU8(data+2u,5u);
        PE_StoreU16(data+8u,100u);PE_StoreU16(data+16u,10u);PE_StoreU16(data+18u,(uint16_t)-20);
        (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_SLOT);
        /* Retail CA4B4: x += vx, y += vy, vy += 3, age++, life--. */
        if (PE_LoadU16(data+8u)!=110u || PE_LoadU16(data+18u)!=(uint16_t)-17 ||
            PE_LoadU8(data+1u)!=8u || PE_LoadU8(data+2u)!=4u || PE_LoadU8(FVW_SLOT+129u)!=1u)
            error="CA4B4 did not run its retail update";
        else error=fvw_cause(4u,0x800C2B90u,
            "unknown XY, last cleared by callback 800C2B90 in context weapon-update");
        PE_EffectStackPosition(position);
        if (!error && position[2]!=0x2858) error="CA4B4 changed the retained Z";
    }
    PE_EffectStackEnd();return error;
}

/* CDFE0 (hit table entry 4) dispatched by C251C after the spawn pass: its
 * frame (S-104..S-72) stays above the borrowed bytes, so the bound Z and the
 * cause survive, and the retail update runs: life -= 3, data+6 -= 4,
 * data+4 += rand() % 11 - 5.  Seed 1 gives rand() = 0x41C6 = 16838, and
 * 16838 % 11 = 8.  The rand seed is restored for later tests. */
static const char *fvw_case_cdfe0_update_preserves_known_z(void)
{
    int16_t position[3];
    pe_addr_t data=FVW_SLOT+512u;
    uint32_t seed=PE_BiosRandSeed();
    const char *error=fvw_spawn_update();
    if (!error) {
        PE_StoreU32(0x800E0F38u,0x800CDFE0u);
        PE_StoreU8(FVW_SLOT+128u,0u);PE_StoreU8(FVW_SLOT+129u,1u);PE_StoreU16(FVW_SLOT+132u,0u);
        PE_StoreU8(data+1u,100u);PE_StoreU16(data+4u,100u);PE_StoreU16(data+6u,100u);
        func_80071A64(1u);
        (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_SLOT);
        if (PE_LoadU8(data+1u)!=97u || PE_LoadU16(data+6u)!=96u || PE_LoadU16(data+4u)!=103u ||
            PE_LoadU8(FVW_SLOT+129u)!=1u)
            error="CDFE0 did not run its retail update";
        else error=fvw_cause(4u,0x800C2B90u,
            "unknown XY, last cleared by callback 800C2B90 in context weapon-update");
        PE_EffectStackPosition(position);
        if (!error && position[2]!=0x2858) error="CDFE0 changed the retained Z";
    }
    func_80071A64(seed);
    PE_EffectStackEnd();return error;
}

static const char *fvw_case_other_update_callback_forgets(void)
{
    const char *error=fvw_spawn_update();
    if (!error) {
        /* A callback outside the frameless update list forgets, by address.
         * CDD04 (an empty draw leaf PE_WeaponCallback returns from) keeps
         * the pass free of side effects; the RNG-calling CDFE0 would advance
         * the host rand seed for later tests. */
        PE_StoreU32(0x800E0F38u,0x800CDD04u);
        PE_StoreU8(FVW_SLOT+128u,0u);PE_StoreU8(FVW_SLOT+129u,1u);
        (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_SLOT);
        error=fvw_cause(0u,0x800CDD04u,
            "unknown XYZ, last cleared by callback 800CDD04 in context weapon-update");
    }
    PE_EffectStackEnd();return error;
}

/* Every frameless leaf of the three weapon update tables keeps a bound Z
 * when C251C dispatches it after the spawn pass. */
static const char *fvw_case_frameless_update_table(void)
{
    static const pe_addr_t leaves[] = {0x800CA4A8u, 0x800CA4B4u, 0x800CA540u, 0x800CDF40u,
                                       0x800CDF4Cu, 0x800CE464u, 0x800CE470u};
    static char buf[320];
    for (unsigned k = 0; k < sizeof leaves / sizeof leaves[0]; k++) {
        int16_t position[3];
        pe_addr_t data=FVW_SLOT+512u;
        const char *error=fvw_spawn_update();
        if (!error) {
            PE_StoreU32(0x800E0F38u,leaves[k]);
            PE_StoreU8(FVW_SLOT+128u,0u);PE_StoreU8(FVW_SLOT+129u,1u);PE_StoreU16(FVW_SLOT+132u,0u);
            PE_StoreU8(data+1u,100u);PE_StoreU8(data+2u,5u);PE_StoreU16(data+4u,100u);
            (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_SLOT);
            error=fvw_cause(4u,0x800C2B90u,"last cleared by callback 800C2B90 in context weapon-update");
            PE_EffectStackPosition(position);
            if (!error && position[2]!=0x2858) error="Z changed";
        }
        PE_EffectStackEnd();
        if (error) {
            snprintf(buf,sizeof buf,"leaf %08X: %s",(unsigned)leaves[k],error);
            return buf;
        }
    }
    return NULL;
}

/* C2B90 reached directly (here as C251C's record callback through the
 * guest-call table, as overlay code reaches it) is not C2758's frame. */
static const char *fvw_case_direct_c2b90_does_not_bind(void)
{
    const char *error;
    if (!PE_GuestCode_Resolve(0x800C2B90u)) return "C2B90 is not reachable through the guest-call table";
    fvw_slot(0xFFFFFFFFu,0,0x800C2B90u);
    PE_EffectStackBegin();PE_EffectStackInvalidate();
    (void)PE_EffectStackWeaponUpdate(0x800CD8F0u,FVW_SLOT);
    error=fvw_cause(0u,PE_EFFECT_STACK_RESET_WEAPON_UPDATE,
        "unknown XYZ, last cleared by weapon-update call in context weapon-update");
    PE_EffectStackEnd();return error;
}

static void fvw_run(const char *name,const char *(*body)(void))
{
    const char *error;
    TEST(name);
    ASSERT(fvw_save(0),"cannot allocate the RAM snapshot");
    error=body();fvw_save(1);
    PE_EffectStackInvalidate();
    ASSERT(!error,error);
    PASS();
}

static void test_FLASHVEC_borrowed_vector_writers(void)
{
    fvw_run("FLASHVEC_c2758_spawn_binds_z_2858",fvw_case_spawn_binds_z);
    fvw_run("FLASHVEC_cde90_preserves_known_z",fvw_case_cde90_preserves_known_z);
    fvw_run("FLASHVEC_cde90_keeps_unknown_z_unknown",fvw_case_cde90_keeps_unknown_z);
    fvw_run("FLASHVEC_other_weapon_callback_forgets",fvw_case_other_callback_forgets);
    fvw_run("FLASHVEC_cdd0c_preserves_known_z",fvw_case_cdd0c_preserves_known_z);
    fvw_run("FLASHVEC_cdd0c_keeps_unknown_z_unknown",fvw_case_cdd0c_keeps_unknown_z);
    fvw_run("FLASHVEC_ca540_update_preserves_known_z",fvw_case_ca540_preserves_known_z);
    fvw_run("FLASHVEC_ca540_stack_record_forgets",fvw_case_ca540_stack_record_forgets);
    fvw_run("FLASHVEC_ca540_below_stack_preserves",fvw_case_ca540_below_stack_preserves);
    fvw_run("FLASHVEC_ca4b4_update_preserves_known_z",fvw_case_ca4b4_update_preserves_known_z);
    fvw_run("FLASHVEC_frameless_update_table_preserves",fvw_case_frameless_update_table);
    fvw_run("FLASHVEC_cdfe0_update_preserves_known_z",fvw_case_cdfe0_update_preserves_known_z);
    fvw_run("FLASHVEC_other_update_callback_forgets",fvw_case_other_update_callback_forgets);
    fvw_run("FLASHVEC_direct_c2b90_call_does_not_bind",fvw_case_direct_c2b90_does_not_bind);
}
