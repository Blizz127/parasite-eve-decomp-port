/* Original pistol flash/casing callbacks and shared textured-quad renderer.
 * BA420.s, BA6A8.s, B3390.s and BF0F0.s. Stack matrices use host values;
 * original persistent scratchpad/packet writes remain guest-addressed. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include <stdio.h>

static void quad_page(void)
{
    PE_StoreU16(0x800E27ACu,(uint16_t)func_80077A64(PE_LoadU8(0x800F33ACu),
        PE_LoadU8(0x800E224Cu),PE_LoadU16(0x800F3424u),PE_LoadU16(0x800F3426u)));
}

/* func_800C2EAC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C2EAC_port.c (src/func_800C2EAC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800C3098(int32_t colors)
{
    if ((int16_t)colors==16) PE_StoreU8(0x800F33ACu,0u);
    else if ((int16_t)colors==256) PE_StoreU8(0x800F33ACu,1u);
    else {
        pe_addr_t p=0x800C2110u;uint8_t ch;
        while ((ch=PE_LoadU8(p++))!=0u) fputc(ch,stderr);
    }
    quad_page();
}

/* func_800C2FF0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C2FF0_port.c (src/func_800C2FF0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_800C3238: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C3238_port.c (src/func_800C3238.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* func_800C608C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800C608C_port.c (src/func_800C608C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void quad_load_matrix(const PeEffectMatrix *m)
{
    unsigned i;
    for (i=0;i<9;i++) g_pe_gte.rt[i/3u][i%3u]=m->r[i];
    for (i=0;i<3;i++) g_pe_gte.tr[i]=m->t[i];
}

static void quad_vertices(void)
{
    PE_GTE_SetV0((int16_t)PE_LoadU16(0x800F3310u),(int16_t)PE_LoadU16(0x800F3312u),(int16_t)PE_LoadU16(0x800F3314u));
    PE_GTE_SetV1((int16_t)PE_LoadU16(0x800F3318u),(int16_t)PE_LoadU16(0x800F331Au),(int16_t)PE_LoadU16(0x800F331Cu));
    PE_GTE_SetV2((int16_t)PE_LoadU16(0x800F3320u),(int16_t)PE_LoadU16(0x800F3322u),(int16_t)PE_LoadU16(0x800F3324u));
}

void PE_EffectQuadC42A4(pe_addr_t style,PeEffectMatrix *matrix,unsigned billboard)
{
    unsigned i,j,bank=PE_LoadU32(0x8009CDDCu);uint32_t xy[3],z[3],depth;
    pe_addr_t packet=PE_LoadU32(0x800B0E58u+bank*4u)+PE_LoadU32(0x8009CDD8u);
    pe_addr_t camera=PE_LoadU32(0x800BCFA4u);
    uint8_t u,v,cx,cy,uv[4][2];
    PE_StoreU32(0x800F33B4u,0x1F800000u);PE_GTE_LoadRT(camera);
    if (!(billboard&255u)) {
        for (i=0;i<3;i++) {
            PE_GTE_SetIR(matrix->r[i],matrix->r[3u+i],matrix->r[6u+i]);PE_GTE_MVMVA(0x9E012u);
            for (j=0;j<3;j++) PE_StoreU16(0x1F80001Cu+j*6u+i*2u,(uint16_t)g_pe_gte.ir[j]);
        }
        PE_GTE_SetV0((int16_t)matrix->t[0],(int16_t)matrix->t[1],(int16_t)matrix->t[2]);
        PE_GTE_MVMVA(0x80012u);
        for (i=0;i<3;i++) PE_StoreU32(0x1F800030u+i*4u,(uint32_t)g_pe_gte.ir[i]);
        PE_GTE_LoadRT(0x1F80001Cu);
    } else {
        PE_GTE_SetV0((int16_t)matrix->t[0],(int16_t)matrix->t[1],(int16_t)matrix->t[2]);
        PE_GTE_MVMVA(0x80012u);
        for (i=0;i<3;i++) matrix->t[i]=g_pe_gte.mac[i];
        quad_load_matrix(matrix);
    }
    func_800C608C((int16_t)PE_LoadU16(style+10u),style,packet+4u);
    quad_vertices();PE_GTE_RTPT_coordinates(xy,z);
    v=(uint8_t)((PE_LoadU8(style+4u)&0xF0u)+PE_LoadU8(0x800F3422u));
    u=(uint8_t)((uint32_t)(PE_LoadU8(style+4u)-v)<<4u);
    cx=(uint8_t)(PE_LoadU8(style+5u)<<4u);cy=PE_LoadU8(style+5u)>>4u;
    PE_StoreU8(0x1F800010u,u);PE_StoreU8(0x1F800011u,v);
    PE_StoreU8(0x1F800012u,cx);PE_StoreU8(0x1F800013u,cy);
    PE_StoreU8(packet+3u,9u);PE_StoreU8(packet+7u,PE_LoadU8(0x800F337Au)?0x2Eu:0x2Cu);
    for (i=0;i<4;i++) {
        unsigned index=i^(PE_LoadU8(style+6u)&3u);
        uv[i][0]=(uint8_t)(u+((index&1u)?PE_LoadU8(0x800F345Cu):0u));
        uv[i][1]=(uint8_t)(v+((index&2u)?PE_LoadU8(0x800F345Du):0u));
        PE_StoreU8(packet+12u+i*8u,uv[i][0]);PE_StoreU8(packet+13u+i*8u,uv[i][1]);
    }
    PE_GTE_AVSZ3(z);depth=g_pe_gte.otz+(uint32_t)(int32_t)(int16_t)PE_LoadU16(style+8u);
    PE_StoreU32(0x1F80000Cu,depth);
    if (depth-1u>=4095u) return;
    for (i=0;i<3;i++) PE_StoreU32(packet+8u+i*8u,xy[i]);
    PE_GTE_SetV0((int16_t)PE_LoadU16(0x800F3328u),(int16_t)PE_LoadU16(0x800F332Au),(int16_t)PE_LoadU16(0x800F332Cu));
    PE_GTE_RTPS_coordinates(&xy[0],&z[0]);PE_StoreU32(packet+32u,xy[0]);
    PE_StoreU16(packet+22u,PE_LoadU16(0x800E27ACu));
    PE_StoreU16(packet+14u,(uint16_t)func_80077AA4(PE_LoadU16(0x800F341Cu)+cx,PE_LoadU16(0x800F341Eu)+cy));
    {
        pe_addr_t ot=PE_LoadU32(0x800B0E38u+bank*4u)+depth*4u;
        PE_StoreU32(packet,(PE_LoadU32(packet)&0xFF000000u)|(PE_LoadU32(ot)&0xFFFFFFu));
        PE_StoreU32(ot,(PE_LoadU32(ot)&0xFF000000u)|(packet&0xFFFFFFu));
        PE_StoreU32(0x8009CDD8u,PE_LoadU32(0x8009CDD8u)+40u);
    }
}

void func_800C42A4(pe_addr_t style,pe_addr_t matrix,unsigned billboard)
{
    PeEffectMatrix m;unsigned i;PE_EffectReadMatrix(matrix,&m);
    PE_EffectQuadC42A4(style,&m,billboard);
    if (billboard&255u) for (i=0;i<3;i++) PE_StoreU32(matrix+20u+i*4u,(uint32_t)m.t[i]);
}

/* func_800CEDA8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800CEDA8_port.c (src/func_800CEDA8.c); hand port retired (port3 switch-over N). */

