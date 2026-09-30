#include "retail_m34_projectile_cases.h"

static void test_DAY1_m34_retained_context(void)
{
    int32_t words[6];
    TEST("DAY1_m34_retained_context");
    ResetTestState();
    PE_StoreU32(0x8018F014u,0x0C0308BEu);PE_StoreU32(0x8018F020u,0x2463FF98u);
    PE_StoreU32(0x8018FF80u,0x8018F00Cu);PE_StoreU32(0x8018FF3Cu,0x8018F434u);
    D_8009D1A0=0x80u;
    PE_M34StackField(1);PE_M34StackActor(0x80130000u);PE_M34StackVm(1);
    PE_M34StackOpcode(0x800184ECu);
    PE_M34StackClipSound(0x80131000u,0x80130000u,31u);
    ASSERT(!PE_M34StackRead(words),"sound alone cannot supply all six words");
    PE_M34StackOpcode(0x80018774u);PE_M34StackConstruct(8u);
    PE_M34StackOpcode(0x800187C0u);
    PE_M34BossEffectCommand(0x80140000u,0u,0u,0u,0x89ABCDEFu,0x76543210u);
    ASSERT(PE_M34StackRead(words),"the three original writers establish the record");
    ASSERT(words[0]==31 && (uint32_t)words[1]==0x80131000u && (uint32_t)words[2]==0x80130000u,
           "sound volume and saved pointers come from this call");
    ASSERT((uint32_t)words[3]==0x80010690u && (uint32_t)words[4]==0x89ABCDEFu && (uint32_t)words[5]==0x76543210u,
           "original VM register and nonzero fifth/sixth arguments survive");
    PE_M34StackOpcode(0);PE_M34StackVm(0);PE_M34StackActor(0);PE_M34StackField(0);
    ASSERT(!PE_M34StackRead(words),"unrelated callers cannot consume a field stack record");
    PE_M34StackField(1);
    ASSERT(PE_M34StackRead(words),"retained words survive the field return");
    PE_M34StackCallback(0x8018F830u);
    { const int16_t vertices[4][4]={{0},{0},{0,0,-1,0},{-2,0,-3,0}};
      PE_M34StackPointQuad(vertices); }
    ASSERT(PE_M34StackRead(words) && (uint32_t)words[0]==0xFFFFu &&
           (uint32_t)words[1]==0xFFFEu && (uint32_t)words[2]==0x8013FFFDu,
           "collision quad halfword stores preserve original high halves");
    PE_M34StackCallback(0x800C9FD8u);
    ASSERT(PE_M34StackRead(words),"translated pistol draw must not wipe M34 retained stack");
    PE_M34StackOpcode(0x800184ECu);
    PE_M34StackClipSound(0x80139999u,0x8013AAAAu,7u);
    ASSERT(PE_M34StackRead(words) && (uint32_t)words[2]==0x8013FFFDu,
           "post-construct pistol clip-sound must not replace F434 owner");
    PE_M34StackOpcode(0x80012850u);
    PE_M34StackClipSound(0x80139999u,0x8013AAAAu,0xFFFFFFFFu);
    ASSERT(PE_M34StackRead(words) && (uint32_t)words[2]==0x8013FFFDu,
           "non-184EC spatial clip must not wipe F434 owner");
    PE_M34StackOpcode(0x800BEEF0u);
    ASSERT(PE_M34StackRead(words),
           "unrelated field opcode must not wipe committed F434 record");
    PE_M34StackCallback(0x800DEAD0u);
    ASSERT(PE_M34StackRead(words),
           "unknown overlay-live callback must not wipe committed F434 record");
    PE_M34StackField(0);
    PE_M34StackCallback(0x800DEAD0u);
    ASSERT(!PE_M34StackRead(words),"unknown callback outside field still invalidates");
    PE_M34StackField(1);PE_M34StackActor(0x80130000u);PE_M34StackVm(1);
    PE_M34StackOpcode(0x800184ECu);
    PE_M34StackClipSound(0x80131000u,0x80130000u,31u);
    PE_M34StackOpcode(0x80018774u);PE_M34StackConstruct(8u);
    PE_M34StackOpcode(0x800187C0u);PE_M34BossEffectCommand(0x80140000u,0u,0u,0u,1u,2u);
    ASSERT(PE_M34StackRead(words),"fresh writers can restore validity");
    PE_M34StackVm(1);
    ASSERT(PE_M34StackRead(words),"overlay-live nested VM must keep F434 words");
    PE_M34StackField(1);
    ASSERT(PE_M34StackRead(words),"overlay-live nested field must keep F434 words");
    PE_M34StackOpcode(0x80018774u);PE_M34StackConstruct(99u);
    ASSERT(PE_M34StackRead(words),"failed construct must not wipe overlay-live record");
    /* Known-only invalidate leaves ClipSound words+sound_actor (connected
     * kite known=0x38). Reconstruct+command; F434 read recovers bit 7. */
    PE_M34StackInvalidate();
    PE_M34StackActor(0x80130000u);PE_M34StackVm(1);PE_M34StackOpcode(0x80018774u);
    PE_M34StackConstruct(8u);
    PE_M34StackOpcode(0x800187C0u);PE_M34BossEffectCommand(0x80140000u,0u,0u,0u,1u,2u);
    ASSERT(PE_M34StackRead(words) && (uint32_t)words[2]==0x80130000u,
           "intact sound residue recovers a 0x38 construct+command record");
    PE_M34StackField(0);ResetTestState();
    ASSERT(!PE_M34StackRead(words),"RAM reset cannot inherit another run's stack");
    PASS();
}

