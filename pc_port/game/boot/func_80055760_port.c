/* Original item availability, item rearrangement and inventory list drawing.
 * 44AA0.s / 486D8.s / 467E0.s / 340EC.s / 40038.s / 40F48.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004E970: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004E970_port.c (src/func_8004E970.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80055760(void)
{
    uint32_t saved=func_80052F0C(),gun=0u,armor=0u,bank;
    int32_t i;
    for (bank=0;bank<(D_8009D04C?2u:1u);bank++) {
        func_80052E30(bank);
        for (i=0;i<(int32_t)D_8009D050;i++)
            if ((254u>>((uint32_t)func_8005415C(i)&31u))&1u) break;
        gun|=i<(int32_t)D_8009D050;
        for (i=0;i<(int32_t)D_8009D050;i++) if (func_8005415C(i)==9) break;
        armor|=i<(int32_t)D_8009D050;
    }
    for (bank=0;bank<(D_8009D04C?2u:1u);bank++) {
        int profile;
        func_80052E30(bank);
        for (i=0;i<(int32_t)D_8009D064;i++) PE_StoreU32(D_8009D058+(uint32_t)i*4u,0u);
        profile=func_8004E970();
        for (i=0;i<(int32_t)D_8009D050;i++) {
            pe_addr_t record=func_8005332C(i),bits;uint32_t enabled,subtype;
            if (!record) continue;
            enabled=(PE_LoadU8(record+5u)>>(func_8005B89C()?1u:0u))&1u;
            if (PE_LoadU8(record+6u)==10u) {
                subtype=PE_LoadU8(record+14u);
                if (profile) {if (subtype==2u) enabled=0u;}
                else if (subtype-4u<3u) enabled&=gun;
                else if (subtype-12u<3u) enabled&=armor;
            }
            if ((uint32_t)(PE_LoadU8(record+4u)-6u)<5u && PE_LoadU16(0x800C0E08u)>=PE_LoadU16(0x800C0E06u))
                enabled=0u;
            bits=D_8009D058+((uint32_t)i>>5u)*4u;
            PE_StoreU32(bits,PE_LoadU32(bits)|(enabled<<((uint32_t)i&31u)));
        }
    }
    func_80052E30(saved);
}

/* func_80050260: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050260_port.c (src/func_80050260.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80055FE0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80055FE0_port.c (src/func_80055FE0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80057C54: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80057C54_port.c (src/func_80057C54.c); hand port retired (port3 switch-over G). */

int32_t func_8005401C(void)
{
    uint32_t count=0u;pe_addr_t p,end;
    func_80052E30(0u);p=D_8009D048;end=p+D_8009D050*2u;
    while (p<end) {count+=PE_LoadU16(p)!=0u;p+=2u;}
    return (int32_t)count;
}

/* func_80054240: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80054240_port.c (src/func_80054240.c); hand port retired (port3 switch-over G). */

/* func_8005FA3C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8005FA3C_port.c (src/func_8005FA3C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80063158: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80063158_port.c (src/func_80063158.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80062F1C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80062F1C_port.c (src/func_80062F1C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80064C80: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80064C80_port.c (src/func_80064C80.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80050804: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050804_port.c (src/func_80050804.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_8004F8D0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F8D0_port.c (src/func_8004F8D0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_800447F0(pe_addr_t window)
{
    pe_addr_t list=func_80062A34(2u,1u),spec=func_8005DA8C(1u);
    uint32_t y=PE_LoadU32(spec+4u)+PE_LoadU32(list+56u)*16u+4u-PE_LoadU32(window+28u);
    func_80063158(window,0,(int32_t)y);
    if (PE_LoadU32(list+104u)) {
        int32_t difference=(int32_t)(PE_LoadU32(list+88u)-PE_LoadU32(list+56u)-PE_LoadU32(list+92u));
        int32_t scroll=(int32_t)PE_LoadU32(list+96u);
        if (!difference) func_80063158(window,0,(int32_t)((uint32_t)scroll-16u));
        else if (difference==1 && scroll<0) func_80063158(window,0,scroll);
    }
    func_8005E8A4(10,2);func_8005F27C(func_8005DC4C(37u));func_8005E8A4(70,4);
    func_8005FA3C(func_8005401C());func_8005EB64(76u);func_8005E8A4(5,0);
    func_8005FA3C((int32_t)func_80052F70());
}

void func_80044174(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(27u,0u,0u,0u),list;
    uint32_t saved;
    PE_StoreU32(window+48u,0x800447F0u);window=func_80062D2C(1u,owner,0u,0u);
    list=func_8006322C(1u,window,window);PE_StoreU32(window+44u,0x80044444u);
    PE_StoreU32(list+48u,0x8004F8D0u);func_80062CB8(list);
    PE_StoreU32(list+132u,0x80057C54u);PE_StoreU32(list+136u,0x80050260u);func_80055760();
    PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
    func_800647D0(list,(int32_t)func_80052F70());saved=PE_LoadU32(0x8009CF98u);
    PE_StoreU32(0x8009CF00u,0u);
    if (saved) {
        saved--;PE_StoreU32(list+68u,saved&1u);PE_StoreU32(list+72u,(saved>>1u)&127u);
        PE_StoreU32(list+92u,(uint32_t)((int32_t)saved>>8));
    }
}