static void pistol_rotate(pe_addr_t matrix,const int16_t in[3],int16_t out[3])
{
    unsigned i;PE_GTE_LoadRT33(matrix);PE_GTE_SetV0(in[0],in[1],in[2]);PE_GTE_MVMVA(0x86012u);
    for (i=0;i<3;i++) out[i]=(int16_t)g_pe_gte.ir[i];
}

void func_800C9C20(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    pe_addr_t actor=PE_LoadU32(slot+8u),matrix=PE_LoadU32(actor+568u);unsigned i;
    (void)rec;(void)data;
    for (i=0;i<3;i++) PE_StoreU16(0x800E2358u+i*2u,(uint16_t)PE_LoadU32(matrix+628u+i*4u));
    PE_StoreU32(0x800E27A4u,actor);func_800CEDA8(0);
}

void func_800C9C8C(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    int16_t in[3],out[3];unsigned i;(void)slot;(void)rec;
    in[0]=(int16_t)-(9+(int)(func_80071A54()%3u));
    in[1]=(int16_t)-(9+(int)(func_80071A54()%3u));in[2]=(int16_t)((int)(func_80071A54()%5u)-2);
    pistol_rotate(PE_LoadU32(PE_LoadU32(0x800E27A4u)+568u),in,out);
    for (i=0;i<3;i++) {
        PE_StoreU16(data+16u+i*2u,(uint16_t)out[i]);
        PE_StoreU16(data+8u+i*2u,PE_LoadU16(0x800E2358u+i*2u));
    }
    PE_StoreU8(data+2u,20u);PE_StoreU8(data+1u,0u);
}

