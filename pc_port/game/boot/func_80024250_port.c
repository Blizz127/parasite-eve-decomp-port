/* Original PE ability application and field synchronization (120D8/41D10.s).
 * Costs are fixed-point PE units; all record arithmetic keeps PS1 wrapping. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t pe_apply_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
static pe_addr_t pe_apply_record(void) {return pe_apply_ram(PE_LoadU32(0x8009D278u));}

static void pe_apply_cure(unsigned mask)
{
    pe_addr_t record=pe_apply_record();uint32_t flags=PE_LoadU32(record+76u);
    if ((flags&mask)!=mask) {
        PE_StoreU32(record+76u,flags&~mask);
        if (mask==12u) PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&~16u);
    }
}

void func_80024250(int index,pe_addr_t actor)
{
    pe_addr_t record;uint32_t cost=0u,flags;
    if (!(D_8009D1A0&2u)) {
        pe_addr_t aya=pe_apply_ram(PE_LoadU32(0x8009D254u));
        func_8006DDCC(PE_LoadU16(0x80010760u+(uint32_t)index*2u),1,
            (int16_t)PE_LoadU16(aya+42u),(int16_t)PE_LoadU16(aya+46u),
            (int16_t)PE_LoadU16(aya+50u));
        PE_StoreU32(0x8009D278u,PE_LoadU32(aya));
    }
    record=pe_apply_record();flags=PE_LoadU32(record+76u);
    switch ((uint32_t)index) {
    case 0:case 1:case 2: {
        static const unsigned hp[]={30,60,280},pe[]={60,120,500};
        PE_StoreU16(record+12u,(uint16_t)(PE_LoadU16(record+12u)+hp[index]));
        cost=pe[index]<<16u;break;
    }
    case 3:pe_apply_cure(3u);cost=100u<<16u;break;
    case 4:case 18:
        if (index==18) PE_StoreU16(record+12u,PE_LoadU16(record+28u));
        pe_apply_cure(3u);pe_apply_cure(12u);pe_apply_cure(48u);pe_apply_cure(192u);
        record=pe_apply_record();PE_StoreU32(record+76u,PE_LoadU32(record+76u)&~4096u);
        cost=(index==18?1300u:600u)<<16u;break;
    case 5:
        PE_StoreU32(record+76u,flags|512u);
        PE_StoreU32(record+8u,PE_LoadU32(record+8u)-(uint32_t)((int32_t)PE_LoadU32(record+40u)/3));break;
    case 7: {
        static const unsigned shifts[]={0,2,4,14,16,12,10};unsigned i;
        pe_addr_t body=pe_apply_ram(PE_LoadU32(pe_apply_ram(actor)));
        uint32_t scan=PE_LoadU32(0x8009D1ACu);
        for (i=0;i<7;i++) {
            unsigned bit=12u+i;
            scan=(scan&~(1u<<bit))|((((PE_LoadU32(body+204u)>>shifts[i])&3u)==2u)<<bit);
            PE_StoreU32(0x8009D1ACu,scan);
        }
        body=pe_apply_ram(PE_LoadU32(pe_apply_ram(actor)));
        scan=(scan&~3072u)|(((PE_LoadU32(body+204u)>>6u)&3u)==2u?1024u:0u);
        PE_StoreU32(0x8009D1ACu,scan);PE_StoreU8(0x8009D1ACu,75u);
        PE_StoreU32(0x8009D1A8u,actor);
        PE_StoreU32(0x8009D1ACu,(PE_LoadU32(0x8009D1ACu)&~768u)|256u);
        cost=50u<<16u;break;
    }
    case 8:case 10: {
        pe_addr_t body=pe_apply_ram(PE_LoadU32(pe_apply_ram(actor)));
        unsigned resistance=(PE_LoadU32(body+204u)>>(index==8?8u:10u))&3u;
        cost=(index==8?90u:150u)<<16u;
        if (resistance==2u || (!resistance && !(func_80071A54()&1u))) {
            body=pe_apply_ram(PE_LoadU32(pe_apply_ram(actor)));
            if (index==8) PE_StoreU32(body,PE_LoadU32(body)|1u);
            else {
                int32_t random=(int32_t)func_80071A54();
                body=pe_apply_ram(PE_LoadU32(pe_apply_ram(actor)));
                PE_StoreU32(body,(PE_LoadU32(body)&~14u)|(((uint32_t)(random%3+3)&7u)<<1u));
                func_8001A680_command_cut(actor,4u);
                PE_StoreU32(pe_apply_ram(actor+152u),PE_LoadU32(pe_apply_ram(actor+152u))|4096u);
            }
        }
        break;
    }
    case 9:
        flags|=256u;PE_StoreU32(record+76u,flags);PE_StoreU16(0x8009D228u,450u);
        if ((flags&192u)==64u || (flags&192u)==128u) PE_StoreU32(record+76u,flags&~192u);
        cost=200u<<16u;break;
    case 11:PE_StoreU32(record+76u,flags|1024u);cost=400u<<16u;break;
    case 12:PE_StoreU32(record+76u,flags|2048u);cost=1000u<<16u;break;
    case 19:PE_StoreU32(record+76u,(flags|0x80000u)&~0x200000u);break;
    default:break;
    }
    record=pe_apply_record();
    if ((int16_t)PE_LoadU16(record+28u)<(int16_t)PE_LoadU16(record+12u))
        PE_StoreU16(record+12u,PE_LoadU16(record+28u));
    record=pe_apply_record();
    if (PE_LoadU32(pe_apply_ram(PE_LoadU32(record+108u)+4u))&512u)
        cost=(uint32_t)((int32_t)(cost*2u)/3);
    PE_StoreU32(record+8u,PE_LoadU32(record+8u)-cost);
}

void func_80051770(int32_t ability)
{
    pe_addr_t aya,record,weapon;uint32_t bank;
    func_80024250(ability,PE_LoadU32(0x8009D254u));
    aya=PE_LoadU32(0x8009D254u);if (!aya) return;
    record=PE_LoadU32(aya);if (!record) return;
    PE_StoreU16(0x800C0E08u,PE_LoadU16(record+12u));
    if (!PE_LoadU32(record+104u)) return;
    bank=func_80052F0C();func_80052E30(0u);
    weapon=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
    if (weapon) PE_StoreU16(weapon+10u,PE_LoadU32(PE_LoadU32(record+104u)+12u)&1023u);
    func_80052E30(bank);
}
