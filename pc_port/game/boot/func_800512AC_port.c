/* Original menu result publication and equipment application.
 * 41A58.s / 120D8.s. Inventory buffer globals retain their existing
 * native ownership; all actor, item and menu records are guest addresses. */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_80051244(void) {PE_StoreU32(0x8009D014u,0x800A1AA0u);}
/* func_8005D970: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005D970_port.c (src/func_8005D970.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* Native callers pass an original stack-local command value directly. */
void PE_MenuCommitResult(uint32_t result)
{
    PE_StoreU32(0x8009D010u,result);
    if (result) func_80062F9C();
}

void PE_MenuApplyAmmo(pe_addr_t item)
{
    pe_addr_t aya=PE_LoadU32(0x8009D254u),record=aya?PE_LoadU32(aya):0u,equipment;
    uint32_t value;
    if (!record) return;
    if (item<0x200000u) item|=0x80000000u;
    equipment=PE_LoadU32(record+104u);
    value=(PE_LoadU32(equipment+12u)&~0x3FFu)|(PE_LoadU16(item+10u)&0x3FFu);
    PE_StoreU32(equipment+12u,value);
    value=PE_LoadU8(item+6u);equipment=PE_LoadU32(record+104u);
    if (value && value<8u) value=value>4u?value-4u:1u;
    else {value=PE_LoadU8(item+6u);value=value>=19u?value-18u:0u;}
    PE_StoreU32(equipment+12u,(PE_LoadU32(equipment+12u)&~0x300000u)|((value&3u)<<20u));
}

void func_800218D8(void)
{
    static const uint32_t packed_masks[]={0x3FFu,0xFFC00u,0x300000u};
    static const uint32_t ability_masks[]={15u,48u,192u,0x100u,0x200u,0x400u,0x800u,
        0x1000u,0x6000u,0x8000u,0x10000u,0x20000u};
    pe_addr_t out=PE_LoadU32(PE_LoadU32(0x8009D278u)+104u);
    uint32_t packed=PE_LoadU32(out+12u),abilities=PE_LoadU32(out+16u),i;
    for (i=0;i<4;i++) PE_StoreU16(out+i*2u,PE_LoadU16(0x800A76D8u+i*2u));
    PE_StoreU32(out+8u,PE_LoadU32(0x800A76E0u));
    for (i=0;i<3;i++) {
        packed=(packed&~packed_masks[i])|(PE_LoadU32(0x800A76E4u)&packed_masks[i]);
        PE_StoreU32(out+12u,packed);
    }
    for (i=0;i<sizeof(ability_masks)/sizeof(ability_masks[0]);i++) {
        abilities=(abilities&~ability_masks[i])|(PE_LoadU32(0x800A76E8u)&ability_masks[i]);
        PE_StoreU32(out+16u,abilities);
    }
}

void func_80021AF8(void)
{
    static const uint32_t packed_masks[]={0x3FFu,0xFFC00u,0xFF00000u,0xF0000000u};
    static const uint32_t ability_masks[]={1u,2u,4u,8u,16u,0x1E0u,0x200u,0x400u,
        0x800u,0x1000u,0x2000u,0x4000u,0x8000u,0x10000u,0x20000u};
    pe_addr_t out=PE_LoadU32(PE_LoadU32(0x8009D278u)+108u);
    uint32_t packed=PE_LoadU32(out),abilities=PE_LoadU32(out+4u),i;
    for (i=0;i<4;i++) {
        packed=(packed&~packed_masks[i])|(PE_LoadU32(0x800A76D8u)&packed_masks[i]);
        PE_StoreU32(out,packed);
    }
    for (i=0;i<sizeof(ability_masks)/sizeof(ability_masks[0]);i++) {
        abilities=(abilities&~ability_masks[i])|(PE_LoadU32(0x800A76DCu)&ability_masks[i]);
        PE_StoreU32(out+4u,abilities);
    }
}

