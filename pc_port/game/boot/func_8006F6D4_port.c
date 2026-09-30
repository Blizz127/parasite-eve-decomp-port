/*
 * PE-BTL50 — func_8006F6D4 event tick plus D4698
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 6F6D4: 83 words 0x8006F6D4..0x8006F820, SHA-256 c38426e2…1389.
 * D4698: 27 words 0x800D4698..0x800D4704, SHA-256 c33ca3b9…6991.
 *
 * Opcode 0x6B / 187C0 jals 6F6D4(*arg0, 0, *arg1, *arg2,
 * *arg3, *arg4). Live (local[7]=0, 1, 0, 0, 0) ticks the
 * 0x6A slot. table[0x55]+8 = 0x800D4698.
 *
 * D4698 a1==0 writes slot → 0x800F32D0 and slot+0x0C →
 * 0x800E2368, then jalrs *(slot+0x8C)+0x30. The early
 * resource-free fixture left +0x8C=0. STG7 restores room
 * exports, so M0005 now supplies descriptor 80190B44 and
 * callback 80190A6C, now native through the effect callback dispatcher.
 *
 * Unknown jalr targets are not invented.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800942E0 0x800942E0u
#define GA_D_800942E4 0x800942E4u
#define GA_D_800942E8 0x800942E8u
#define GA_D_800F32D0 0x800F32D0u
#define GA_D_800E2368 0x800E2368u
#define GA_FN_D4698   0x800D4698u

int func_800D4698(pe_addr_t slot, unsigned int mode, unsigned int a2,
                  unsigned int a3, unsigned int extra0, unsigned int extra1)
{
    pe_addr_t rec;
    pe_addr_t fn_addr;
    uint32_t fn;

    if (mode != 0u) return 0;
    rec = PE_LoadU32(slot + 0x8Cu);
    PE_StoreU32(GA_D_800F32D0, slot);
    PE_StoreU32(GA_D_800E2368, slot + 0x0Cu);
    fn_addr = rec + 0x30u;
    if (fn_addr<0x200000u) fn_addr|=0x80000000u;
    fn = PE_LoadU32(fn_addr);
    PE_EffectCallback_SetExtra1(extra1);   /* retail a3 for overlay callbacks */
    if (fn) PE_StoreU32(slot+0x14u,(uint32_t)PE_EffectCallback(fn,(int32_t)a2,a3,extra0));
    return 0;
}

int func_8006F6D4(unsigned int index, unsigned int mode, unsigned int a2,
                  pe_addr_t out0, pe_addr_t out1, pe_addr_t out2)
{
    pe_addr_t pool;
    pe_addr_t slot;
    pe_addr_t table;
    pe_addr_t entry;
    pe_addr_t fn;
    unsigned int code;
    unsigned int stride;

    if (index >= 0x16u)
        return -10;
    if (index < 0xBu) {
        pool = PE_LoadU32(GA_D_800942E4);
        stride = 0xA0Cu;
        slot = pool + index * stride;
    } else {
        pool = PE_LoadU32(GA_D_800942E8);
        stride = 0x10Cu;
        slot = pool + (index - 0xBu) * stride;
    }

    code = PE_LoadU8(slot + 1u);
    if (code >= 0xC0u)
        return -11;
    if (code >= 0x55u)
        code = 0x55u;

    table = PE_LoadU32(GA_D_800942E0);
    entry = PE_LoadU32(table + code * 4u);
    if (entry == 0u)
        return -12;
    fn = PE_LoadU32(entry + 8u);
    if (fn == 0u)
        return -1;

    if (mode == 1u && a2 == 0u) {
        PE_StoreU32(out0, PE_LoadU8(slot + 2u));
        PE_StoreU32(out1, PE_LoadU8(slot + 3u));
        PE_StoreU32(out2, PE_LoadU32(slot + 4u));
    }

    if (fn == 0x8018F0B8u && PE_M34BossEffectOverlay())
        return PE_M34BossEffectCommand(slot, mode, a2, out0, out1, out2);
    if (fn == 0x801915ACu && PE_M28MovementOverlay())
        return PE_M28MovementCommand(slot, mode, a2, out0, out1, out2);
    if (fn == 0x8019159Cu && PE_M32MovementOverlay())
        return PE_M32MovementCommand(slot, mode, a2, out0, out1, out2);
    if (fn == 0x8018F020u && PE_MirrorOverlay())
        (void)PE_MirrorCommand(slot, mode, a2, out0, out1);
    else if (fn == GA_FN_D4698)
        (void)func_800D4698(slot, mode, a2, out0, out1, out2);
    else if (fn==0x800C9B3Cu || fn==0x800CD89Cu || fn==0x800CE118u)
        (void)func_800C2AF0(slot,(int32_t)mode,(int32_t)a2,out0);
    return 0;
}