static void test_DAY1_m34_projectile_initializer(void)
{
    TEST("DAY1_m34_projectile_initializer");
    for(unsigned k=0;k<sizeof(m34_projectile_cases)/sizeof(m34_projectile_cases[0]);k++) {
        ResetTestState();
        for(unsigned i=0;i<sizeof(m34_projectile_trig)/sizeof(m34_projectile_trig[0]);i++)
            PE_StoreU32(m34_projectile_trig[i][0],m34_projectile_trig[i][1]);
        PE_StoreU16(0x8018EFF4u,3072u);
        for(unsigned i=0;i<0x48;i++)PE_StoreU8(0x80140000u+i,(uint8_t)(i*37u+13u));
        PE_StoreU32(0x800E2248u,0x80150000u);
        for(unsigned i=0;i<9;i++)PE_StoreU16(0x80190028u+i*2u,(uint16_t)m34_projectile_matrices[m34_projectile_cases[k].matrix][i]);
        PE_StoreU16(0x8019003Au,0xAA55u);
        PE_StoreU32(0x8019003Cu,(uint32_t)-12345);PE_StoreU32(0x80190040u,257u);PE_StoreU32(0x80190044u,7890u);
        PE_StoreU32(0x80150018u,(uint32_t)m34_projectile_cases[k].turn);PE_StoreU32(0x80150050u,16384u);
        PE_StoreU32(0x80150048u,(uint32_t)m34_projectile_cases[k].speed);PE_StoreU32(0x80150014u,255u);
        PE_StoreU32(0x80190084u,(uint32_t)-1234);PE_StoreU32(0x8019008Cu,2345u);PE_StoreU16(0x800942ECu,1200u);
        PE_M34BossProjectileInit(0x80140000u,m34_projectile_retained[m34_projectile_cases[k].retained]);
        for(unsigned i=0;i<18;i++) {
            if(PE_LoadU32(0x80140000u+i*4u)!=m34_projectile_cases[k].record[i])
                fprintf(stderr,"M34 projectile case%u word%u got%08X want%08X\n",k,i,PE_LoadU32(0x80140000u+i*4u),m34_projectile_cases[k].record[i]);
            ASSERT(PE_LoadU32(0x80140000u+i*4u)==m34_projectile_cases[k].record[i],"projectile record matches original retained-stack result");
        }
        for(unsigned i=0;i<9;i++)ASSERT(g_pe_gte.rt[i/3][i%3]==m34_projectile_cases[k].gte[i],"GTE rotation matches original");
        for(unsigned i=0;i<3;i++) {
            ASSERT(g_pe_gte.tr[i]==m34_projectile_cases[k].gte[i+9],"GTE translation matches original");
            ASSERT(g_pe_gte.ir[i]==m34_projectile_cases[k].gte[i+12],"GTE IR matches original");
            ASSERT(g_pe_gte.mac[i]==m34_projectile_cases[k].gte[i+15],"GTE MAC matches original");
        }
        ASSERT(!g_stub_order_count && !PE_Port_ShouldStop(),"explicit-stack initializer executes natively");
    }
    ResetTestState();PASS();
}
