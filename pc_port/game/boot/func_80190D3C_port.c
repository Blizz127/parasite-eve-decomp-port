/* Original object render wrappers 80190D3C..80191580 (529 words).
 * SHA256 d430a28dfa21a41f473027f8bc3c6247511b48a0440d938af53abe89cef742d7.
 * Matrix copies retain their two 16-byte load/store groups. Template and point
 * snapshots precede all mutations; visibility is called even for forced draws.
 */
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

static void object_matrix_copy(pe_addr_t dst, pe_addr_t src)
{
    for(unsigned block=0;block<2;block++) {
        uint32_t words[4];
        for(unsigned i=0;i<4;i++)words[i]=PE_LoadU32(src+block*16u+i*4u);
        for(unsigned i=0;i<4;i++)PE_StoreU32(dst+block*16u+i*4u,words[i]);
    }
}
static void object_matrix_submit(pe_addr_t matrix)
{
    object_matrix_copy(PE_LoadU32(0x8019BFF0u),matrix);
    pe_addr_t dest=PE_LoadU32(0x8019BFF0u);
    uint32_t epoch=PE_Port_StopEpoch();
    func_800787D4(0x8019CC30u,dest,dest);
    if(PE_Port_StopEpoch()!=epoch)return;
    func_80078E94(PE_LoadU32(0x8019BFF0u));
    func_80078E04(PE_LoadU32(0x8019BFF0u));
}
static int16_t object_matrix_half(const uint32_t m[8], unsigned i)
{
    return (int16_t)(m[i/2]>>((i%2)*16));
}
static void object_matrix_rt(const uint32_t m[8])
{
    for(unsigned i=0;i<9;i++)g_pe_gte.rt[i/3][i%3]=object_matrix_half(m,i);
}
static void object_matrix_translation(const uint32_t m[8])
{
    for(unsigned i=0;i<3;i++)g_pe_gte.tr[i]=(int32_t)m[5+i];
}
static void object_matrix_snapshot(uint32_t m[8], pe_addr_t p)
{
    for(unsigned i=0;i<8;i++)m[i]=PE_LoadU32(p+i*4u);
}
void func_80190D3C(pe_addr_t object, pe_addr_t context)
{
    uint32_t epoch=PE_Port_StopEpoch();
    func_800794C4(object+0x28u,object+8u);
    object_matrix_submit(object+8u);
    if(PE_Port_StopEpoch()!=epoch)return;
    func_80197BA0(context,PE_LoadU32(object+4u));
}
void func_80190E04(pe_addr_t object, pe_addr_t context, uint32_t fixed_depth,
                   uint32_t force_draw, uint32_t color_pass)
{
    uint32_t templ[8],saved[3],epoch=PE_Port_StopEpoch();
    const pe_addr_t point=0x1F800300u;
    object_matrix_snapshot(templ,0x8018F014u);
    for(unsigned i=0;i<3;i++){saved[i]=PE_LoadU32(point+i*4u);PE_StoreU32(point+i*4u,PE_LoadU32(object+0x1Cu+i*4u));}
    int visible=func_801904B0(point,(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+0x32u));
    if(PE_Port_StopEpoch()!=epoch || !(visible|(force_draw&255u)))goto done;
    func_800794C4(object+0x28u,object+8u);
    color_pass&=255u;
    if(color_pass==1u) {
        object_matrix_rt(templ);
        for(unsigned col=0;col<3;col++) {
            pe_addr_t p=object+8u+col*2u;
            PE_GTE_SetIR((int16_t)PE_LoadU16(p),(int16_t)PE_LoadU16(p+6u),(int16_t)PE_LoadU16(p+12u));
            PE_GTE_MVMVA(0x49E012u);
            for(unsigned row=0;row<3;row++)PE_StoreU16(p+row*6u,(uint16_t)g_pe_gte.ir[row]);
        }
        object_matrix_translation(templ);
        PE_GTE_SetV0((int16_t)PE_LoadU16(object+0x1Cu),(int16_t)PE_LoadU16(object+0x20u),(int16_t)PE_LoadU16(object+0x24u));
        PE_GTE_MVMVA(0x480012u);
        for(unsigned i=0;i<3;i++)PE_StoreU32(object+0x1Cu+i*4u,(uint32_t)g_pe_gte.ir[i]);
    }
    object_matrix_submit(object+8u);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    fixed_depth&=255u;
    if(!fixed_depth) {
        if(color_pass==0)func_801995BC(context,PE_LoadU32(object+4u),(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+2u));
        if(color_pass==1)func_8019A318(context,PE_LoadU32(object+4u),(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+2u));
    }
    if(fixed_depth==1)func_8019B1D0(context,PE_LoadU32(object+4u),(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+2u));
 done:
    for(unsigned i=0;i<3;i++)PE_StoreU32(point+i*4u,saved[i]);
}
void func_80191114(pe_addr_t object, pe_addr_t context, uint32_t force_draw, uint32_t mode)
{
    uint32_t templ[8],own[8],saved[5],epoch=PE_Port_StopEpoch();
    const pe_addr_t point=0x1F800300u,angles=point+12u;
    object_matrix_snapshot(templ,0x8018F014u);
    for(unsigned i=0;i<5;i++)saved[i]=PE_LoadU32(point+i*4u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(point+i*4u,PE_LoadU32(object+0x20u+i*4u));
    uint32_t radius=(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+0x36u);
    mode&=255u;
    if(mode==2u) {
        for(unsigned i=0;i<2;i++)PE_StoreU32(angles+i*4u,PE_LoadU32(object+0x2Cu+i*4u));
        func_800794C4(angles,object+12u);
        object_matrix_snapshot(own,object+12u);
        object_matrix_rt(own);
        for(unsigned col=0;col<3;col++) {
            PE_GTE_SetIR(object_matrix_half(templ,col),object_matrix_half(templ,col+3u),object_matrix_half(templ,col+6u));
            PE_GTE_MVMVA(0x49E012u);
            for(unsigned row=0;row<3;row++)PE_StoreU16(object+12u+col*2u+row*6u,(uint16_t)g_pe_gte.ir[row]);
        }
        object_matrix_translation(own);
        PE_GTE_SetV0((int16_t)templ[5],(int16_t)templ[6],(int16_t)templ[7]);
        PE_GTE_MVMVA(0x480012u);
        for(unsigned i=0;i<3;i++)PE_StoreU32(object+0x20u+i*4u,(uint32_t)g_pe_gte.ir[i]);
        for(unsigned i=0;i<3;i++)PE_StoreU32(object+0x20u+i*4u,own[5+i]);
    } else func_800794C4(object+0x2Cu,object+12u);
    object_matrix_submit(object+12u);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    int visible=func_80190254(point,radius);
    if(PE_Port_StopEpoch()!=epoch || !(visible|(force_draw&255u)))goto done;
    int32_t depth=(int32_t)PE_LoadU32(PE_LoadU32(0x8019BFF0u)+28u);
    unsigned index_offset;
    if(depth<(int32_t)PE_LoadU32(object+0x3Cu)) {
        if(depth<385)goto done;
        index_offset=6;
    } else if(depth<(int32_t)PE_LoadU32(object+0x38u)) {
        if(mode==2)goto done;
        index_offset=4;
    } else index_offset=2;
    if(mode==0)func_801995BC(context,PE_LoadU32(object+8u),(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+index_offset));
    if(mode==1 || mode==2)func_8019A318(context,PE_LoadU32(object+8u),(uint32_t)(int32_t)(int16_t)PE_LoadU16(object+index_offset));
 done:
    for(unsigned i=0;i<5;i++)PE_StoreU32(point+i*4u,saved[i]);
}
