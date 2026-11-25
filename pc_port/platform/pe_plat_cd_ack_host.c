#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_bios_string.h"
#include "pe_guestcode.h"
#include "pe_bootstrap.h"
#include "game_port.h"
static void cd_ack_copy(uint32_t dst,const uint8_t *packet)
{
    for(unsigned i=0;i<8u;i++) PE_StoreU8(dst+i,packet[i]);
}
int pe_plat_cd_acknowledge_host(void)
{
    uint8_t packet[8]={0};uint32_t p,irq,current,n=0,status=0,target;
    if(PE_Port_ShouldStop())return 0;
    p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,1);
    p=PE_LoadU32(0x8009b288u);irq=PE_CdLoadU8(p)&7u;
    if(!irq)return 0;
    for(;;){
        if(PE_Port_ShouldStop())return 0;
        current=PE_CdLoadU8(p)&7u;
        if(current==irq)break;
        irq=PE_CdLoadU8(p)&7u;
    }
    while(n<8u){p=PE_LoadU32(0x8009b27cu);if(!(PE_CdLoadU8(p)&0x20u))break;p=PE_LoadU32(0x8009b280u);packet[n++]=PE_CdLoadU8(p);}
    p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,1);
    p=PE_LoadU32(0x8009b288u);PE_CdStoreU8(p,7);
    p=PE_LoadU32(0x8009b284u);PE_CdStoreU8(p,7);
    int update=1;
    if(irq==3u){uint32_t index=PE_LoadU8(0x8009afd5u);if(!PE_LoadU32(0x8009b17cu+4u*index))update=0;}
    if(update){
        if(!(PE_LoadU32(0x8009afc4u)&0x10u) && (packet[0]&0x10u)){uint32_t v=PE_LoadU32(0x8009afccu);PE_StoreU32(0x8009afccu,v+1u);}
        status=packet[0]&0x1du;PE_StoreU32(0x8009afc4u,packet[0]);PE_StoreU32(0x8009afc8u,packet[1]);
    }
    if(irq==5u && (int32_t)PE_LoadU32(0x8009afc0u)>0){
        (void)PE_Printf_Guest(0x80011b44u,0,0,0,0,0,0,0);if(PE_Port_ShouldStop())return 0;
        if((int32_t)PE_LoadU32(0x8009afc0u)>0){
            uint32_t index=PE_LoadU8(0x8009afd5u);uint32_t a2=PE_LoadU32(0x8009afc4u);uint32_t a3=PE_LoadU32(0x8009afc8u);uint32_t a1=PE_LoadU32(0x8009afdcu+4u*index);
            (void)PE_Printf_Guest(0x80011b50u,a1,a2,a3,0,0,0,0);if(PE_Port_ShouldStop())return 0;
        }
    }
    if(irq==0u || irq>5u){
        (void)PE_GuestCall("CD unknown interrupt message",0x80073c5cu,1u,0x80011b6cu,0,0,0);if(PE_Port_ShouldStop())return 0;
        (void)PE_Printf_Guest(0x80011b80u,irq,0,0,0,0,0,0);return 0;
    }
    target=PE_LoadU32(0x80011b8cu+4u*(irq-1u));
    switch(target){
    case 0x8007ae5cu:
        if(status && n==1u)status=0;
        PE_StoreU8(0x8009b295u,status?5u:1u);cd_ack_copy(0x800a3468u,packet);
        p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,0);p=PE_LoadU32(0x8009b288u);PE_CdStoreU8(p,0);return 4;
    case 0x8007ae10u:
        PE_StoreU8(0x8009b294u,status?5u:2u);cd_ack_copy(0x800a3460u,packet);return 2;
    case 0x8007ad10u:{
        uint32_t state;if(status)state=5;else{uint32_t index=PE_LoadU8(0x8009afd5u);state=PE_LoadU32(0x8009b07cu+4u*index)?3u:2u;}
        PE_StoreU8(0x8009b294u,state);cd_ack_copy(0x800a3460u,packet);return state==3u?1:2;
    }
    case 0x8007aedcu:{
        PE_StoreU8(0x8009b296u,4);uint8_t v=PE_LoadU8(0x8009b296u);PE_StoreU8(0x8009b295u,v);cd_ack_copy(0x800a3470u,packet);cd_ack_copy(0x800a3468u,packet);return 4;
    }
    case 0x8007af5cu:{
        PE_StoreU8(0x8009b295u,5);uint8_t v=PE_LoadU8(0x8009b295u);PE_StoreU8(0x8009b294u,v);cd_ack_copy(0x800a3460u,packet);cd_ack_copy(0x800a3468u,packet);return 6;
    }
    default:
        target=PE_LoadU32(0x80011b8cu+4u*(irq-1u));
        Bootstrap_ReturnVoid4Indirect("func_8007AAB4_jump_table","func_8007AAB4",target,0,0,0,0);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
    }
}
