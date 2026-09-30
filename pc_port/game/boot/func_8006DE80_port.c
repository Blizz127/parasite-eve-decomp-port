/*
 * PE-BTL99 — func_8006DE80 ABI wrapper into func_8006DED4.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 6DE80: 21 words 0x8006DE80..0x8006DED4 exclusive,
 * SHA-256 860d94bcbf926fb977614f7d917d69f919c20aee54126f58fbba90dc426529d9.
 *
 *   6DED4(lw(D_800B0E08), a0, a1, (int16)a2, (int16)a3, (int16)a4)
 *
 * 1F814 jal @ 0x8001F970 after 1A680, before +0x98 bit 0x100:
 *   a0=0x46A, a1=0, a2=lh Aya+0x2A, a3=lh Aya+0x2E,
 *   stack=lh Aya+0x32.
 *
 * Death-arm jal @ 0x8001F430 uses a0=0x46B and is not this cut.
 *
 * Spatial pan/attenuation and the sound lookup/queue are now native.
 * The transient SVECTOR and projected position use native storage.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

#define GA_D_800B0E08 0x800B0E08u

/* Complete 6E514 lookup: the header packs a 10-bit count and 22-bit
 * offset; each 12-byte sound entry carries its ID at +A. */
pe_addr_t func_8006E514(pe_addr_t package, uint32_t id)
{
    uint32_t packed=PE_LoadU32(package+PE_LoadU32(package+4u)+0x2Cu);
    uint32_t count=packed>>22u, i;
    pe_addr_t entry=package+(packed&0x3FFFFFu);
    for (i=0;i<count;i++,entry+=12u)
        if (PE_LoadU16(entry+10u)==id)
            return package+(PE_LoadU32(entry+4u)&0xFFFFFFu);
    return 0u;
}

/* func_8006DF50: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DF50_port.c (src/func_8006DF50.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static int spatial_sound(int16_t x, int16_t y, int16_t z, uint32_t *pan, uint32_t *volume)
{
    uint32_t screen,depth;
    int32_t distance,near=PE_LoadU16(0x800B0DD0u),far=PE_LoadU16(0x800B0DD2u);
    int32_t extent=far-near, attenuation,level,spread;
    func_800661A4(); PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    g_pe_gte.h=(int32_t)PE_LoadU32(PE_LoadU32(0x800BCFA8u));
    PE_GTE_SetV0(x,y,z); PE_GTE_RTPS_coordinates(&screen,&depth);
    func_800661CC();
    *pan=(uint32_t)(((int16_t)screen+40)*128/400+64);
    if (*pan>=256u) *pan=255u;
    distance=(int32_t)(depth>>2u);
    if (distance<near) distance=near;
    else if (distance>far) distance=far;
    if (!extent) {
        Bootstrap_ReturnVoid("func_8006DFA8_zero_depth_extent","func_8006DED4");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return 0;
    }
    distance=far-distance;
    attenuation=(int32_t)((uint32_t)distance*(uint32_t)distance);
    attenuation=(int32_t)((int64_t)attenuation/extent);
    spread=(int32_t)PE_LoadU8(0x800B0DCFu)-PE_LoadU8(0x800B0DCEu);
    level=(int32_t)((uint32_t)attenuation*(uint32_t)spread);
    level=(int32_t)((int64_t)level/extent)+PE_LoadU8(0x800B0DCEu);
    *volume=(uint32_t)level; if (*volume>=128u) *volume=127u;
    return 1;
}

int func_8006DFA8(pe_addr_t position, pe_addr_t pan, pe_addr_t volume)
{
    uint32_t p,v;
    if (spatial_sound((int16_t)PE_LoadU16(position),(int16_t)PE_LoadU16(position+2u),
                      (int16_t)PE_LoadU16(position+4u),&p,&v)) {
        PE_StoreU32(pan,p); PE_StoreU32(volume,v);
    }
    return 0;
}

int32_t PE_SpatialSoundRequest(pe_addr_t dest,int id,int a1,int x,int y,int z,uint32_t *computed_volume)
{
    uint32_t pan,volume;
    PE_M34StackInvalidate();
    /* An absent package cannot produce a sound. Early resource-free
     * native fixtures also have no camera or attenuation configuration. */
    if (!dest) return -1;
    if (spatial_sound((int16_t)x,(int16_t)y,(int16_t)z,&pan,&volume)) {
        if(computed_volume)*computed_volume=volume;
        return func_8006DF50(dest,(uint32_t)id,(uint32_t)a1,pan,volume);
    }
    return -1;
}

int32_t func_8006DED4(pe_addr_t dest,int id,int a1,int x,int y,int z)
{
    return PE_SpatialSoundRequest(dest,id,a1,x,y,z,NULL);
}

/* func_8006DE80: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DE80_port.c (src/func_8006DE80.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int32_t func_8006DD38(uint32_t index,uint32_t key,int32_t x,int32_t y,int32_t z)
{
    uint32_t pan,volume;
    if (!spatial_sound((int16_t)x,(int16_t)y,(int16_t)z,&pan,&volume)) return 0;
    if (PE_LoadU8(0x800B0CE8u))
        return func_80086608(PE_LoadU32(0x800B0E0Cu+index*4u),key,pan,volume);
    return 0;
}

/* func_8006DDCC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006DDCC_port.c (src/func_8006DDCC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
