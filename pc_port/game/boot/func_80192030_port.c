/* M0000I transition exit, original92030..9234C; preserve story routing,
 * fade timing, flag writes and provider ordering. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

/* func_80038D48: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80038D48_port.c (src/func_80038D48.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800868AC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800868AC_port.c (src/func_800868AC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80192030(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    uint32_t flag=PE_LoadU32(0x800A77FCu)&0x2000u;
    PE_StoreU32(0x8009D280u,0xA9400048u);
    uint32_t story=PE_LoadU32(0x800A7918u),token=0xA9400048u;
    int fade=1;
    PE_StoreU32(0x800A77F4u,999);
    if(story==128)token=0xA80650C8u;
    else if(story==120){token=0xA8003348u;fade=0;}
    else if(story==520){token=0xA80032C8u;fade=0;}
    else if(story==528)token=flag?0xA8029148u:0xA80290C8u;
    else switch(PE_LoadU32(0x8019CA68u)){
    case 0:token=0xA80281C8u;break;
    case 1:token=0xA80260C8u;break;
    case 2:token=0xA8009348u;break;
    case 3:token=0xA8046048u;break;
    case 4:
        if(story==448){token=0xA80034C8u;fade=0;}
        else token=flag?0xA80222C8u:0xA80630C8u;
        break;
    case 5:token=0xA8049048u;break;
    case 6:token=0xA8002248u;break;
    case 7:
        if(story==208 || story==376){token=0xA80033C8u;fade=0;}
        else token=story-384u<136u?0xA80201C8u:0xA8004348u;
        break;
    case 8:
        if(story==224){token=0xA80034C8u;fade=0;}
        else token=(int32_t)story<296?0xA8067248u:0xA8005448u;
        break;
    case 9:
        if(story==184 || story==328){token=0xA8003448u;fade=0;}
        else token=flag?0xA8029148u:0xA80290C8u;
        break;
    default:break;
    }
    PE_StoreU32(0x8009D280u,token);
    func_80074D28(0);if(PE_Port_StopEpoch()!=epoch)return;
    func_80074A44(1);if(PE_Port_StopEpoch()!=epoch)return;
    RECT rect={0,0,320,448};
    func_80074F44(&rect,0,0,0);if(PE_Port_StopEpoch()!=epoch)return;
    func_80074DC0(0);if(PE_Port_StopEpoch()!=epoch)return;
    func_800755F0(0x800BCE80u);if(PE_Port_StopEpoch()!=epoch)return;
    if(fade){
        func_80086C5C((int8_t)PE_LoadU8(0x800B0DB5u),30,0);
        if(PE_Port_StopEpoch()!=epoch)return;
    }
    func_800868AC(30,0);if(PE_Port_StopEpoch()!=epoch)return;
    for(unsigned i=0;i<30;i++){
        func_80073A44(0);if(PE_Port_StopEpoch()!=epoch)return;
    }
    if(fade){
        PE_StoreU8(0x800B0DB4u,255);PE_StoreU8(0x800B0DB2u,255);
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~64u);
        func_80086FF8();if(PE_Port_StopEpoch()!=epoch)return;
    }
    func_80087024();if(PE_Port_StopEpoch()!=epoch)return;
    func_80038D48();
}
