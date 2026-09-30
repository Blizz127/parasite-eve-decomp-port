/* Original item animation, escape judgement and command dispatch, 120D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80022210: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80022210_port.c (src/func_80022210.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int8_t func_800255E4(void)
{
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu),record;
    int maximum=0;
    int8_t result=0;
    for (;actor;actor=PE_LoadU32(actor+4u)) {
        pe_addr_t body;
        int level;
        if (actor==PE_LoadU32(0x8009D254u)) continue;
        body=PE_LoadU32(actor);
        if (!body || (int32_t)PE_LoadU32(body+16u)<=0) continue;
        level=(int8_t)PE_LoadU8(body+4u);
        if (maximum<level) maximum=level;
        if (PE_LoadU32(body+204u)&0x40000u) {result=-1;break;}
    }
    record=PE_LoadU32(0x8009D278u);
    if (!result) {
        int8_t difference=(int8_t)(PE_LoadU8(record+4u)-maximum);
        int8_t chance=difference>=2?80:difference==1 || !difference?40:difference==-1?25:15;
        unsigned attempts;
        if ((int16_t)PE_LoadU16(record+12u)*10<(int16_t)PE_LoadU16(record+28u))
            chance=(int8_t)(chance*3/2);
        attempts=(PE_LoadU32(record+76u)>>25u)&7u;
        switch (attempts) {
        case 1:chance=(int8_t)(chance*3/2);break;
        case 2:chance=(int8_t)(chance*2);break;
        case 3:chance=(int8_t)(chance*3);break;
        case 4:chance=(int8_t)(chance*4);break;
        case 5:chance=100;break;
        }
        result=(int32_t)func_80071A54()%100<chance;
    }
    if (result<=0) {
        uint32_t flags,attempts,language;
        PE_StoreU8(0x8009D1CEu,1u);
        language=(uint32_t)func_8005BCB0();
        PE_StoreU32(0x8009D1F8u,result==-1?0x800915C0u+language*14u:0x8009159Cu+language*17u);
        record=PE_LoadU32(0x8009D278u);flags=PE_LoadU32(record+76u);attempts=(flags>>25u)&7u;
        if (attempts<5u) PE_StoreU32(record+76u,(flags&0xF1FFFFFFu)|((attempts+1u)<<25u));
    }
    return result;
}

void func_80021DE0(void)
{
    uint8_t index=PE_LoadU8(0x8009D1D4u);
    pe_addr_t actor;
    int16_t command;
    if (index>=PE_LoadU8(0x8009CE3Cu)) {
        PE_StoreU8(0x8009D1D4u,0u);PE_StoreU8(0x8009CE3Cu,0u);return;
    }
    actor=PE_LoadU32(0x8009D254u);
    PE_StoreU32(actor+104u,0u);PE_StoreU32(actor+108u,0u);PE_StoreU32(actor+112u,0u);
    if (PE_LoadU8(actor+14u)<4u) return;
    command=(int16_t)PE_LoadU16(0x800BE834u+index*8u);
    if (command<3) func_80021F38();
    else if (command<387) func_80022210();
    else if (command<407) func_80022394();
    else if (command<409) {
        func_8001A680_command_cut(actor,13u);
        actor=PE_LoadU32(0x8009D254u);
        PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)|1u);
        PE_StoreU8(0x8009D1D4u,(uint8_t)(PE_LoadU8(0x8009D1D4u)+1u));
        PE_StoreU32(actor+152u,PE_LoadU32(actor+152u)|0x100u);
    } else {
        if (func_800255E4()==1) {
            func_8001A680_command_cut(PE_LoadU32(0x8009D254u),PE_LoadU8(PE_LoadU32(0x8009D278u)+18u));
            PE_StoreU32(0x8009D28Cu,4u);
        }
        PE_StoreU8(0x8009D1D4u,(uint8_t)(PE_LoadU8(0x8009D1D4u)+1u));
    }
}
