/*
 * PE-BTL54 — func_800375E0 message-record open (translated
 * retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 161 words 0x800375E0..0x80037864, SHA-256
 * d7f8b96cf54e7f4e98ec49c081059b5e47916ce9e25a5f9e97294fd1eb58d58a.
 * Zero jal. First free D_800BCEA8 record (byte0==0) becomes
 * state 1; +0x08 = a1; +0x10 = a0 as s16; flags clear
 * 0x00100000 and 0x00200000. a1==0 skips window-geometry copy
 * from gp+0x128..+0x12E. Opcode 0x0D passes a1=0 and a
 * -1-only a2 list, so the decimal-digit loop is not taken.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCEA8 0x800BCEA8u
#define GA_D_8009CEA0 0x8009CEA0u
#define GA_D_8009CEA4 0x8009CEA4u
#define GA_D_8009CED0 0x8009CED0u
#define GA_GP         0x8009CD70u
#define REC_STRIDE    56u

static int32_t pe_375e0_sdiv10(int32_t n)
{
    int64_t prod;
    int32_t hi;

    prod = (int64_t)n * (int32_t)0x66666667;
    hi = (int32_t)(prod >> 32);
    return (hi >> 2) - (n >> 31);
}

void func_800375E0(int id, unsigned int mode, pe_addr_t list)
{
    unsigned int slot;
    pe_addr_t rec;
    uint32_t flags;
    int16_t value;
    unsigned int list_i;
    unsigned int digit_i;
    int32_t quot;
    uint16_t raw;
    pe_addr_t dest;

    id = (int)(int16_t)id;
    mode &= 0xFFu;
    for (slot = 0; slot < 4u; slot++) {
        rec = GA_D_800BCEA8 + slot * REC_STRIDE;
        if (PE_LoadU8(rec) != 0u)
            continue;
        flags = PE_LoadU32(rec + 0x0Cu);
        PE_StoreU8(rec, 1u);
        PE_StoreU8(rec + 9u, 0u);
        PE_StoreU16(rec + 0x10u, (uint16_t)id);
        PE_StoreU8(GA_D_8009CEA0, 0u);
        PE_StoreU8(GA_D_8009CEA4, 0xFFu);
        PE_StoreU8(rec + 8u, (uint8_t)mode);
        flags &= 0xFFCFFFFFu;
        flags &= 0xFFDFFFFFu;
        PE_StoreU32(rec + 0x0Cu, flags);
        if (mode != 0u) {
            PE_StoreU16(rec + 0x12u, PE_LoadU16(GA_GP + 0x128u));
            PE_StoreU16(rec + 0x14u, PE_LoadU16(GA_GP + 0x12Au));
            PE_StoreU16(rec + 0x16u, PE_LoadU16(GA_GP + 0x12Cu));
            PE_StoreU16(rec + 0x18u, PE_LoadU16(GA_GP + 0x12Eu));
            if (mode == 3u)
                PE_StoreU32(rec + 0x0Cu, PE_LoadU32(rec + 0x0Cu) | 0x100000u);
        } else if (PE_LoadU8(GA_D_8009CED0) != 0u) {
            PE_StoreU8(rec + 9u, 1u);
        }
        for (list_i = 0; list_i < 5u; list_i++) {
            raw = PE_LoadU16(list);
            list += 2u;
            value = (int16_t)raw;
            if (value == -1)
                return;
            quot = pe_375e0_sdiv10((int32_t)value);
            dest = rec + 0x1Au + list_i * 6u;
            PE_StoreU8(dest, (uint8_t)(raw - (uint16_t)(quot * 10)));
            digit_i = 0u;
            while (((uint32_t)quot << 16) != 0u) {
                int32_t next;
                uint16_t cur;

                digit_i++;
                cur = (uint16_t)quot;
                next = pe_375e0_sdiv10(quot);
                PE_StoreU8(dest + digit_i,
                           (uint8_t)(cur - (uint16_t)(next * 10)));
                quot = next;
            }
            PE_StoreU8(rec + 0x1Fu + list_i * 6u, (uint8_t)(digit_i + 1u));
        }
        return;
    }
}