void func_800254BC(int32_t command)
{
    if (!(D_8009D1A0&2u)) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u);
        PE_StoreU32(0x8009D278u,PE_LoadU32(aya));
        func_8006DE80(1107,1,(int16_t)PE_LoadU16(aya+42u),(int16_t)PE_LoadU16(aya+46u),
            (int16_t)PE_LoadU16(aya+50u));
    }
    if (command==407) {
        func_800218D8();func_800209F0();PE_StoreU8(PE_LoadU32(0x8009D278u)+18u,13u);
    } else if (command==408) func_80021AF8();
}

void func_800512AC(int cmd,pe_addr_t src)
{
    pe_addr_t aya=PE_LoadU32(0x8009D254u),record=aya?PE_LoadU32(aya):0u;
    pe_addr_t out,equipment;uint32_t first[4],i;
    switch (cmd) {
    case 0:PE_MenuCommitResult(PE_LoadU32(src)+3u);return;
    case 1:PE_MenuCommitResult(PE_LoadU32(src)+387u);return;
    case 2:
        if (!record) break;
        out=PE_LoadU32(0x8009D1E0u);
        if (out) {
            equipment=PE_LoadU32(record+104u);
            for (i=0;i<4;i++) first[i]=PE_LoadU32(equipment+i*4u);
            for (i=0;i<4;i++) PE_StoreU32(out+i*4u,first[i]);
            first[0]=PE_LoadU32(equipment+16u);first[1]=PE_LoadU32(equipment+20u);
            PE_StoreU32(out+16u,first[0]);PE_StoreU32(out+20u,first[1]);
            func_80051980(0,PE_LoadU32(0x8009D1E0u));
        }
        if (func_8005B89C()) PE_StoreU32(0x8009D010u,407u);
        else func_800254BC(407);
        break;
    case 3:
        if (!record) break;
        out=PE_LoadU32(0x8009D1E0u);
        if (out) {
            equipment=PE_LoadU32(record+108u);
            first[0]=PE_LoadU32(equipment);first[1]=PE_LoadU32(equipment+4u);
            PE_StoreU32(out,first[0]);PE_StoreU32(out+4u,first[1]);
            func_80051E64(PE_LoadU32(0x8009D1E0u));
        }
        if (func_8005B89C()) PE_StoreU32(0x8009D010u,408u);
        else func_800254BC(408);
        break;
    case 5:
        if (!record) break;
        /* The guest argument can alias the packed output. Preserve each
         * original pointer reload; native stack-local callers use the helper. */
        equipment=PE_LoadU32(record+104u);out=PE_LoadU32(src);
        if (out<0x200000u) out|=0x80000000u;
        i=(PE_LoadU32(equipment+12u)&~0x3FFu)|(PE_LoadU16(out+10u)&0x3FFu);
        PE_StoreU32(equipment+12u,i);
        out=PE_LoadU32(src);if (out<0x200000u) out|=0x80000000u;
        i=PE_LoadU8(out+6u);equipment=PE_LoadU32(record+104u);
        if (i && i<8u) i=i>4u?i-4u:1u;
        else {
            out=PE_LoadU32(src);if (out<0x200000u) out|=0x80000000u;
            i=PE_LoadU8(out+6u);i=i>=19u?i-18u:0u;
        }
        PE_StoreU32(equipment+12u,(PE_LoadU32(equipment+12u)&~0x300000u)|((i&3u)<<20u));
        break;
    case 8:PE_StoreU32(0x8009D010u,409u);break;
    case 9:PE_StoreU32(0x8009D010u,UINT32_MAX);break;
    case 10:PE_StoreU32(0x8009D010u,1000u);break;
    case 11:PE_StoreU32(0x8009D010u,1u);break;
    case 12:D_8009D018=4u;func_80052E30(0u);func_80051CC4();PE_StoreU32(0x8009D010u,2u);break;
    default:break;
    }
    if (PE_LoadU32(0x8009D010u)) func_80062F9C();
}