void func_800C9D9C(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    pe_addr_t actor=PE_LoadU32(0x800E27A4u),matrix=PE_LoadU32(actor+568u)+608u;
    pe_addr_t weapon=PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u))+104u);
    pe_addr_t offset=0x800E0AF8u+(uint32_t)((int32_t)(int16_t)(PE_LoadU16(weapon+6u)-1u)*8);
    int16_t in[3],out[3];unsigned i;(void)slot;(void)rec;
    for (i=0;i<3;i++) in[i]=(int16_t)PE_LoadU16(offset+i*2u);
    pistol_rotate(matrix,in,out);
    for (i=0;i<3;i++) PE_StoreU16(data+8u+i*2u,(uint16_t)(PE_LoadU16(0x800E2358u+i*2u)+(uint16_t)out[i]));
    for (i=0;i<8;i++) PE_StoreU32(data+16u+i*4u,PE_LoadU32(matrix+i*4u));
    PE_StoreU16(data+4u,127u);
}

static void pistol_scale(PeEffectMatrix *m,const int32_t scale[3])
{
    unsigned i;
    for (i=0;i<9;i++) m->r[i]=(int16_t)((int32_t)((uint32_t)(int32_t)m->r[i]*(uint32_t)scale[i%3u])>>12);
}

void func_800C9EA8(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    PeEffectMatrix m={{0},0,{0}};int16_t angles[3]={0,0,0};int32_t scale[3];unsigned i;
    pe_addr_t weapon=PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u))+104u);
    int32_t kind=(int16_t)(PE_LoadU16(weapon+6u)-1u);(void)slot;(void)rec;
    angles[2]=(int16_t)((int32_t)(int8_t)PE_LoadU8(data+1u)*64);
    func_800C2EAC(3u);func_800C3098(16);func_800C2FF0(16u,16u);func_800C3238(0u);
    PE_RotMatrix794C4_values(angles,m.r);
    for (i=0;i<3;i++) {m.t[i]=(int16_t)PE_LoadU16(data+8u+i*2u);scale[i]=(int16_t)PE_LoadU16(0x800E0AD8u+(uint32_t)kind*2u);}
    pistol_scale(&m,scale);PE_EffectQuadC42A4(0x800E22F8u,&m,1u);
}

