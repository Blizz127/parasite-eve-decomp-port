/* Original item animation, escape judgement and command dispatch, 120D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

extern void func_8001A680(pe_addr_t actor, unsigned int command);

/* Native game adapter for the matched 80022210 body. Stop boundaries unwind
 * before later item or queue work; normal guest-memory order is retained. */
void func_80022210(void)
{
    pe_addr_t rec=PE_LoadU32(0x8009D278u), actor;
    if (PE_LoadU32(rec+0x4Cu)&0x200000u) {
        func_8001A680(PE_LoadU32(0x8009D254u),14u);
        if (PE_Port_ShouldStop()) return;
        actor=PE_LoadU32(0x8009D254u);
        func_8006DE80(0x4B3,0,(int16_t)PE_LoadU16(actor+0x2Au),
            (int16_t)PE_LoadU16(actor+0x2Eu),(int16_t)PE_LoadU16(actor+0x32u));
        if (PE_Port_ShouldStop()) return;
        PE_StoreU32(0x8009D1A0u,PE_LoadU32(0x8009D1A0u)|0x100u);
        rec=PE_LoadU32(0x8009D278u);
        PE_StoreU32(rec+0x4Cu,PE_LoadU32(rec+0x4Cu)&~0x200000u);
    }
    actor=PE_LoadU32(0x8009D254u);
    if (PE_LoadU8(actor+0xFu)==PE_LoadU16(actor+0x1Au)) {
        rec=PE_LoadU32(0x8009D278u);
        func_8001A680(actor,PE_LoadU8(rec+0x12u));
        if (PE_Port_ShouldStop()) return;
        rec=PE_LoadU32(0x8009D278u);
        PE_StoreU32(rec+0x4Cu,PE_LoadU32(rec+0x4Cu)|0x200000u);
        func_80023E14((int16_t)PE_LoadU16(0x800BE834u+(uint32_t)PE_LoadU8(0x8009D1D4u)*8u)-3);
        if (PE_Port_ShouldStop()) return;
        (void)func_8006F39C(0x55u,PE_LoadU32(0x8009D254u));
        if (PE_Port_ShouldStop()) return;
        PE_StoreU8(0x8009D1D4u,(uint8_t)(PE_LoadU8(0x8009D1D4u)+1u));
        {
            int16_t command=(int16_t)PE_LoadU16(0x800BE834u+(uint32_t)PE_LoadU8(0x8009D1D4u)*8u);
            actor=PE_LoadU32(0x8009D254u);
            if (command<3 || command>=0x197 || PE_LoadU8(actor+0xEu)<4u)
                PE_StoreU32(0x8009D1A0u,PE_LoadU32(0x8009D1A0u)&~0x100u);
        }
    }
}

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
