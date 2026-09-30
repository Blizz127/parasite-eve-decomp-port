/* Complete original effect-mesh renderer and palette/UV operations.
 * B3390.s / B76F8.s, including 787D4 matrix concatenation in scratchpad. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

void func_800C6D5C(pe_addr_t mesh,uint32_t u,uint32_t v)
{
    pe_addr_t face=mesh+16u;unsigned group,i,j;
    if (PE_LoadU16(mesh+12u)!=(u&255u) || PE_LoadU16(mesh+14u)!=(v&255u)) {
        PE_StoreU16(mesh+12u,(uint16_t)(u&255u));PE_StoreU16(mesh+14u,(uint16_t)(v&255u));
        for (group=0;group<2;group++) {
            uint32_t count=PE_LoadU16(mesh+group*4u)+PE_LoadU16(mesh+group*4u+2u),vertices=group+3u;
            for (i=0;i<count;i++,face+=vertices*4u) for (j=0;j<vertices;j++) {
                pe_addr_t uv=face+vertices*2u+j*2u;
                PE_StoreU8(uv,(uint8_t)(PE_LoadU8(uv)+u));PE_StoreU8(uv+1u,(uint8_t)(PE_LoadU8(uv+1u)+v));
            }
        }
    }
    PE_StoreU16(0x800F3420u,0u);
}

/* func_800C6EC0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C6EC0_port.c (src/func_800C6EC0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_800C6ED8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C6ED8_port.c (src/func_800C6ED8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800C6EF8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C6EF8_port.c (src/func_800C6EF8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800C6F4C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C6F4C_port.c (src/func_800C6F4C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800C6FA0(pe_addr_t mesh,uint32_t level)
{
    pe_addr_t colors=mesh+PE_LoadU16(mesh+8u);unsigned i,j;
    level&=65535u;
    for (i=0;i<PE_LoadU16(mesh+10u);i++) for (j=0;j<3;j++) {
        uint32_t value=PE_LoadU8(colors+i*4u+j)*level;
        if (value>32767u) value=32767u;
        value>>=7u;PE_StoreU32(0x1F800030u,value);PE_StoreU8(colors+i*4u+j,(uint8_t)value);
    }
}

/* func_800C70EC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C70EC_port.c (src/func_800C70EC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void PE_EffectReadMatrix(pe_addr_t address,PeEffectMatrix *out)
{
    unsigned i;
    for (i=0;i<9;i++) out->r[i]=(int16_t)PE_LoadU16(address+i*2u);
    out->pad=(int16_t)PE_LoadU16(address+18u);
    for (i=0;i<3;i++) out->t[i]=(int32_t)PE_LoadU32(address+20u+i*4u);
}

static void effect_concat_matrix(const PeEffectMatrix *b)
{
    pe_addr_t camera=PE_LoadU32(0x800BCFA4u);int16_t rotation[9];unsigned col,row;
    PE_GTE_LoadRT33(camera);
    for (col=0;col<3;col++) {
        PE_GTE_SetV0(b->r[col],b->r[3u+col],b->r[6u+col]);PE_GTE_MVMVA(0x86000u);
        for (row=0;row<3;row++) rotation[row*3u+col]=(int16_t)g_pe_gte.ir[row];
    }
    for (row=0;row<8;row++) PE_StoreU16(0x1F800038u+row*2u,(uint16_t)rotation[row]);
    PE_StoreU32(0x1F800048u,(uint32_t)(int32_t)rotation[8]);
    PE_GTE_SetV0((int16_t)b->t[0],(int16_t)b->t[1],(int16_t)b->t[2]);PE_GTE_MVMVA(0x86000u);
    for (row=0;row<3;row++) PE_StoreU32(0x1F80004Cu+row*4u,(uint32_t)g_pe_gte.mac[row]+PE_LoadU32(camera+20u+row*4u));
    PE_GTE_LoadRT(0x1F800038u);
}

static int32_t effect_nclip(const uint32_t xy[3])
{
    int32_t x[3],y[3];int64_t value=0;unsigned i;
    for (i=0;i<3;i++) {x[i]=(int16_t)xy[i];y[i]=(int16_t)(xy[i]>>16u);}
    for (i=0;i<3;i++) value+=(int64_t)x[i]*(y[(i+1u)%3u]-y[(i+2u)%3u]);
    g_pe_gte.mac0=(int32_t)value;return (int32_t)value;
}

void PE_EffectMeshC71E4(pe_addr_t mesh,const PeEffectMatrix *matrix)
{
    pe_addr_t face=mesh+16u,packet;
    unsigned group,n,j;
    PE_StoreU16(0x1F800024u,PE_LoadU16(0x800F346Cu));
    PE_StoreU16(0x1F800026u,PE_LoadU16(0x800F3414u));
    PE_StoreU16(0x1F800034u,PE_LoadU16(0x800F33E4u));
    effect_concat_matrix(matrix);
    packet=PE_LoadU32(0x800B0E58u+PE_LoadU32(0x8009CDDCu)*4u)+PE_LoadU32(0x8009CDD8u);
    for (group=0;group<4;group++) {
        uint32_t count=PE_LoadU16(mesh+group*2u),vertices=group<2u?3u:4u;
        unsigned shaded=group&1u,stride=shaded?12u:8u;
        uint32_t bytes=shaded?vertices*12u+4u:vertices*8u+8u;
        uint8_t command=(uint8_t)(0x24u+(group>=2u?8u:0u)+(shaded?16u:0u));
        for (n=0;n<count;n++,face+=vertices*4u) {
            pe_addr_t points[4];uint32_t xy[3],z[3],depth;
            int32_t winding;
            for (j=0;j<vertices;j++) points[j]=mesh+PE_LoadU16(face+j*2u);
            PE_GTE_SetV0((int16_t)PE_LoadU16(points[0]),(int16_t)PE_LoadU16(points[0]+2u),(int16_t)PE_LoadU16(points[0]+4u));
            PE_GTE_SetV1((int16_t)PE_LoadU16(points[1]),(int16_t)PE_LoadU16(points[1]+2u),(int16_t)PE_LoadU16(points[1]+4u));
            PE_GTE_SetV2((int16_t)PE_LoadU16(points[2]),(int16_t)PE_LoadU16(points[2]+2u),(int16_t)PE_LoadU16(points[2]+4u));
            PE_GTE_RTPT_coordinates(xy,z);
            for (j=0;j<3;j++) PE_StoreU16(packet+12u+j*stride,PE_LoadU16(face+vertices*2u+j*2u));
            if (group==2u) PE_StoreU16(packet+36u,PE_LoadU16(face+14u));
            winding=effect_nclip(xy);PE_StoreU32(0x1F800020u,(uint32_t)winding);
            if (winding<=0) continue;
            PE_GTE_AVSZ3(z);
            if (group==3u) PE_StoreU16(packet+48u,PE_LoadU16(face+14u));
            depth=g_pe_gte.otz+(uint32_t)(int32_t)(int16_t)PE_LoadU16(0x800F3420u);
            PE_StoreU32(0x1F800018u,depth);
            if (group!=3u) {
                PE_StoreU16(packet+12u+stride+2u,PE_LoadU16(0x1F800024u));
                PE_StoreU16(packet+14u,PE_LoadU16(0x1F800026u));
            }
            if ((int32_t)depth<=0) continue;
            for (j=0;j<(shaded?vertices:1u);j++)
                PE_StoreU32(packet+4u+j*stride,PE_LoadU32(mesh+PE_LoadU16(points[j]+6u)));
            for (j=0;j<3;j++) PE_StoreU32(packet+8u+j*stride,xy[j]);
            if (vertices==4u) {
                uint32_t last_xy,last_z;
                PE_GTE_SetV0((int16_t)PE_LoadU16(points[3]),(int16_t)PE_LoadU16(points[3]+2u),(int16_t)PE_LoadU16(points[3]+4u));
                PE_GTE_RTPS_coordinates(&last_xy,&last_z);PE_StoreU32(packet+8u+3u*stride,last_xy);
            }
            if (group==3u) {
                PE_StoreU16(packet+26u,PE_LoadU16(0x1F800024u));
                PE_StoreU16(packet+14u,PE_LoadU16(0x1F800026u));
            }
            PE_StoreU8(packet+3u,(uint8_t)(bytes/4u-1u));
            PE_StoreU8(packet+7u,(uint8_t)(command|(PE_LoadU16(0x1F800034u)?2u:0u)));
            func_80077AC4(PE_LoadU32(0x800B0E38u+PE_LoadU32(0x8009CDDCu)*4u)+depth*4u,packet);
            PE_StoreU32(0x8009CDD8u,PE_LoadU32(0x8009CDD8u)+bytes);packet+=bytes;
        }
    }
}

void func_800C71E4(pe_addr_t mesh,pe_addr_t matrix)
{
    PeEffectMatrix value;PE_EffectReadMatrix(matrix,&value);PE_EffectMeshC71E4(mesh,&value);
}
