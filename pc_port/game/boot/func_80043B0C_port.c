/* Original inventory command, equipment and stat drawing.
 * 340EC.s / 37CD0.s / 40038.s / 40F48.s / 43CE4.s / 50074.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include <limits.h>

/* func_80051E48: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80051E48_port.c (src/func_80051E48.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
pe_addr_t func_8005DBF8(void) {return 0x800A8028u+PE_LoadU32(0x800A8040u);}

/* func_800605C4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800605C4_port.c (src/func_800605C4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80060590: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80060590_port.c (src/func_80060590.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8006055C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006055C_port.c (src/func_8006055C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80060528: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80060528_port.c (src/func_80060528.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80021080: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80021080_port.c (src/func_80021080.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80052534: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80052534_port.c (src/func_80052534.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800602D0(int32_t value,int32_t digits)
{
    int32_t divisor=1,i;
    for (i=1;i<digits;i++) divisor=(int32_t)((uint32_t)divisor*10u);
    if (value<0) {
        value=(int32_t)(0u-(uint32_t)value);divisor/=10;digits--;
        while (value<divisor) {
            func_8005EED4(15u);divisor/=10;digits--;
        }
        func_8005F5B8(112u);func_8005E8A4(9,0);
    }
    for (i=0;i<digits;i++) {
        int32_t digit;
        if (!divisor || (value==INT32_MIN && divisor==-1)) abort();
        digit=value/divisor;
        func_8005EED4(i<digits-1 && !digit?15u:(uint8_t)(digit%10));
        divisor/=10;
    }
}

/* func_800605F8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800605F8_port.c (src/func_800605F8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800536B8(int32_t index)
{
    int32_t id=(int16_t)PE_LoadU16(D_8009D048+(uint32_t)index*2u);
    pe_addr_t record,name=0u;
    if ((uint32_t)(id-256)<128u) {
        record=0x800BEEACu+(uint32_t)id*32u;
        if (PE_LoadU8(record+5u)&16u) name=0x800C20A4u+(PE_LoadU8(record+6u)==9u?16u:0u);
        else name=func_8005DC9C(PE_LoadU8(record+4u)-1u);
    } else if ((uint32_t)(id-1)<255u) name=func_8005DC9C((uint32_t)(id-1));
    else if ((uint32_t)(id-512)<9u) name=func_8005DC9C(D_8009D03C+(uint32_t)id-513u);
    record=func_8005332C(index);
    if (record) func_800534E4(record,name);
}

void func_80043B0C(pe_addr_t node)
{
    uint32_t level;int32_t remaining;
    (void)node;
    func_8005E8A4(2,2);func_8005EB64(71u);func_8005E8A4(40,2);func_8005F27C(func_8005BEE8());
    func_8005E8A4(0,21);func_8005EB64(151u);func_8005E8A4(60,0);
    func_800605C4(PE_LoadU8(0x800C0E0Au)+1);
    func_8005E8A4(-120,26);func_8005EB64(152u);func_8005E8A4(66,0);
    level=PE_LoadU8(0x800C0E0Au);
    remaining=level<98u?(int32_t)(PE_LoadU32(func_8005DBF8()+level*4u+4u)-PE_LoadU32(0x800C0E00u)):0;
    func_80060528(remaining);func_8005E8A4(-124,21);func_8005EB64(148u);
    func_8005E8A4(0,16);func_800536B8((int8_t)PE_LoadU8(0x800C0E20u));
    func_8005E8A4(0,16);
    if ((int8_t)PE_LoadU8(0x800C0E22u)<0) func_8005F5B8(57u);
    else func_800536B8((int8_t)PE_LoadU8(0x800C0E22u));
}

void func_80043C64(pe_addr_t node)
{
    uint32_t i;pe_addr_t bonuses=func_80051E48();
    (void)node;func_8005E8A4(4,5);
    for (i=1u;i<7u;i++) {
        uint32_t delta=PE_LoadU32(0x8009CF40u)*PE_LoadU32(0x800A18FCu+i*4u);
        int32_t value=(int32_t)(PE_LoadU32(0x800A18D8u+i*4u)+(uint32_t)((int32_t)delta>>7));
        int32_t bonus=(int32_t)PE_LoadU32(bonuses+i*4u),index;
        func_8005E8A4(2,0);func_8005EB64(i+140u);func_8005E8A4(74,0);
        PE_func_8005B91C_HostOut((int32_t)i,value,&index,NULL);func_800605F8(index+1);
        if (bonus) {
            func_8005E8A4(6,0);func_8005F5B8(bonus>0?111u:112u);
            func_800605F8(bonus<0?(int32_t)(0u-(uint32_t)bonus):bonus);func_8005E8A4(-24,0);
        }
        func_8005E8A4(-94,i==4u?22:14);
    }
}

/* func_8004905C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004905C_port.c (src/func_8004905C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80050748(uint32_t index)
{
    int32_t remaining=(int32_t)index,option=-1;
    uint32_t enabled=PE_LoadU32(0x8009CEF0u)&(func_8005B89C()?0x1Fu:0x1EFu);
    while (remaining>=0) {
        remaining-=(int32_t)(enabled&1u);option++;enabled>>=1u;
        if (option>=9) break;
    }
    if ((int32_t)index!=func_80063428(PE_LoadU32(0x8009CEF4u)) || (option==4 && !func_80052534()))
        func_8005EB58(1u);
    func_8005E8A4(-2,-2);func_8005EB64((uint32_t)(option+86));
}

void func_8004F838(pe_addr_t node)
{
    uint32_t rows;
    PE_StoreU32(0x8009CEF4u,node);func_800638D8(node,0x80050748u);func_8005EB58(1u);
    for (rows=PE_LoadU32(node+56u);rows;rows--) {func_8005EB64(104u);func_8005E8A4(0,16);}
    if (!func_8005257C() && PE_LoadU32(0x8009CEF8u)) func_80033A40();
}
