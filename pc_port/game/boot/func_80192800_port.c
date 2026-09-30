/* Original transition renderer 80192800..80193478 (798 words).
 * SHA256 926ba5886cd51536f747a8514c1ae8cd268b44b75aeaae8492f72112eef84690.
 * Draw order, pointer reloads, grouped copies, and depth writes are observable. */
#include "pe_port_compat.h"
#include "game_port.h"

static void transition_copy_matrix(pe_addr_t dst, pe_addr_t src)
{
    for(unsigned block=0;block<2;block++) {
        uint32_t words[4];
        for(unsigned i=0;i<4;i++)words[i]=PE_LoadU32(src+8u+block*16u+i*4u);
        for(unsigned i=0;i<4;i++)PE_StoreU32(dst+8u+block*16u+i*4u,words[i]);
    }
}
static void transition_copy_angles(pe_addr_t dst_global, pe_addr_t src_global)
{
    pe_addr_t dst=PE_LoadU32(dst_global),src=PE_LoadU32(src_global);
    uint32_t a=PE_LoadU32(src+0x28u),b=PE_LoadU32(src+0x2Cu);
    PE_StoreU32(dst+0x28u,a);PE_StoreU32(dst+0x2Cu,b);
    PE_StoreU16(PE_LoadU32(dst_global)+2u,0x53u);
}
/* A negative bias means there is no depth store at this call site. */
#define DRAW(g,f,force,color,bias) do { \
    pe_addr_t object=PE_LoadU32(g),context=PE_LoadU32(0x8019C9C0u); \
    if((bias)>=0)PE_StoreU32(0x801EA5E4u,(uint32_t)(bias)); \
    func_80190E04(object,context,f,force,color); \
    if(PE_Port_StopEpoch()!=epoch)return; \
} while(0)
#define LOD(g,force,mode,bias) do { \
    pe_addr_t object=PE_LoadU32(g),context=PE_LoadU32(0x8019C9C0u); \
    if((bias)>=0)PE_StoreU32(0x801EA5E4u,(uint32_t)(bias)); \
    func_80191114(object,context,force,mode); \
    if(PE_Port_StopEpoch()!=epoch)return; \
} while(0)
void func_80192800(void)
{
    uint32_t epoch=PE_Port_StopEpoch();
    if(PE_LoadU16(0x8019C034u))DRAW(0x8019CC28u,1,1,0,4001);
    DRAW(0x801E4A80u,1,1,0,4000);
    DRAW(0x8019C15Cu,1,1,0,3999);
    for(pe_addr_t g=0x8019C160u;g<=0x8019C168u;g+=4)DRAW(g,1,1,0,-1);
    static const uint32_t advance[]={10,2,40,30};
    for(unsigned i=0;i<4;i++) {
        pe_addr_t p=PE_LoadU32(0x8019C15Cu+i*4u)+0x1Cu;
        PE_StoreU32(p,PE_LoadU32(p)+advance[i]);
    }
    for(unsigned i=0;i<4;i++) {
        pe_addr_t p=PE_LoadU32(0x8019C15Cu+i*4u)+0x1Cu;
        if((int32_t)PE_LoadU32(p)>28384)PE_StoreU32(p,(uint32_t)-25384);
    }
    DRAW(0x8019CDA4u,0,1,0,1000);DRAW(0x8019CDACu,0,1,0,-1);
    DRAW(0x8019CDA0u,1,1,0,3899);DRAW(0x8019CDA8u,1,1,0,-1);
    DRAW(0x8019CDB0u,1,1,0,3898);DRAW(0x8019CDB4u,1,1,0,3897);
    DRAW(0x8019CDB8u,0,1,0,0);
    for(pe_addr_t g=0x8019CDBCu;g<=0x8019CDC8u;g+=4)DRAW(g,0,1,0,-1);
    DRAW(0x8019C180u,0,1,0,-1);
    pe_addr_t p=PE_LoadU32(0x801EA578u);
    uint16_t index=PE_LoadU16(p+2u)==0x24u?0x52u:0x24u;
    PE_StoreU16(p+2u,index);
    for(pe_addr_t g=0x801EA57Cu;g<=0x801EA584u;g+=4)PE_StoreU16(PE_LoadU32(g)+2u,index);
    DRAW(0x801EA578u,0,0,0,0);
    if(PE_LoadU16(0x8019C034u)!=2)DRAW(0x801EA580u,0,0,0,-1);
    DRAW(0x801EA57Cu,0,0,1,1000);
    if(PE_LoadU16(0x8019C034u)!=2)DRAW(0x801EA584u,0,0,1,-1);
    LOD(0x8019C820u,1,0,0);LOD(0x8019C9C8u,1,0,-1);
    LOD(0x8019C824u,1,2,1000);LOD(0x8019C9CCu,1,2,-1);
    DRAW(0x801EA588u,0,0,0,0);DRAW(0x801EA58Cu,0,0,0,-1);
    DRAW(0x801EA588u,0,0,1,10);DRAW(0x801EA58Cu,0,0,1,-1);
    DRAW(0x8019CC20u,0,1,0,0);DRAW(0x8019CC24u,0,1,1,800);
    if(PE_LoadU8(0x8019C1F0u)==1) {
        uint32_t r=func_80071A54();
        PE_StoreU16(0x8019C0D0u,(uint16_t)(PE_LoadU16(0x8019C0D0u)+(int32_t)r%40+20));
        r=func_80071A54();
        pe_addr_t dst=PE_LoadU32(0x8019C148u),src=PE_LoadU32(0x801EA578u);
        PE_StoreU16(0x8019C0D2u,(uint16_t)(PE_LoadU16(0x8019C0D2u)+(int32_t)r%52+26));
        transition_copy_matrix(dst,src);transition_copy_angles(0x8019C148u,0x801EA578u);
        static const pe_addr_t targets[]={0x8019C150u,0x8019C14Cu,0x8019C154u};
        static const pe_addr_t sources[]={0x801EA580u,0x801EA578u,0x801EA580u};
        for(unsigned i=0;i<3;i++) {
            dst=PE_LoadU32(targets[i]);src=PE_LoadU32(sources[i]);
            transition_copy_matrix(dst,src);transition_copy_angles(targets[i],sources[i]);
        }
        DRAW(0x8019C148u,0,0,0,0);DRAW(0x8019C14Cu,0,0,1,800);
    }
    for(uint32_t i=0;(int32_t)i<((int32_t)PE_LoadU32(0x8019C020u)>>1);i++) {
        uint32_t flags=PE_LoadU32(0x800A77FCu);
        PE_StoreU32(0x801EA5E4u,0);
        PE_StoreU8(0x801EA264u,(flags&0x2000u)?60:20);
        PE_StoreU8(0x801EA265u,(flags&0x2000u)?60:40);PE_StoreU8(0x801EA266u,40);
        LOD(0x8019C3B0u+i*8u,0,0,-1);
        if(PE_LoadU16(0x8019C032u)==1 && (i&3u))LOD(0x8019C3B4u+i*8u,0,1,800);
    }
    PE_StoreU32(0x801EA5E4u,0);
    for(unsigned i=0;i<8;i++)LOD(0x801E4DB0u+i*4u,0,0,-1);
    LOD(0x801E4DD0u,0,0,570);LOD(0x801E4DD4u,0,0,0);
    if(PE_LoadU16(0x8019C032u)==1) {
        uint32_t enabled=PE_LoadU8(0x8019C044u);
        PE_StoreU32(0x801EA5E4u,800);
        if(enabled) {
            static const pe_addr_t targets[]={0x801E4DDCu,0x801E4DE0u,0x801E4DE4u,0x801E4DE8u,0x801E4DF0u,0x801E4DF4u,0x801E4DF8u,0x801E4DFCu};
            for(unsigned i=0;i<8;i++)LOD(targets[i],0,1,-1);
        }
        LOD(0x801E4DECu,0,1,-1);
    }
    pe_addr_t object=PE_LoadU32(0x801EA260u),context=PE_LoadU32(0x8019C9C0u);
    PE_StoreU32(0x801EA5E0u,500);func_80190D3C(object,context);
}
#undef DRAW
#undef LOD
