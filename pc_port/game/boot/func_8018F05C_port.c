/* Complete original transition camera setup 8018F05C..8018F344.
 * Includes the SDK matrix concatenation and alternate Euler rotation leaves. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

/* func_800787D4: now pe_plat_gte_cmd.c (clean-room, docs/evidence/native-gte-cmd-20261031) */

static int32_t mul12(int32_t a,int32_t b)
{return (int32_t)((uint32_t)a*(uint32_t)b)>>12;}

/* func_800799E4: now pe_plat_gte_trig.c (clean-room, docs/evidence/native-gte-trig-20261030) */

void func_8018F05C(void)
{
    const pe_addr_t scratch=0x1F800280u,eye=0x8019C330u,target=0x8019C810u;
    const pe_addr_t angles=0x8019BFC4u,matrix=0x8019CC30u;
    uint8_t saved[128];unsigned epoch=PE_Port_StopEpoch();
    for(unsigned i=0;i<128;i++)saved[i]=PE_LoadU8(scratch+i);
    /* Scratch offsets correspond to original stack locals at sp+0x10. */
    for(unsigned i=0;i<32;i++) {
        uint8_t b=PE_LoadU8(0x8018EFF4u+i);
        PE_StoreU8(scratch+32u+i,b);PE_StoreU8(scratch+64u+i,b);
    }
    for(unsigned i=0;i<3;i++)PE_StoreU32(scratch+i*4u,PE_LoadU32(target+i*4u)-PE_LoadU32(eye+i*4u));
    func_80078134(scratch,scratch+16u);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    uint32_t x=PE_LoadU32(scratch+16u),z=PE_LoadU32(scratch+24u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(scratch+52u+i*4u,0u-PE_LoadU32(eye+i*4u));
    uint32_t length=func_80078004(x*x+z*z);
    PE_StoreU16(angles,(uint16_t)func_80079FB4((int32_t)PE_LoadU32(scratch+20u),(int32_t)length));
    PE_StoreU16(angles+2u,(uint16_t)(func_80079FB4((int32_t)z,(int32_t)x)-1024));
    PE_StoreU16(angles+4u,0);
    func_800794C4(angles,scratch+64u);
    func_800787D4(scratch+64u,scratch+32u,matrix);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    for(unsigned i=0;i<3;i++) {
        PE_StoreU16(scratch+96u+i*2u,(uint16_t)PE_LoadU32(target+i*4u));
        PE_StoreU16(scratch+104u+i*2u,(uint16_t)PE_LoadU32(eye+i*4u));
        PE_StoreU32(scratch+112u+i*4u,i==1u?(uint32_t)-20000:0u);
    }
    func_8018F344(matrix,scratch+104u,scratch+96u,scratch+112u);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    x=PE_LoadU32(scratch);z=PE_LoadU32(scratch+8u);
    length=func_80078004(x*x+z*z);
    PE_StoreU16(angles,(uint16_t)(1024u-(uint32_t)func_80079FB4((int32_t)length,(int32_t)(0u-PE_LoadU32(scratch+4u)))));
    PE_StoreU16(angles+2u,(uint16_t)(1024u-(uint32_t)func_80079FB4((int32_t)z,(int32_t)x)));
    PE_StoreU16(angles+4u,0);
    func_800799E4(angles,0x8019CDF0u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x8019CE04u+i*4u,PE_LoadU32(eye+i*4u));
    func_80078E94(matrix);func_80078E04(matrix);
 done:
    for(unsigned i=0;i<128;i++)PE_StoreU8(scratch+i,saved[i]);
}
