/*
 * Phase 6D-S — Boot state initializer with five counting loops.
 *
 * All stores now go through PE_StoreU8/U16/U32 into contiguous guest RAM
 * starting at 0x800B0CD8.  No host pointer arithmetic across separate arrays.
 *
 * Matching source: VRAM 0x8006A674 / file 0x5AE74 / size 0x260 (152 words).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B0CD8  0x800B0CD8u
#define GA_D_80094488  0x80094488u
#define GA_D_8009448C  0x8009448Cu

/* D_800B0CDC..D_800B0CEB are guest-RAM lvalue macros (psx_compat.h), so the
 * named stores below land in the same guest bytes as the PE_Store block
 * writes — one store, no split-brain. */

void func_8006A674(void)
{
    pe_addr_t base = GA_D_800B0CD8;
    uint32_t  cursor_fill_offset_command;
    uint32_t  count_or_record_cursor;
    uint32_t  shared_value_cursor;
    pe_addr_t down_count_cursor;   /* guest address */
    pe_addr_t tail_cursor;
    uint8_t   status_flags;

    count_or_record_cursor = 0;
    cursor_fill_offset_command = (uint32_t)base;

    PE_StoreU32(base, 3);
    D_800B0CDC = 10;
    D_800B0CDE = -1;
    D_800B0CE0 = 2;
    shared_value_cursor = (uint32_t)-1;
    D_800B0CE2 = 11;
    D_800B0CE1 = (signed char)shared_value_cursor;
    D_800B0CE3 = 0;
    D_800B0CE4 = (signed char)shared_value_cursor;
    D_800B0CE5 = 1;
    D_800B0CE6 = 0;
    D_800B0CE7 = 0;
    D_800B0CE8 = 0;
    D_800B0CE9 = 0;
    D_800B0CEA = 0;
    D_800B0CEB = 0;

    /* Loop 1: 0x31 iterations, zero u32 at base+0x14 + i*4 */
    do {
        PE_StoreU32(cursor_fill_offset_command + 0x14, 0);
        count_or_record_cursor++;
        cursor_fill_offset_command += 4;
    } while (count_or_record_cursor < 0x31UL);

    /* Loop 2: 2 iterations, fill -1 to pairs at base+0xDC..0xDF */
    count_or_record_cursor = 0;
    cursor_fill_offset_command = (uint32_t)-1;
    shared_value_cursor = (uint32_t)base;
    PE_StoreU8(base + 0xD9, 8);
    PE_StoreU8(base + 0xD8, 0);
    PE_StoreU8(base + 0xDB, (uint8_t)-1);
    PE_StoreU8(base + 0xDA, (uint8_t)-1);

    do {
        PE_StoreU8(shared_value_cursor + 0xDD, (uint8_t)cursor_fill_offset_command);
        PE_StoreU8(shared_value_cursor + 0xDC, (uint8_t)cursor_fill_offset_command);
        shared_value_cursor += 2;
    } while ((int32_t)++count_or_record_cursor < 2);

    /* Loop 3 preparation + loop */
    count_or_record_cursor = GA_D_80094488;
    cursor_fill_offset_command = 0;

    PE_StoreU8(base + 0xFF, 0x7F);
    PE_StoreU8(base + 0xFE, 0x7F);
    PE_StoreU8(base + 0xE0, 0x27);
    PE_StoreU8(base + 0xE1, 0x0D);
    PE_StoreU8(base + 0xE3, 1);
    shared_value_cursor = (uint32_t)-1;
    PE_StoreU8(base + 0xE6, 0x98);
    PE_StoreU8(base + 0xE2, 0);
    PE_StoreU16(base + 0xE4, (uint16_t)shared_value_cursor);
    PE_StoreU8(base + 0xE7, (uint8_t)-1);
    PE_StoreU16(base + 0xE8, (uint16_t)shared_value_cursor);
    PE_StoreU8(base + 0xEB, 0);
    PE_StoreU8(base + 0xEA, 0);

    do {
        PE_StoreU16(count_or_record_cursor + 6, 0);
        PE_StoreU16(GA_D_8009448C + cursor_fill_offset_command, 0);
        cursor_fill_offset_command += 8;
        count_or_record_cursor += 8;
    } while ((int32_t)cursor_fill_offset_command < 0x20);

    /* Loop 4 prep */
    cursor_fill_offset_command = 0xE1000440UL;
    count_or_record_cursor = 2;

    PE_StoreU8 (base + 0xF6, 0x30);
    PE_StoreU8 (base + 0xF7, 0x7F);
    PE_StoreU16(base + 0xF8, 0x100);
    PE_StoreU16(base + 0xFA, 0x800);
    PE_StoreU8 (base + 0x107, 3);
    PE_StoreU8 (base + 0x10B, 0x60);
    PE_StoreU16(base + 0x110, 0x140);
    PE_StoreU16(base + 0x112, 0xE0);
    PE_StoreU8 (base + 0x117, 1);

    status_flags = PE_LoadU8(base + 0x10B);
    shared_value_cursor = 0x20;
    PE_StoreU8(base + 0xED,  (uint8_t)shared_value_cursor);
    PE_StoreU8(base + 0x108, (uint8_t)shared_value_cursor);
    PE_StoreU8(base + 0x109, (uint8_t)shared_value_cursor);
    PE_StoreU8(base + 0x10A, (uint8_t)shared_value_cursor);

    shared_value_cursor = PE_LoadU32(base + 0x150);
    down_count_cursor = base + 8;

    PE_StoreU8 (base + 0xEC, 0);
    PE_StoreU8 (base + 0xEE, 0);
    PE_StoreU8 (base + 0xEF, 0);
    PE_StoreU8 (base + 0xF0, 0);
    PE_StoreU8 (base + 0xF1, 0);
    PE_StoreU8 (base + 0xF4, 0);
    PE_StoreU8 (base + 0xF2, 0);
    PE_StoreU8 (base + 0xF4, 0);
    PE_StoreU16(base + 0x10C, 0);
    PE_StoreU16(base + 0x10E, 0);
    PE_StoreU32(base + 0x118, (uint32_t)cursor_fill_offset_command);
    PE_StoreU32(base + 0x11C, 0);
    PE_StoreU32(base + 0x120, 0);
    PE_StoreU32(base + 0x124, 0);

    PE_StoreU8(base + 0x10B, status_flags | 2);
    PE_StoreU32(base + 0x128, (uint32_t)shared_value_cursor);
    PE_StoreU32(base + 0x12C, (uint32_t)shared_value_cursor + 0x1400);
    PE_StoreU32(base + 0x130, (uint32_t)shared_value_cursor + 0x2800);

    /* Loop 4: down-count 2→0, zero u32 */
    do {
        PE_StoreU32(down_count_cursor + 0x134, 0);
        count_or_record_cursor--;
        down_count_cursor -= 4;
    } while ((int32_t)count_or_record_cursor >= 0);

    /* Loop 5: down-count 1→0, zero u32 + final store */
    count_or_record_cursor = 1;
    tail_cursor = base + 4;
    do {
        PE_StoreU32(tail_cursor + 0x140, 0);
        count_or_record_cursor--;
        tail_cursor -= 4;
    } while ((int32_t)count_or_record_cursor >= 0);

    PE_StoreU32(base + 0x148, 0);
}
