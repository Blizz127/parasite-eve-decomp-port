/* Original M0000I 8018F344..8018F55C and SDK 78134/78194..78254.
 * Build the camera basis with the original table-normalized GTE products.
 * Do not replace the integer normalizer with floating-point unit vectors. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

int PE_NormalizeVectorRetail(const int32_t in[3],int32_t out[3],uint32_t *length)
{
    uint32_t sum=0,leading,index;
    int shift;
    for(unsigned i=0;i<3;i++) {
        int32_t v=(int16_t)in[i];
        g_pe_gte.mac[i]=v*v;
        g_pe_gte.ir[i]=v*v>32767?32767:v*v;
    }
    /* The original uses trapping ADD for both additions after SQR. */
    for(unsigned i=0;i<3;i++) {
        if((uint64_t)sum+(uint32_t)g_pe_gte.mac[i]>INT32_MAX) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
        }
        sum+=(uint32_t)g_pe_gte.mac[i];
    }
    leading=PE_GTE_LZCR(sum)&~1u;
    shift=(int)leading-24;
    index=shift>=0?sum<<((unsigned)shift&31u):(uint32_t)((int32_t)sum>>((24u-leading)&31u));
    g_pe_gte.ir0=(int16_t)PE_LoadU16(0x80096250u+((index-64u)<<1u));
    PE_GTE_SetIR((int16_t)in[0],(int16_t)in[1],(int16_t)in[2]);
    PE_GTE_GPF(0,0);
    shift=((31-(int)leading)>>1)&31;
    for(unsigned i=0;i<3;i++)out[i]=g_pe_gte.mac[i]>>shift;
    *length=sum;return 1;
}

/* func_80078134: now pe_plat_gte_vn.c (clean-room, docs/evidence/native-gte-vn-20261031) */

static void cross(const int32_t diagonal[3],const int32_t vector[3],int32_t out[3])
{
    /* CTC2 writes whole control words 0,2,4, including off-diagonals. */
    g_pe_gte.rt[0][0]=(int16_t)diagonal[0];
    g_pe_gte.rt[0][1]=(int16_t)((uint32_t)diagonal[0]>>16);
    g_pe_gte.rt[1][1]=(int16_t)diagonal[1];
    g_pe_gte.rt[1][2]=(int16_t)((uint32_t)diagonal[1]>>16);
    g_pe_gte.rt[2][2]=(int16_t)diagonal[2];
    PE_GTE_SetIR((int16_t)vector[0],(int16_t)vector[1],(int16_t)vector[2]);
    PE_GTE_OP(1,0);
    for(unsigned i=0;i<3;i++)out[i]=g_pe_gte.mac[i];
}

void func_8018F344(pe_addr_t matrix,pe_addr_t eye,pe_addr_t target,pe_addr_t up)
{
    int32_t delta[3],forward[3],vertical[3],product[3],right[3],down[3];
    uint32_t length;
    for(unsigned i=0;i<3;i++)delta[i]=(int16_t)PE_LoadU16(target+i*2u)-(int16_t)PE_LoadU16(eye+i*2u);
    if(!PE_NormalizeVectorRetail(delta,forward,&length))return;
    for(unsigned i=0;i<3;i++)vertical[i]=(int32_t)PE_LoadU32(up+i*4u);
    if(forward[2]==vertical[2])forward[2]=(int32_t)((uint32_t)forward[2]+1u);
    cross(forward,vertical,product);
    if(!PE_NormalizeVectorRetail(product,right,&length))return;
    cross(forward,right,product);
    if(!PE_NormalizeVectorRetail(product,down,&length))return;
    for(unsigned i=0;i<3;i++)PE_StoreU16(matrix+i*2u,(uint16_t)right[i]);
    for(unsigned i=0;i<3;i++)PE_StoreU16(matrix+6u+i*2u,(uint16_t)down[i]);
    for(unsigned i=0;i<3;i++)PE_StoreU16(matrix+12u+i*2u,(uint16_t)forward[i]);
    PE_GTE_LoadRT33(matrix);
    PE_GTE_SetV0((int16_t)PE_LoadU16(eye),(int16_t)PE_LoadU16(eye+2u),(int16_t)PE_LoadU16(eye+4u));
    PE_GTE_MVMVA(0x86000u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(matrix+20u+i*4u,(uint32_t)g_pe_gte.mac[i]);
    PE_StoreU32(matrix+20u,0u-PE_LoadU32(matrix+20u));
    PE_StoreU32(matrix+28u,0u-PE_LoadU32(matrix+28u));
    PE_StoreU32(matrix+24u,0u-PE_LoadU32(matrix+24u));
}
