/* Whole M0000I transition constructor, original 80196498..80197BA0.
 * Resource-relative positions, paired pool objects and entry-mode cameras. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "transition_constructor_data.h"

static uint32_t transition_short(pe_addr_t a)
{ return (uint32_t)(int32_t)(int16_t)PE_LoadU16(a); }

static void transition_position(pe_addr_t slot,pe_addr_t source)
{
    for(unsigned j=0;j<3;j++)
        PE_StoreU32(PE_LoadU32(slot)+0x20u+j*4u,transition_short(source+j*2u));
}

static int transition_sample(uint32_t time,uint32_t id,uint32_t value[3],int *count)
{
    unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t package=func_8006EC6C(0x801D0260u,2);
    if(PE_Port_StopEpoch()!=epoch)return 0;
    const pe_addr_t scratch=0x1F800260u;
    uint32_t saved[6];
    for(unsigned j=0;j<6;j++)saved[j]=PE_LoadU32(scratch+j*4u);
    *count=func_8018F55C(time,id,package,scratch,scratch+16u);
    for(unsigned j=0;j<3;j++)value[j]=PE_LoadU32(scratch+j*4u);
    for(unsigned j=0;j<6;j++)PE_StoreU32(scratch+j*4u,saved[j]);
    return PE_Port_StopEpoch()==epoch;
}

void func_80196498(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    func_80191854();if(PE_Port_StopEpoch()!=epoch)return;
    func_80190998();func_80195F6C();
    func_80078E34(0x8019C11Cu);func_80078E64(0x8019C0FCu);
    func_80078FC4(64,64,64);func_80077E64(4096,8192,768);func_80078FE4(255,255,255);
    for(unsigned i=0;i<2;i++){
        pe_addr_t a=0x80091648u+i*16u;
        uint32_t y=PE_LoadU16(a+2u),x=PE_LoadU16(a);
        PE_StoreU16(a+8u,(uint16_t)(((y&256u)>>4)|((x&1023u)>>6)|32u|((y&512u)<<2)));
        PE_StoreU16(a+10u,(uint16_t)((PE_LoadU16(a+6u)<<6)|((PE_LoadU16(a+4u)>>4)&63u)));
    }
    if(!PE_LoadU32(0x8019C008u)){
        pe_addr_t p=func_8006EC6C(0x801D0260u,0);if(PE_Port_StopEpoch()!=epoch)return;
        func_800371B0(p);if(PE_Port_StopEpoch()!=epoch)return;
    }
    if(PE_LoadU32(0x8019C008u)){
        pe_addr_t p=func_8006EC6C(0x801D0260u,1);if(PE_Port_StopEpoch()!=epoch)return;
        func_800371B0(p);if(PE_Port_StopEpoch()!=epoch)return;
    }
    pe_addr_t package=func_8006EC6C(0x801D0260u,2);if(PE_Port_StopEpoch()!=epoch)return;
    pe_addr_t positions=package+PE_LoadU32(package)+8u;
    for(unsigned i=0;i<sizeof(transition_record_constants)/sizeof(transition_record_constants[0]);i++)
        PE_StoreU8(0x801EA370u+transition_record_constants[i].offset,transition_record_constants[i].value);
    uint32_t flags=PE_LoadU32(0x800A77FCu);
    for(unsigned i=0;i<sizeof(transition_record_flags)/sizeof(transition_record_flags[0]);i++)
        PE_StoreU8(0x801EA370u+transition_record_flags[i].offset,(uint8_t)((flags>>transition_record_flags[i].bit)&1u));
    for(unsigned i=0;i<10;i++){
        pe_addr_t record=0x801EA370u+i*52u,slot=0x801E4DB0u+i*4u;
        for(unsigned j=0;j<3;j++)PE_StoreU16(record+j*2u,PE_LoadU16(positions+i*8u+j*2u));
        pe_addr_t resource=func_8006EC6C(0x8019CE10u,0);if(PE_Port_StopEpoch()!=epoch)return;
        pe_addr_t object=func_80190C1C(0x8019C340u,transition_short(record+20u),transition_short(record+24u),transition_short(record+22u),transition_short(record+30u),transition_short(record+28u),resource);
        PE_StoreU32(slot,object);transition_position(slot,record);
        PE_StoreU16(PE_LoadU32(slot)+0x2Eu,PE_LoadU16(record+26u));
        resource=func_8006EC6C(0x8019CE10u,0);if(PE_Port_StopEpoch()!=epoch)return;
        object=func_80190C1C(0x8019C340u,transition_short(record+20u),transition_short(record+24u),transition_short(record+22u),1000,500,resource);
        slot+=40u;PE_StoreU32(slot,object);transition_position(slot,record);
        PE_StoreU16(PE_LoadU32(slot)+0x2Eu,PE_LoadU16(record+26u));
        PE_StoreU16(PE_LoadU32(slot)+0x2Cu,2048);
    }
    pe_addr_t resource=func_8006EC6C(0x8019CE10u,0);if(PE_Port_StopEpoch()!=epoch)return;
    pe_addr_t other=func_8006EC6C(0x8019CE10u,1);if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU32(0x801EA260u,func_80190AEC(0x8019C340u,other));
    static const struct { uint32_t id,slot; } singles[]={
        {0x51,0x8019C180},{0x28,0x8019CDA0},{0x29,0x8019CDA4},{0x2A,0x8019CDA8},
        {0x2B,0x8019CDAC},{0x2C,0x8019CDB0},{0x2D,0x8019CDB4},{0x2E,0x8019CDB8},
        {0x2F,0x8019CDBC},{0x30,0x8019CDC0},{0x31,0x8019CDC4},{0x32,0x8019CDC8},
        {0x25,0x801E4A80},{0x54,0x8019CC28},{0x23,0x801EA588},{0x23,0x801EA58C},
        {0x52,0x801EA578},{0x52,0x801EA580},{0x52,0x801EA57C},{0x52,0x801EA584},
        {0x53,0x8019C148},{0x53,0x8019C150},{0x53,0x8019C14C},{0x53,0x8019C154},
        {0x21,0x8019CC20},{0x21,0x8019CC24},{0x26,0x8019C15C},{0x26,0x8019C160},
        {0x26,0x8019C164},{0x26,0x8019C168}};
    for(unsigned i=0;i<30;i++)PE_StoreU32(singles[i].slot,func_80190B78(0x8019C340u,singles[i].id,resource));
    static const pe_addr_t pairs[]={0x8019C820u,0x8019C824u,0x8019C9C8u,0x8019C9CCu};
    for(unsigned i=0;i<4;i++)PE_StoreU32(pairs[i],func_80190C1C(0x8019C340u,34,32,32,(i&1)?20:10000,(i&1)?10:8000,resource));
    for(unsigned i=2;i<4;i++)PE_StoreU16(PE_LoadU32(pairs[i])+0x2Eu,200);
    for(unsigned i=2;i<4;i++){
        pe_addr_t o=PE_LoadU32(pairs[i]);
        PE_StoreU32(o+0x20u,820);PE_StoreU32(o+0x24u,i==2?20u:(uint32_t)-20);
        PE_StoreU32(o+0x28u,PE_LoadU32(o+0x28u)-4000u);
    }
    static const int32_t offsets[4][3]={{-13000,7000,-6000},{-2000,9000,2000},{5000,6000,4000},{10000,7000,3000}};
    for(unsigned i=0;i<4;i++)for(unsigned j=0;j<3;j++)
        PE_StoreU32(PE_LoadU32(0x8019C15Cu+i*4u)+0x1Cu+j*4u,(uint32_t)offsets[i][j]);
    for(unsigned i=0;i<9;i++){
        package=func_8006EC6C(0x801D0260u,2);if(PE_Port_StopEpoch()!=epoch)return;
        pe_addr_t path=package+PE_LoadU32(package+4u+i*4u);
        int count=(int16_t)PE_LoadU16(path+6u);
        for(int j=0;j<count;j++){
            pe_addr_t pos=path+8u+(uint32_t)j*8u;
            for(unsigned second=0;second<2;second++){
                pe_addr_t o=func_80190C1C(0x8019C340u,51u+i*3u,52u+i*3u,53u+i*3u,
                    second?2000u:PE_LoadU16(0x8019C0D4u+i*2u),second?500u:PE_LoadU16(0x8019C0E8u+i*2u),resource);
                uint32_t n=PE_LoadU32(0x8019C020u);pe_addr_t slot=0x8019C3B0u+n*4u;
                PE_StoreU32(slot,o);
                PE_StoreU32(o+0x20u,transition_short(pos));
                o=PE_LoadU32(slot);uint32_t y=transition_short(pos+2u);
                if(!second)PE_StoreU32(0x8019C020u,n+1u);
                PE_StoreU32(o+0x24u,y);
                PE_StoreU32(PE_LoadU32(slot)+0x28u,transition_short(pos+4u));
                if(second){
                    o=PE_LoadU32(slot);PE_StoreU32(0x8019C020u,n+1u);
                    PE_StoreU16(o+0x2Cu,(uint16_t)(PE_LoadU16(o+0x2Cu)+2048u));
                }
            }
        }
    }
    uint32_t variant=transition_short(0x8019CC52u);
    PE_StoreU32(PE_LoadU32(0x801E4A80u)+0x1Cu,0);
    PE_StoreU32(PE_LoadU32(0x801E4A80u)+0x20u,1880);
    PE_StoreU32(PE_LoadU32(0x801E4A80u)+0x24u,0);
    func_80195994(variant,1,1,0);if(PE_Port_StopEpoch()!=epoch)return;
    variant=transition_short(0x8019CC52u);PE_StoreU16(0x8019C058u,0);
    uint32_t value[3];int count;
    for(unsigned i=0;i<10;i++){
        if(!transition_sample(i<<8,variant+64u,value,&count))return;
        for(unsigned j=0;j<3;j++)PE_StoreU32(0x8019CAA8u+i*16u+j*4u,value[j]<<16);
    }
    if(PE_LoadU32(0x8019BFF8u)==1){
        uint32_t v=PE_LoadU32(0x8019BFFCu);
        PE_StoreU16(0x8019C02Cu,0);PE_StoreU8(0x8019C045u,1);PE_StoreU8(0x8019C044u,1);
        PE_StoreU32(0x8019C048u,1);PE_StoreU16(0x8019CC52u,(uint16_t)v);
    }
    if(!PE_LoadU32(0x8019BFF4u)){
        PE_StoreU16(0x8019CC52u,7);PE_StoreU32(0x8019CC1Cu,0);
        PE_StoreU8(0x8019C045u,0);PE_StoreU8(0x8019C046u,0);PE_StoreU32(0x8019C048u,0);
        PE_StoreU8(0x8019C044u,0);PE_StoreU16(0x8019C02Cu,0);PE_StoreU16(0x8019C02Eu,0);PE_StoreU16(0x8019C034u,10);
        func_80195994(7,0,0,0);if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU8(0x8019C00Du,1);
    }
    if(PE_LoadU32(0x8019BFF4u)==7){
        uint32_t id=PE_LoadU8(0x8019C017u);
        PE_StoreU16(0x8019CC52u,9);PE_StoreU32(0x8019CC1Cu,0);PE_StoreU16(0x8019C02Cu,0);
        PE_StoreU8(0x8019C045u,0);PE_StoreU8(0x8019C044u,1);PE_StoreU32(0x8019C048u,1);PE_StoreU32(0x8019BFF8u,1);
        if(!transition_sample(0,id+1u,value,&count))return;
        PE_StoreU16(0x8019C054u,0);
        for(unsigned j=0;j<3;j++)PE_StoreU32(0x8019C810u+j*4u,value[j]);
        if(!transition_sample(0,id,value,&count))return;
        PE_StoreU16(0x8019C050u,0);PE_StoreU8(0x8019C040u,(uint8_t)(count-2));
        PE_StoreU8(0x8019C041u,0);PE_StoreU8(0x8019C042u,(uint8_t)id);PE_StoreU16(0x8019C02Cu,0);
        for(unsigned j=0;j<3;j++)PE_StoreU32(0x8019C330u+j*4u,value[j]);
    }
}
