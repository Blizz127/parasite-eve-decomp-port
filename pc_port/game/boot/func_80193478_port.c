/* Original M0000I scene motion: 93478..938E8,938E8..939B0,939B0..93AB0.
 * Preserve retail's asymmetric bank damping and blockwise overlapping copies. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static pe_addr_t motion_actor(pe_addr_t slot)
{return PE_LoadU32(slot);}

static void motion_clamp(pe_addr_t slot,int limit)
{
    pe_addr_t actor=motion_actor(slot);
    if((int16_t)PE_LoadU16(actor+44u)>limit) {
        PE_StoreU16(actor+44u,(uint16_t)limit);
        actor=motion_actor(slot);
    }
    if((int16_t)PE_LoadU16(actor+44u)<-limit)
        PE_StoreU16(actor+44u,(uint16_t)-limit);
}

static void motion_copy(pe_addr_t dest_slot,pe_addr_t source_slot)
{
    pe_addr_t dst=motion_actor(dest_slot),src=motion_actor(source_slot);
    uint32_t words[4];
    for(unsigned block=8;block<40;block+=16) {
        for(unsigned i=0;i<4;i++)words[i]=PE_LoadU32(src+block+i*4u);
        for(unsigned i=0;i<4;i++)PE_StoreU32(dst+block+i*4u,words[i]);
    }
    dst=motion_actor(dest_slot);src=motion_actor(source_slot);
    words[0]=PE_LoadU32(src+40u);words[1]=PE_LoadU32(src+44u);
    PE_StoreU32(dst+40u,words[0]);PE_StoreU32(dst+44u,words[1]);
}

void func_80193478(void)
{
    const pe_addr_t scratch=0x1F800380u,first=0x801EA578u,second=0x801EA580u;
    uint8_t saved[16];unsigned epoch=PE_Port_StopEpoch();
    for(unsigned i=0;i<16;i++)saved[i]=PE_LoadU8(scratch+i);
    for(unsigned i=0;i<4;i++) {
        pe_addr_t package=func_8006EC6C(0x801D0260u,2);
        if(PE_Port_StopEpoch()!=epoch)goto done;
        static const pe_addr_t slots[]={0x801EA588u,0x801EA58Cu,0x801EA578u,0x801EA580u};
        pe_addr_t actor=motion_actor(slots[i]);
        func_8018F55C(PE_LoadU32(i<2?0x8019C038u:0x8019C03Cu),74u+i,package,
                       actor+28u,i<2?actor+40u:scratch+(i-2u)*8u);
        if(PE_Port_StopEpoch()!=epoch)goto done;
    }
    PE_StoreU16(motion_actor(first)+40u,PE_LoadU16(scratch));
    PE_StoreU16(motion_actor(first)+42u,PE_LoadU16(scratch+2u));
    pe_addr_t actor=motion_actor(first);
    PE_StoreU16(actor+44u,(uint16_t)(PE_LoadU16(actor+44u)+PE_LoadU16(scratch+4u)));
    motion_clamp(first,512);
    actor=motion_actor(first);
    int32_t bank=(int16_t)PE_LoadU16(actor+44u);
    if(bank>0) {
        PE_StoreU16(actor+44u,(uint16_t)(bank-8));
        actor=motion_actor(first);bank=(int16_t)PE_LoadU16(actor+44u);
    }
    if(bank<0)PE_StoreU16(actor+44u,(uint16_t)(bank+8));
    PE_StoreU16(motion_actor(second)+40u,PE_LoadU16(scratch+8u));
    PE_StoreU16(motion_actor(second)+42u,PE_LoadU16(scratch+10u));
    actor=motion_actor(second);
    PE_StoreU16(actor+44u,(uint16_t)(PE_LoadU16(actor+44u)+PE_LoadU16(scratch+12u)));
    motion_clamp(second,384);
    /* Original936D0..93728 tests second but modifies first. */
    if((int16_t)PE_LoadU16(motion_actor(second)+44u)>0) {
        actor=motion_actor(first);PE_StoreU16(actor+44u,(uint16_t)(PE_LoadU16(actor+44u)-8u));
    }
    if((int16_t)PE_LoadU16(motion_actor(second)+44u)<0) {
        actor=motion_actor(first);PE_StoreU16(actor+44u,(uint16_t)(PE_LoadU16(actor+44u)+8u));
    }
    static const pe_addr_t yaw_slots[]={0x801EA588u,0x801EA58Cu,0x801EA578u,0x801EA580u};
    for(unsigned i=0;i<4;i++) {
        actor=motion_actor(yaw_slots[i]);PE_StoreU16(actor+42u,(uint16_t)(PE_LoadU16(actor+42u)-1024u));
    }
    motion_copy(0x801EA57Cu,first);motion_copy(0x801EA584u,second);
    uint32_t time=PE_LoadU32(0x8019C038u),other=PE_LoadU32(0x8019C03Cu);
    PE_StoreU32(0x8019C038u,time+4u);PE_StoreU32(0x8019C03Cu,other+16u);
 done:
    for(unsigned i=0;i<16;i++)PE_StoreU8(scratch+i,saved[i]);
}

static void motion_path(uint32_t id,int delta)
{
    const pe_addr_t scratch=0x1F800380u;
    uint8_t saved[24];unsigned epoch=PE_Port_StopEpoch();
    for(unsigned i=0;i<24;i++)saved[i]=PE_LoadU8(scratch+i);
    PE_StoreU16(0x8019C058u,delta?32:0);
    for(unsigned i=0;i<10;i++) {
        pe_addr_t package=func_8006EC6C(0x801D0260u,2);
        if(PE_Port_StopEpoch()!=epoch)break;
        func_8018F55C(i*256u,(uint32_t)((int32_t)(int16_t)id+64),package,scratch,scratch+16u);
        if(PE_Port_StopEpoch()!=epoch)break;
        for(unsigned j=0;j<3;j++) {
            uint32_t value=PE_LoadU32(scratch+j*4u)<<16;
            pe_addr_t current=0x8019CAA8u+i*16u+j*4u;
            if(delta) {
                value=(uint32_t)((int32_t)(value-PE_LoadU32(current))>>5);
                PE_StoreU32(0x801EA268u+i*16u+j*4u,value);
            } else PE_StoreU32(current,value);
        }
    }
    for(unsigned i=0;i<24;i++)PE_StoreU8(scratch+i,saved[i]);
}
void func_801938E8(uint32_t id){motion_path(id,0);}
void func_801939B0(uint32_t id){motion_path(id,1);}
