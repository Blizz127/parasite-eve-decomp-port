/* Original M0000I visibility tests8FFF4..90998.
 * Preserve wrapped plane arithmetic, strict/inclusive boundary differences,
 * signed16 expanded radius, and retail DIV/BREAK boundaries. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static const pe_addr_t plane_vectors[]={0x8019CBB0u,0x8019CBD0u,0x8019CBF0u,0x8019CB50u};
static const pe_addr_t plane_offsets[]={0x8019CB48u,0x8019CB4Cu,0x8019CBA8u,0x8019CA90u};
static const pe_addr_t plane_references[]={0x8019CC04u,0x8019CC0Cu,0x8019CC10u,0x8019CBC4u};
static const pe_addr_t plane_lengths[]={0x8019CBC8u,0x8019CC00u,0x8019CC08u,0x8019CBACu};

static int32_t plane_value(unsigned index,const uint32_t point[3])
{
    uint32_t value=PE_LoadU32(plane_offsets[index]);
    for(unsigned i=0;i<3;i++)value+=PE_LoadU32(plane_vectors[index]+i*4u)*point[i];
    return (int32_t)value;
}
static int point_test(pe_addr_t input,int shorts)
{
    uint32_t point[3];
    for(unsigned i=0;i<3;i++)point[i]=shorts?(uint32_t)(int32_t)(int16_t)PE_LoadU16(input+i*2u):PE_LoadU32(input+i*4u);
    int inside=1;
    for(unsigned i=0;i<2;i++) {
        int32_t value=plane_value(i,point),reference=(int32_t)PE_LoadU32(plane_references[i]);
        inside&=(value>0 && reference>0)||(value<0 && reference<0);
    }
    return inside;
}
int func_8018FFF4(pe_addr_t point){return point_test(point,0);}
int func_80190124(pe_addr_t point){return point_test(point,1);}

static int radius_test(pe_addr_t input,uint32_t radius,unsigned planes)
{
    uint32_t point[3];int32_t values[4];int sides[4];
    for(unsigned i=0;i<3;i++)point[i]=PE_LoadU32(input+i*4u);
    for(unsigned i=0;i<planes;i++) {
        values[i]=plane_value(i,point);
        int32_t reference=(int32_t)PE_LoadU32(plane_references[i]);
        sides[i]=(reference>0 && values[i]>=0)||(reference<0 && values[i]<=0);
    }
    int32_t limit=(int16_t)(radius*8u+30u);int result=1;
    /* The original evaluates every plane, even if a previous plane rejects
     * the point. A later division may therefore trap before the AND result. */
    for(unsigned i=0;i<planes;i++) {
        if(!sides[i]) {
            int32_t numerator=values[i]<0?(int32_t)(0u-(uint32_t)values[i]):values[i];
            int32_t denominator=(int32_t)PE_LoadU32(plane_lengths[i]);
            if(!denominator || (numerator==INT32_MIN && denominator==-1)) {
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
            }
            sides[i]=numerator/denominator<limit;
        }
        result&=sides[i];
    }
    return result;
}
int func_80190254(pe_addr_t point,uint32_t radius){return radius_test(point,radius,2);}
int func_801904B0(pe_addr_t point,uint32_t radius){return radius_test(point,radius,4);}
