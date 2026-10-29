/* Menu-tail provenance (func_801942FC -> func_800375E0): unproven list
 * entries never become invented digits.  Their groups (and every later group,
 * since an unknown value may be the -1 terminator) are marked unknown; known
 * entries are written exactly as func_800375E0 writes them. */
static void test_message_partial_open(void)
{
    TEST("message_partial_open");
    const pe_addr_t rec=0x800BCEA8u,list=0x80100000u;
    for(unsigned i=0;i<4u*56u;i++)PE_StoreU8(rec+i,0);
    const uint16_t v[5]={0,0,0x1111,0x2222,0xC2C4};
    for(unsigned i=0;i<5;i++)PE_StoreU16(list+i*2u,v[i]);
    PE_MessageOpenPartial(40,3,list,(1u<<2)|(1u<<3));
    ASSERT(PE_LoadU8(rec)==1u && PE_LoadU16(rec+0x10u)==40u,"record not opened");
    ASSERT(PE_LoadU8(rec+0x1Fu)==1u && PE_LoadU8(rec+0x25u)==1u,"known groups not written");
    ASSERT(PE_LoadU8(rec+0x1Au+2u*6u+5u)==0u,"unknown group written");
    ASSERT(PE_MessageDigitsKnown(rec,0)&&PE_MessageDigitsKnown(rec,1),"known groups flagged");
    for(unsigned g=2;g<5;g++)ASSERT(!PE_MessageDigitsKnown(rec,g),"unknown group not flagged");
    /* Reopen with a short list: groups past the terminator stay unknown. */
    PE_StoreU8(rec,0);PE_StoreU16(list,7);PE_StoreU16(list+2u,0xFFFF);
    func_800375E0(41,3,list);
    ASSERT(PE_MessageDigitsKnown(rec,0)&&!PE_MessageDigitsKnown(rec,2),"stale groups lost provenance");
    /* A full known list proves every group again. */
    PE_StoreU8(rec,0);for(unsigned i=0;i<5;i++)PE_StoreU16(list+i*2u,(uint16_t)(i+1u));
    func_800375E0(42,3,list);
    for(unsigned g=0;g<5;g++)ASSERT(PE_MessageDigitsKnown(rec,g),"full list left unknown group");
    ASSERT(PE_MessageDigitsKnown(rec+4u*56u,0),"out-of-table record must read known");
    PASS();
}