void func_800C9FD8(pe_addr_t slot,pe_addr_t rec,pe_addr_t data)
{
    int32_t scale[3];unsigned plane,i,col,row;(void)slot;(void)rec;
    func_800C2EAC(3u);func_800C3098(16);func_800C2FF0(32u,16u);func_800C3238(2u);
    PE_StoreU16(0x800F34C2u,PE_LoadU16(data+4u));
    for (i=0;i<3;i++) scale[i]=(int32_t)PE_LoadU32(0x800C21B4u+i*4u);
    for (plane=0;plane<2;plane++) {
        PeEffectMatrix m;int16_t angles[3],rotation[9];
        for (i=0;i<3;i++) angles[i]=(int16_t)PE_LoadU16(0x800C21A4u+plane*8u+i*2u);
        PE_RotMatrix79754_values(angles,rotation);PE_EffectReadMatrix(data+16u,&m);
        for (i=0;i<3;i++) m.t[i]=(int16_t)PE_LoadU16(data+8u+i*2u);
        quad_load_matrix(&m);
        for (col=0;col<3;col++) {
            PE_GTE_SetIR(rotation[col],rotation[3u+col],rotation[6u+col]);PE_GTE_MVMVA(0x9E012u);
            for (row=0;row<3;row++) m.r[row*3u+col]=(int16_t)g_pe_gte.ir[row];
        }
        PE_GTE_SetV0(0,0,0);PE_GTE_MVMVA(0x80012u);
        for (i=0;i<3;i++) m.t[i]=g_pe_gte.ir[i];
        pistol_scale(&m,scale);PE_EffectQuadC42A4(0x800F34B8u,&m,0u);
    }
}

/* func_800C4FC4 (B57C4.s, 0x574; handwritten GTE wrapper, no matched C):
 * ribbon/ring glow renderer.  r (style block): +0 SVECTOR ring array
 * (2n vertices: ring 0 = [0,n), ring 1 = [n,2n)), +4/+8 colour ramps,
 * +0xC u16 n, +0x12 s16 OT bias, +0x14 s16 ramp time.  billboard == 0:
 * scratch RT/TR = camera (D_800BCFA4) x matrix; else the matrix translation
 * is RotTrans'd through the camera in place (matrix+0x14) and the matrix is
 * loaded as-is.  Each segment emits a POLY_G4 (code 0x38, 0x3A when
 * D_800F337A) through RotTransPers4 (func_80079304: RTPT v0..v2 + RTPS v3,
 * otz = SZ3 >> 2) plus a draw-mode word from func_80077C84, both linked at
 * OT[otz + bias]; the packet cursor advances 0x2C per segment. */
