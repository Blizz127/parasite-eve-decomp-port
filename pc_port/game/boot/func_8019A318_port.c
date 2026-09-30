/* Original transition color-adjusted model emitter, 8019A318..8019B1D0.
 * 942 words, SHA256 cb887e9575fa14d4044f7d590846e5f0d5c29d1580889af040cb5262a584a2d4.
 * Stream-specific rejection and scratch history are deliberately distinct.
 */
#include "pe_port_compat.h"
#include "pe_sdk.h"

static void color_model_vertex(int16_t v[3], pe_addr_t p)
{
    uint32_t xy=PE_LoadU32(p),z=PE_LoadU32(p+4u);
    v[0]=(int16_t)xy;v[1]=(int16_t)(xy>>16);v[2]=(int16_t)z;
}

static void color_model_copy(pe_addr_t dst, pe_addr_t src)
{
    PE_StoreU32(dst,PE_LoadU32(src));
}

static void color_model_link(pe_addr_t packet)
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

void func_8019A318(pe_addr_t unused, pe_addr_t model, uint32_t submodel)
{
    static const unsigned stride[8]={0x1C,0x24,0x24,0x30,0x28,0x30,0x30,0x3C};
    static const unsigned size[8]={0x14,0x18,0x1C,0x24,0x20,0x28,0x28,0x34};
    static const unsigned vertex[8]={4,4,12,16,16,16,24,28};
    const pe_addr_t scratch=0x1F800000u;
    pe_addr_t streams[8];
    (void)unused;
    uint32_t offset=PE_LoadU32(model+submodel*4u);
    pe_addr_t descriptor=PE_LoadU32(0x8019C9C0u);
    model+=offset;
    pe_addr_t packet=PE_LoadU32(descriptor);
    for(unsigned i=0;i<8;i++)streams[i]=model+PE_LoadU32(model+0x10u+i*4u);
    for(unsigned kind=0;kind<8;kind++) {
        pe_addr_t src=streams[kind];
        for(uint32_t i=0;i<PE_LoadU16(model+kind*2u);i++,src+=stride[kind]) {
            pe_addr_t p=src+vertex[kind];
            int32_t clip;
            if(kind<4) {
                if(kind==1 || kind==3)
                    clip=func_80079414(p,p+8u,p+16u,p+24u,scratch,scratch+4u,scratch+8u,scratch+12u,scratch+16u,scratch+24u,scratch+20u);
                else
                    clip=func_80079384(p,p+8u,p+16u,scratch,scratch+4u,scratch+8u,scratch+16u,scratch+24u,scratch+20u);
                /* Original adds bias even when the SDK rejected winding and
                 * left the preceding depth word in scratch. Only G4 checks
                 * the unshifted depth; the first three streams shift first. */
                uint32_t sum=PE_LoadU32(scratch+24u)+PE_LoadU32(0x801EA5E4u);
                if(kind!=3)sum=(uint32_t)((int32_t)sum>>2);
                PE_StoreU32(scratch+24u,sum);
                if(clip<=0 || sum-1u>=0xFFFu)continue;
                if(kind==3)PE_StoreU32(scratch+24u,(uint32_t)((int32_t)sum>>2));
                PE_StoreU8(packet+3u,(uint8_t)(size[kind]/4u-1u));
                unsigned step=kind>=2?8u:4u;
                for(unsigned j=0;j<((kind==1 || kind==3)?4u:3u);j++)color_model_copy(packet+8u+j*step,scratch+j*4u);
            } else {
                uint32_t xy[3],z[3];
                color_model_vertex(g_pe_gte.v0,p);color_model_vertex(g_pe_gte.v1,p+8u);color_model_vertex(g_pe_gte.v2,p+16u);
                PE_GTE_RTPT_coordinates(xy,z);
                PE_StoreU32(scratch+32u,(uint32_t)PE_GTE_NCLIP());
                clip=(int32_t)PE_LoadU32(scratch+32u);
                if(clip<=0)continue;
                PE_GTE_AVSZ3(z);
                PE_StoreU32(scratch+24u,g_pe_gte.otz);
                uint32_t sum=PE_LoadU32(scratch+24u)+PE_LoadU32(0x801EA5E4u);
                sum=(uint32_t)((int32_t)sum>>2);
                PE_StoreU32(scratch+24u,sum);
                if((int32_t)sum<=0)continue;
                unsigned step=kind>=6?12u:8u;
                for(unsigned j=0;j<3;j++)PE_StoreU32(packet+8u+j*step,g_pe_gte.sxy[j]);
                if(kind==5 || kind==7) {
                    color_model_vertex(g_pe_gte.v0,p+24u);
                    PE_GTE_RTPS_coordinates(xy,z);
                    PE_StoreU32(packet+8u+3u*step,g_pe_gte.sxy[2]);
                }
                PE_StoreU8(packet+3u,(uint8_t)(size[kind]/4u-1u));
            }
            switch(kind) {
            case 0:case 1:
                color_model_copy(packet+4u,src);break;
            case 2:case 3:
                /* Only the first command/color word is copied. Secondary
                 * color padding bytes retain their previous packet contents. */
                color_model_copy(packet+4u,src);
                for(unsigned j=0;j<(kind==2?3u:4u);j++)
                    for(unsigned channel=0;channel<3;channel++)
                        PE_StoreU8(packet+4u+j*8u+channel,PE_LoadU8(src+j*4u+channel)>>2);
                break;
            case 4:case 5:
                color_model_copy(packet+4u,src);
                for(unsigned channel=0;channel<3;channel++)
                    PE_StoreU8(packet+4u+channel,PE_LoadU8(0x801EA264u+channel));
                for(unsigned j=0;j<3;j++)color_model_copy(packet+12u+j*8u,src+4u+j*4u);
                if(kind==5)PE_StoreU16(packet+36u,PE_LoadU16(src+14u));
                break;
            case 6:case 7: {
                color_model_copy(packet+4u,src);
                for(unsigned channel=0;channel<3;channel++)
                    PE_StoreU8(packet+4u+channel,PE_LoadU8(0x801EA264u+channel));
                /* The original reads the uniform word once after first RGB. */
                uint32_t color=PE_LoadU32(0x801EA264u);
                for(unsigned j=1;j<(kind==6?3u:4u);j++)PE_StoreU32(packet+4u+j*12u,color);
                for(unsigned j=0;j<3;j++)color_model_copy(packet+12u+j*12u,src+(kind==6?12u:16u)+j*4u);
                if(kind==7)PE_StoreU16(packet+48u,PE_LoadU16(src+26u));
                break;
            }
            }
            color_model_link(packet);
            packet+=size[kind];
        }
    }
    PE_StoreU32(PE_LoadU32(0x8019C9C0u),packet);
}
