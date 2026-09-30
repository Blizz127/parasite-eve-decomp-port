/* SEW17: original field item award and pickup window, native translation.
 * 194B0..194F8, 15BAC..15C7C, 532B4..5332C, 4F490..4F808,
 * 50204..50260, 51060..51084, 19540..19618, 4C34C..4C4B4 and
 * 55E14..55FB4 in the SHA-1-verified Disc 1 EXE. */
#include "psx_compat.h"
#include "pe_port_compat.h"

pe_addr_t func_800532B4(uint32_t id)
{
    if (id-256u<128u) return 0x800BEEACu+id*32u;
    if (id-1u<255u) return func_8005DB44(id-1u);
    if (id-512u<9u) return 0x8009DE64u+id*32u;
    return 0u;
}

int func_800194B0(pe_addr_t args)
{
    int result=func_80053D2C((int32_t)PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU32(PE_LoadU32(args+4u),(uint32_t)result);
    return 1;
}

/* Retail 55E14..55FB4: mark items eligible for chest storage. */
void func_80055E14(void)
{
    int32_t count=(int32_t)PE_LoadU32(0x8009D064u);
    pe_addr_t bits=PE_LoadU32(0x8009D058u);
    for (int32_t i=0;i<count;i++) PE_StoreU32(bits+(uint32_t)i*4u,0u);
    for (int32_t i=0;i<(int32_t)PE_LoadU32(0x8009D050u);i++) {
        pe_addr_t record,entry;uint32_t value;
        if (i==(int8_t)PE_LoadU8(0x800C0E20u) ||
            i==(int8_t)PE_LoadU8(0x800C0E22u) || !func_80057654(i)) continue;
        record=func_8005332C(i);
        entry=PE_LoadU32(0x8009D058u)+((uint32_t)i>>5u)*4u;
        value=PE_LoadU32(entry);
        if (record && !(PE_LoadU8(record+5u)&0xE0u)) value|=1u<<((uint32_t)i&31u);
        PE_StoreU32(entry,value);
    }
}

/* Retail 4C34C..4C4B4: create the item-selection window once. */
int func_8004C34C(uint32_t item)
{
    pe_addr_t window,list;uint32_t saved;
    if (func_80062A34(1u,1u)) return 1;
    func_8005DE88();
    window=func_80062D2C(27u,0u,0u,0u);PE_StoreU32(window+48u,0x800447F0u);
    window=func_80062D2C(1u,0u,0u,0u);list=func_8006322C(1u,window,window);
    PE_StoreU32(window+44u,0x80044444u);PE_StoreU32(list+48u,0x8004F8D0u);
    func_80062CB8(list);
    PE_StoreU32(list+132u,0x80057C54u);PE_StoreU32(list+136u,0x80050260u);
    func_80055760();PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
    func_800647D0(list,(int32_t)func_80052F70());
    saved=PE_LoadU32(0x8009CF98u);PE_StoreU32(0x8009CF00u,0u);
    if (saved) {
        saved--;
        PE_StoreU32(list+68u,saved&1u);PE_StoreU32(list+72u,(saved>>1u)&127u);
        PE_StoreU32(list+92u,(uint32_t)((int32_t)saved>>8));
    }
    func_8004C594();PE_StoreU32(func_80062A34(2u,1u)+132u,0u);
    func_80055E14();PE_StoreU32(0x8009CF00u,1u);PE_StoreU32(0x8009CF5Cu,item);
    PE_StoreU32(0x8009CEFCu,0u);func_8005B890(0);
    return 0;
}

/* func_80019540: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019540_port.c (src/func_80019540.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_8004F490(uint32_t item)
{
    pe_addr_t record=func_800532B4(item),window,list,help;
    PE_StoreU32(0x8009CF58u,record);
    if (PE_LoadU8(record+6u)<10u) {
        window=func_80062D2C(5u,0u,0u,0u);list=func_8006322C(5u,window,window);
        PE_StoreU32(window+48u,0x8004F644u);PE_StoreU32(window+44u,0x8004F730u);
        PE_StoreU32(window+52u,128u);func_80063158(window,68,20);
        PE_StoreU32(list+48u,0x80050204u);func_80064C20(list);
        PE_StoreU32(list+112u,0xFFFFFFFFu);func_80062CB8(window);
        window=func_80062D2C(6u,0u,0u,0u);PE_StoreU32(window+52u,128u);
        func_80063158(window,68,20);list=func_8006322C(6u,window,window);
        PE_StoreU32(list+48u,0x8005022Cu);func_80064C20(list);
        PE_StoreU32(list+60u,62u);
        func_800647D0(list,PE_LoadU8(PE_LoadU32(0x8009CF58u)+20u));
        PE_StoreU32(0x8009CF18u,PE_LoadU8(PE_LoadU32(0x8009CF58u)+6u)!=9u);
    } else {
        window=func_80062D2C(55u,0u,0u,0u);
        PE_StoreU32(window+48u,0x8004F7D8u);PE_StoreU32(window+44u,0x8004F730u);
        func_80062CB8(window);
    }
    help=func_80062D2C(19u,0u,0u,0u);PE_StoreU32(help+48u,0x8004F798u);
    func_8005DE88();
}

/* func_80015BAC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80015BAC_port.c (src/func_80015BAC.c); hand port retired (VM batch 3, port3). */

/* func_8004F644: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F644_port.c (src/func_8004F644.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_8004F730(pe_addr_t window,uint32_t event)
{
    (void)window;
    if (event&0x10040u) {
        if (PE_LoadU32(0x8009CF38u)) {
            PE_StoreU32(0x8009CF38u,0u);
            PE_MenuCommitResult(PE_LoadU8(PE_LoadU32(0x8009CF58u)+4u)+3u);
        } else func_800512AC(9,0u);
        func_800525EC();
    }
    return 1;
}

/* func_8004F798: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F798_port.c (src/func_8004F798.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_8004F7D8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004F7D8_port.c (src/func_8004F7D8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80051060: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80051060_port.c (src/func_80051060.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
/* func_80050204: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80050204_port.c (src/func_80050204.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
void func_8005022C(pe_addr_t list)
{
    PE_StoreU32(0x8009CEF4u,list);
    PE_StoreU32(0x8009CF20u,PE_LoadU32(0x8009CF58u));
    func_800638D8(list,0x80050AD8u);
}
