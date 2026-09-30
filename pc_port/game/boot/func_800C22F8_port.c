/* Weapon effect storage and pistol constructors, translated from B2AF8.s,
 * B3390.s, BA234.s and BDF28.s. Guest effect programs remain data; all
 * initialization and callbacks execute as native C. */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_800C6CE0(pe_addr_t slot)
{
    pe_addr_t actor=PE_LoadU32(slot+8u);
    int kind=0;
    uint8_t code=PE_LoadU8(slot+1u);
    if (actor) {
        pe_addr_t body=PE_LoadU32(actor);
        kind=!body?1:(int32_t)PE_LoadU32(body+0x10u)>0?3:2;
    }
    if (code==7u || code==14u) kind=4;
    if (actor==PE_LoadU32(0x8009D254u)) kind=5;
    return kind;
}

pe_addr_t func_800C22F8(pe_addr_t slot)
{
    unsigned i;
    for (i=0xCu;i<0xA0Cu;i++) PE_StoreU8(slot+i,0u);
    PE_StoreU8(slot+2u,0u); PE_StoreU8(slot+3u,0u);
    PE_StoreU32(0x800E2248u,slot+0xCu);
    PE_StoreU32(0x800F34F4u,slot+0x80u);
    PE_StoreU32(0x800F32A8u,slot);
    PE_StoreU32(0x800F3330u,slot+0x200u);
    /* The original repeats the already-zero header and 64 child records. */
    if (func_800C6CE0(slot)==3) {
        pe_addr_t body=PE_LoadU32(PE_LoadU32(slot+8u));
        PE_StoreU32(body,(PE_LoadU32(body)&0xC0FFFFFFu)|0x01000000u);
    }
    return PE_LoadU32(0x800E2248u)+0x6Cu;
}

static void effect_rgb(pe_addr_t address,uint8_t red,uint8_t green,uint8_t blue)
{
    PE_StoreU8(address,red); PE_StoreU8(address+1u,green); PE_StoreU8(address+2u,blue);
}

int func_800C9A70(pe_addr_t slot)
{
    PE_StoreU32(func_800C22F8(slot),0x800E0B38u);
    effect_rgb(0x800E22F8u,128u,128u,128u);
    PE_StoreU8(0x800E22FCu,0xBDu); PE_StoreU8(0x800E22FDu,9u);
    PE_StoreU8(0x800E22FEu,0u); PE_StoreU16(0x800E2300u,0u);
    PE_StoreU16(0x800E2302u,128u);
    effect_rgb(0x800F34B8u,80u,80u,80u);
    PE_StoreU8(0x800F34BCu,0xACu); PE_StoreU8(0x800F34BDu,6u);
    PE_StoreU8(0x800F34BEu,0u); PE_StoreU16(0x800F34C0u,(uint16_t)-50);
    PE_StoreU16(0x800F34C2u,128u);
    return 0;
}

int func_800CD728(pe_addr_t slot)
{
    unsigned i;
    PE_StoreU32(func_800C22F8(slot),0x800E0F6Cu);
    for (i=0;i<3;i++) {
        PE_StoreU16(0x800F33F0u+i*2u,0u);
        PE_StoreU32(0x800F33F8u+i*4u,0x5F4u);
        PE_StoreU16(0x800E27B8u+i*2u,0u);
        PE_StoreU16(0x800E2778u+i*2u,0u);
    }
    effect_rgb(0x800F3408u,0x15u,32u,32u);
    PE_StoreU8(0x800F340Cu,64u); PE_StoreU8(0x800F340Du,32u);
    PE_StoreU16(0x800F340Eu,(uint16_t)-100);
    effect_rgb(0x800E27D0u,255u,176u,176u);
    PE_StoreU8(0x800E27D4u,0x46u); PE_StoreU8(0x800E27D5u,0x30u);
    PE_StoreU16(0x800E27D6u,(uint16_t)-110);
    PE_StoreU32(0x800E2780u,164u); PE_StoreU32(0x800E2784u,164u);
    PE_StoreU32(0x800E2788u,264u);
    effect_rgb(0x800E2790u,128u,128u,128u);
    PE_StoreU8(0x800E2794u,0x6Eu); PE_StoreU8(0x800E2795u,3u);
    PE_StoreU16(0x800E2796u,(uint16_t)-150);
    return 0;
}

int func_800C2AF0(pe_addr_t slot,int32_t unused,int32_t index,uint32_t value)
{
    (void)unused;
    PE_StoreU32(0x800E2248u,slot+0xCu);
    PE_StoreU32(slot+0x54u+(uint32_t)index*4u,value);
    return 0;
}
