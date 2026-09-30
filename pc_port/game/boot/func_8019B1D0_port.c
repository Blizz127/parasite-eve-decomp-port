/* Original transition fixed-depth model emitter, 8019B1D0..8019BD78.
 * 746 words, SHA256 2c946ccd523fdfd5dbcc32a802e0545f54e8f1e8f18d0a656a47f04b3f11f658.
 * Eight original primitive streams. Read/write ordering retains RAM aliases,
 * scratch-coordinate history, and the inline FT4 AVSZ3-before-RTPS sequence.
 */
#include "pe_port_compat.h"
#include "pe_sdk.h"

static void fixed_model_vertex(int16_t v[3], pe_addr_t p)
{
    uint32_t xy=PE_LoadU32(p),z=PE_LoadU32(p+4u);
    v[0]=(int16_t)xy;v[1]=(int16_t)(xy>>16);v[2]=(int16_t)z;
}

static void fixed_model_copy(pe_addr_t dst, pe_addr_t src)
{
    PE_StoreU32(dst,PE_LoadU32(src));
}

static void fixed_model_link(pe_addr_t packet)
{
    uint32_t tag=PE_LoadU32(packet);
    pe_addr_t descriptor=PE_LoadU32(0x8019C9C0u);
    uint32_t index=PE_LoadU32(0x1F800018u);
    pe_addr_t ot=PE_LoadU32(descriptor+4u);
    uint32_t old=PE_LoadU32(ot+index*4u);
    PE_StoreU32(packet,(tag&0xFF000000u)|(old&0xFFFFFFu));
    index=PE_LoadU32(0x1F800018u);
    ot=PE_LoadU32(descriptor+4u);
    old=PE_LoadU32(ot+index*4u);
    PE_StoreU32(ot+index*4u,(old&0xFF000000u)|(packet&0xFFFFFFu));
}

void func_8019B1D0(pe_addr_t unused, pe_addr_t model, uint32_t submodel)
{
    static const unsigned stride[8]={0x1C,0x24,0x24,0x30,0x28,0x30,0x30,0x3C};
    static const unsigned size[8]={0x14,0x18,0x1C,0x24,0x20,0x28,0x28,0x34};
    static const unsigned vertex[8]={4,4,12,16,16,16,24,28};
    const pe_addr_t scratch=0x1F800000u,depth=0x1F8003FCu;
    pe_addr_t streams[8];
    (void)unused;
    uint32_t offset=PE_LoadU32(model+submodel*4u);
    pe_addr_t descriptor=PE_LoadU32(0x8019C9C0u);
    model+=offset;
    for(unsigned i=0;i<8;i++)streams[i]=model+PE_LoadU32(model+0x10u+i*4u);
    PE_StoreU32(scratch+0x18u,PE_LoadU32(0x801EA5E4u));
    pe_addr_t packet=PE_LoadU32(descriptor);
    /* Original SDK depth destination is an unused local stack word. */
    uint32_t saved_depth=PE_LoadU32(depth);
    for(unsigned kind=0;kind<8;kind++) {
        pe_addr_t src=streams[kind];
        for(uint32_t i=0;i<PE_LoadU16(model+kind*2u);i++,src+=stride[kind]) {
            pe_addr_t p=src+vertex[kind];
            int32_t clip;
            if(kind==4 || kind==5) {
                uint32_t xy[3],z[3];
                fixed_model_vertex(g_pe_gte.v0,p);fixed_model_vertex(g_pe_gte.v1,p+8u);fixed_model_vertex(g_pe_gte.v2,p+16u);
                PE_GTE_RTPT_coordinates(xy,z);
                PE_StoreU32(scratch+0x20u,(uint32_t)PE_GTE_NCLIP());
                clip=(int32_t)PE_LoadU32(scratch+0x20u);
                if(clip>0 && kind==5) {
                    PE_GTE_AVSZ3(z);
                    PE_StoreU32(packet+8u,g_pe_gte.sxy[0]);
                    PE_StoreU32(packet+16u,g_pe_gte.sxy[1]);
                    PE_StoreU32(packet+24u,g_pe_gte.sxy[2]);
                    fixed_model_vertex(g_pe_gte.v0,src+0x28u);
                    PE_GTE_RTPS_coordinates(xy,z);
                    PE_StoreU32(packet+32u,g_pe_gte.sxy[2]);
                }
            } else if(kind==1 || kind==3 || kind==7) {
                clip=func_80079414(p,p+8u,p+16u,p+24u,scratch,scratch+4u,scratch+8u,scratch+12u,scratch+16u,depth,scratch+20u);
            } else {
                clip=func_80079384(p,p+8u,p+16u,scratch,scratch+4u,scratch+8u,scratch+16u,depth,scratch+20u);
            }
            if(clip<=0)continue;
            PE_StoreU8(packet+3u,(uint8_t)(size[kind]/4u-1u));
            switch(kind) {
            case 0:case 1:
                for(unsigned j=0;j<(kind==0?3u:4u);j++)fixed_model_copy(packet+8u+j*4u,scratch+j*4u);
                fixed_model_copy(packet+4u,src);
                break;
            case 2:case 3:
                for(unsigned j=0;j<(kind==2?3u:4u);j++)fixed_model_copy(packet+8u+j*8u,scratch+j*4u);
                for(unsigned j=0;j<(kind==2?3u:4u);j++)fixed_model_copy(packet+4u+j*8u,src+j*4u);
                break;
            case 4:
                /* Original intentionally reads retained scratch, not current SXY. */
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+8u+j*8u,scratch+j*4u);
                fixed_model_copy(packet+4u,src);
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+12u+j*8u,src+4u+j*4u);
                break;
            case 5:
                fixed_model_copy(packet+4u,src);
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+12u+j*8u,src+4u+j*4u);
                PE_StoreU16(packet+36u,PE_LoadU16(src+14u));
                break;
            case 6:
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+8u+j*12u,scratch+j*4u);
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+12u+j*12u,src+12u+j*4u);
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+4u+j*12u,src+j*4u);
                break;
            case 7:
                for(unsigned j=0;j<4;j++)fixed_model_copy(packet+8u+j*12u,scratch+j*4u);
                for(unsigned j=0;j<4;j++)fixed_model_copy(packet+4u+j*12u,src+j*4u);
                for(unsigned j=0;j<3;j++)fixed_model_copy(packet+12u+j*12u,src+16u+j*4u);
                PE_StoreU16(packet+48u,PE_LoadU16(src+26u));
                break;
            }
            fixed_model_link(packet);
            packet+=size[kind];
        }
    }
    PE_StoreU32(PE_LoadU32(0x8019C9C0u),packet);
    PE_StoreU32(depth,saved_depth);
}