void func_800C4FC4(pe_addr_t r,pe_addr_t matrix,unsigned billboard)
{
    unsigned i,n,bank;uint32_t c0,c1;pe_addr_t verts;
    pe_addr_t camera=PE_LoadU32(0x800BCFA4u);
    PE_StoreU32(0x800F33B4u,0x1F800000u);
    if (!(billboard&255u)) {
        PE_GTE_LoadRT33(camera);
        for (i=0;i<3;i++) {
            PE_GTE_SetIR((int16_t)PE_LoadU16(matrix+i*2u),(int16_t)PE_LoadU16(matrix+6u+i*2u),
                (int16_t)PE_LoadU16(matrix+12u+i*2u));
            PE_GTE_MVMVA(0x9E012u);
            for (unsigned j=0;j<3;j++) PE_StoreU16(0x1F80001Cu+i*2u+j*6u,(uint16_t)g_pe_gte.ir[j]);
        }
        PE_GTE_LoadRT(camera);
        PE_GTE_SetV0((int16_t)PE_LoadU16(matrix+20u),(int16_t)PE_LoadU16(matrix+24u),(int16_t)PE_LoadU16(matrix+28u));
        PE_GTE_MVMVA(0x80012u);
        for (i=0;i<3;i++) PE_StoreU32(0x1F800030u+i*4u,(uint32_t)g_pe_gte.ir[i]);
        PE_GTE_LoadRT(0x1F80001Cu);
    } else {
        PE_GTE_LoadRT(camera);
        PE_GTE_SetV0((int16_t)PE_LoadU32(matrix+20u),(int16_t)PE_LoadU32(matrix+24u),(int16_t)PE_LoadU32(matrix+28u));
        PE_GTE_MVMVA(0x80012u);   /* func_800792D4 RotTrans -> matrix+0x14 (MAC) */
        for (i=0;i<3;i++) PE_StoreU32(matrix+20u+i*4u,(uint32_t)g_pe_gte.mac[i]);
        PE_GTE_LoadRT(matrix);
    }
    PE_StoreU32(0x1F800040u,0u);   /* host stand-in for the 3-byte stack colours */
    func_800C608C((int16_t)PE_LoadU16(r+0x14u),r+8u,0x1F800040u);   /* sp+0x28 */
    c0=PE_LoadU32(0x1F800040u);
    func_800C608C((int16_t)PE_LoadU16(r+0x14u),r+4u,0x1F800040u);   /* sp+0x30 */
    c1=PE_LoadU32(0x1F800040u);
    n=PE_LoadU16(r+0xCu);verts=PE_LoadU32(r);
    for (i=0;i<n;i++) {
        pe_addr_t pk,dm,ot;uint32_t off,xy[3],z[3],xy3,z3,depth;unsigned k1,j;
        bank=PE_LoadU32(0x8009CDDCu);off=PE_LoadU32(0x8009CDD8u);
        PE_StoreU32(0x8009CDD8u,off+0x2Cu);
        pk=PE_LoadU32(0x800B0E58u+bank*4u)+off;dm=pk+0x24u;
        (void)func_80077C84(dm,0u,0u,(PE_LoadU8(0x800E224Cu)&3u)<<5);
        k1=(i+1u)%n;
        {
            const pe_addr_t v[4]={verts+(i&0xFFFFu)*8u,verts+k1*8u,
                verts+((n+i)&0xFFFFu)*8u,verts+((n+k1)&0xFFFFu)*8u};
            PE_GTE_SetV0((int16_t)PE_LoadU16(v[0]),(int16_t)PE_LoadU16(v[0]+2u),(int16_t)PE_LoadU16(v[0]+4u));
            PE_GTE_SetV1((int16_t)PE_LoadU16(v[1]),(int16_t)PE_LoadU16(v[1]+2u),(int16_t)PE_LoadU16(v[1]+4u));
            PE_GTE_SetV2((int16_t)PE_LoadU16(v[2]),(int16_t)PE_LoadU16(v[2]+2u),(int16_t)PE_LoadU16(v[2]+4u));
            PE_GTE_RTPT_coordinates(xy,z);
            PE_GTE_SetV0((int16_t)PE_LoadU16(v[3]),(int16_t)PE_LoadU16(v[3]+2u),(int16_t)PE_LoadU16(v[3]+4u));
            PE_GTE_RTPS_coordinates(&xy3,&z3);
        }
        for (j=0;j<3;j++) PE_StoreU32(pk+8u+j*8u,xy[j]);
        PE_StoreU32(pk+0x20u,xy3);
        PE_StoreU32(0x1F80000Cu,(uint32_t)((int32_t)z3>>2));   /* RotTransPers4 return */
        PE_StoreU32(pk+4u,c0);PE_StoreU32(pk+0xCu,c0);
        PE_StoreU8(pk+3u,8u);PE_StoreU8(pk+7u,0x38u);
        PE_StoreU32(pk+0x14u,c1);PE_StoreU32(pk+0x1Cu,c1);
        PE_StoreU8(pk+7u,PE_LoadU8(0x800F337Au)?0x3Au:0x38u);
        depth=PE_LoadU32(0x1F80000Cu)+(uint32_t)(int32_t)(int16_t)PE_LoadU16(r+0x12u);
        ot=PE_LoadU32(0x800B0E38u+bank*4u)+depth*4u;
        PE_StoreU32(pk,(PE_LoadU32(pk)&0xFF000000u)|(PE_LoadU32(ot)&0xFFFFFFu));
        PE_StoreU32(ot,(PE_LoadU32(ot)&0xFF000000u)|(pk&0xFFFFFFu));
        PE_StoreU32(dm,(PE_LoadU32(dm)&0xFF000000u)|(PE_LoadU32(ot)&0xFFFFFFu));
        PE_StoreU32(ot,(PE_LoadU32(ot)&0xFF000000u)|(dm&0xFFFFFFu));
    }
}
