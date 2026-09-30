/*
 * PE-FTE1 — func_80070E54 frame tail, func_80042FE8 LoadImage wrapper,
 * func_8006EBE4 status halfword.  Native translations of the matched
 * src/ leaves (src/func_80070E54.c, src/func_80042FE8.c,
 * src/func_8006EBE4.c); authority is the retail Disc 1 executable
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.  Oracle:
 * pc_port/tools/pe_fte1_70e54_oracle.py.
 *
 * func_80070E54: 86 words 0x80070E54..0x80070FAC.  Callers: the 6E9A0
 * fade poll (jal @ 0x8006EB54) and the 3F3C4 field tick (jal @
 * 0x8003F590).  Retail order:
 *   DrawSync(0); 42FE8();
 *   if (B0CD8 & 0x200) { VSync(4); if ((short)6EBE4() >= 3) SetDispMask(1); }
 *   else VSync(2);
 *   74A44(1); PutDispEnv(BCE80 + 20*CDDC);
 *   if ((char)6EC08() || (B0CD8 & 0x200)) PutDrawEnv(BCDC8 + 92*CDDC);
 *   else DrawOTagEnv(B0E38[CDDC] + 0x3FFC, BCDC8 + 92*CDDC);
 *   CDDC = (CDDC == 0);
 * The retail OT-pointer load is `lw 0x160(B0CD8 + 4*CDDC)`, i.e.
 * D_800B0E38[CDDC] addressed as a field of the D_800B0CD8 block.
 * D_8009CDDC lives in guest RAM (gp+0x6C) — the same word the 3F3C4
 * port reads and flips; PutDispEnv receives the host view of the
 * guest DISPENV exactly as the field tick passes it.
 *
 * func_80042FE8: 20 words 0x80042FE8..0x80043038.  When D_8009CED8
 * (gp+0x168) == 6, LoadImage({0, 0x1E0, 0x100, D_8009CEDC}, D_800B0E54).
 * The RECT is a retail stack temporary; the port builds it on the host
 * stack because func_8007506C takes the host RECT view.
 *
 * func_8006EBE4: 6 words.  D_800B0DBA ? (short)D_800B0DBC : -1.
 *
 * Not a matching leaf in pc_port terms: these files are native
 * translations; the byte-exact claim belongs to src/ under the era gate.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"
#include "pe_guest_ram.h"

extern int func_80074A44(int mode);
extern int func_8006EC08(void);

/* func_8006EBE4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8006EBE4_port.c (src/func_8006EBE4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

void func_80042FE8(void)
{
    RECT rect;

    if (PE_LoadU32(0x8009CED8u) != 6u)
        return;
    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = 0x100;
    rect.h = (int16_t)PE_LoadU32(0x8009CEDCu);
    (void)func_8007506C(&rect, PE_LoadU32(0x800B0E54u));
}

void func_80070E54(void)
{
    uint32_t cddc;

    func_80074DC0(0);
    func_80042FE8();
    if ((PE_LoadU32(0x800B0CD8u) & 0x200u) != 0u) {
        func_80073A44(4);
        if ((int16_t)func_8006EBE4() >= 3)
            func_80074D28(1);
    } else {
        func_80073A44(2);
    }
    func_80074A44(1);
    cddc = PE_LoadU32(0x8009CDDCu);
    uint32_t epoch=PE_Port_StopEpoch();
    func_800755F0(0x800BCE80u + cddc * 20u);
    if(PE_Port_StopEpoch()!=epoch)return;
    if ((int8_t)func_8006EC08() != 0 ||
        (PE_LoadU32(0x800B0CD8u) & 0x200u) != 0u) {
        cddc = PE_LoadU32(0x8009CDDCu);
        (void)func_80075424(0x800BCDC8u + 92u * cddc);
    } else {
        cddc = PE_LoadU32(0x8009CDDCu);
        func_800754E4(PE_LoadU32(0x800B0CD8u + 0x160u + cddc * 4u) + 0x3FFCu,
                      0x800BCDC8u + 92u * cddc);
    }
    cddc = PE_LoadU32(0x8009CDDCu);
    PE_StoreU32(0x8009CDDCu, cddc == 0u);
}
