/* Boot state initializer with five counting loops.
 * VRAM 0x8006A674 / file 0x5AE74 / size 0x260 (152 words).
 * Era path: gcc-2.7.2-psx -O1 -G0 -fschedule-insns2 + maspsx 2.21
 * --dont-expand-li + MASPSX_THREE_WORD_SYMBOL_STORE=1 (L3's
 * D_8009448C[cursor] store needs the 3-word symbol+register form).
 *
 * Flag requirements (all proven, 5EX Stage 1 measurement):
 *  - -O1 (not -O2): -O2's hardwired optimize>1 path hoists the shared -1
 *    constant and the independent-init sequence (45-word residual in three
 *    regions). -O1 materializes per-use like retail; the delta is not
 *    reachable by any -f flag combination at -O2.
 *  - -fschedule-insns2: at plain -O1, 21 words remain as pure 2-instruction
 *    order swaps — retail materializes each li/addiu BEFORE the adjacent
 *    store, plain -O1 emits it after. The post-allocation scheduler moves
 *    them into position exactly as retail's ccpsx did. 0/152 mismatch.
 *  - Register pins below are load-bearing: dropping all six degrades the
 *    match to 46 mismatched words at -O1 -fschedule-insns2.
 */
extern unsigned char D_800B0CD8[];

extern unsigned short D_800B0CDC;
extern signed short D_800B0CDE;
extern signed char D_800B0CE0;
extern signed char D_800B0CE1;
extern signed char D_800B0CE2;
extern signed char D_800B0CE3;
extern signed char D_800B0CE4;
extern signed char D_800B0CE5;
extern signed char D_800B0CE6;
extern signed char D_800B0CE7;
extern signed char D_800B0CE8;
extern signed char D_800B0CE9;
extern signed char D_800B0CEA;
extern signed char D_800B0CEB;

extern unsigned char D_80094488;
extern unsigned char D_8009448C[];

void func_8006A674(void) {
    /*
     * Retail deliberately reuses these three registers across sequential
     * phases: $a0 is the word cursor, -1 fill, record byte offset, then GPU
     * command; $a1 is the up-counter, record cursor, then down-counter; $a2
     * is the function-long struct base; $v1 carries the shared -1, the L2
     * cursor, the 0x20 fill, then the arena pointer.  Plain allocation splits
     * those roles and changes the instruction schedule from word 0 onward.
     * The bounded natural/order retry also put the preserved flags in $a3
     * and the final countdown cursor in $t0; their retail allocations are
     * independently $v0 and $a3, so only those two proven roles are pinned.
     */
    register unsigned long cursor_fill_offset_command asm("$4");
    register unsigned long count_or_record_cursor asm("$5");
    register unsigned char *base asm("$6");
    register unsigned long shared_value_cursor asm("$3");
    register unsigned char *down_count_cursor asm("$7");
    unsigned char *tail_cursor;
    register unsigned char status_flags asm("$2");

    count_or_record_cursor = 0;
    base = D_800B0CD8;
    cursor_fill_offset_command = (unsigned long)base;

    *(unsigned int *)base = 3;
    D_800B0CDC = 10;
    D_800B0CDE = -1;
    D_800B0CE0 = 2;
    shared_value_cursor = (unsigned long)-1;
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

    do {
        *(unsigned int *)((unsigned char *)cursor_fill_offset_command + 0x14) = 0;
        count_or_record_cursor++;
        cursor_fill_offset_command += 4;
    } while (count_or_record_cursor < 0x31UL);

    count_or_record_cursor = 0;
    cursor_fill_offset_command = (unsigned long)-1;
    shared_value_cursor = (unsigned long)base;
    base[0xD9] = 8;
    base[0xD8] = 0;
    *(signed char *)(base + 0xDB) = -1;
    *(signed char *)(base + 0xDA) = -1;

    do {
        *(signed char *)((unsigned char *)shared_value_cursor + 0xDD) =
            (signed char)cursor_fill_offset_command;
        *(signed char *)((unsigned char *)shared_value_cursor + 0xDC) =
            (signed char)cursor_fill_offset_command;
        shared_value_cursor += 2;
    } while ((signed long)++count_or_record_cursor < 2);

    count_or_record_cursor = (unsigned long)&D_80094488;
    cursor_fill_offset_command = 0;

    base[0xFF] = 0x7F;
    base[0xFE] = 0x7F;
    base[0xE0] = 0x27;
    base[0xE1] = 0x0D;
    base[0xE3] = 1;
    shared_value_cursor = (unsigned long)-1;
    base[0xE6] = 0x98;
    base[0xE2] = 0;
    *(signed short *)(base + 0xE4) = (signed short)shared_value_cursor;
    *(signed char *)(base + 0xE7) = -1;
    *(signed short *)(base + 0xE8) = (signed short)shared_value_cursor;
    base[0xEB] = 0;
    base[0xEA] = 0;

    do {
        *(unsigned short *)((unsigned char *)count_or_record_cursor + 6) = 0;
        *(unsigned short *)((unsigned char *)D_8009448C +
                            cursor_fill_offset_command) = 0;
        cursor_fill_offset_command += 8;
        count_or_record_cursor += 8;
    } while ((signed long)cursor_fill_offset_command < 0x20);

    cursor_fill_offset_command = 0xE1000440UL;
    count_or_record_cursor = 2;

    base[0xF6] = 0x30;
    base[0xF7] = 0x7F;
    *(unsigned short *)(base + 0xF8) = 0x100;
    *(unsigned short *)(base + 0xFA) = 0x800;
    base[0x107] = 3;
    base[0x10B] = 0x60;
    *(unsigned short *)(base + 0x110) = 0x140;
    *(unsigned short *)(base + 0x112) = 0xE0;
    base[0x117] = 1;

    status_flags = base[0x10B];
    shared_value_cursor = 0x20;
    base[0xED] = (unsigned char)shared_value_cursor;
    base[0x108] = (unsigned char)shared_value_cursor;
    base[0x109] = (unsigned char)shared_value_cursor;
    base[0x10A] = (unsigned char)shared_value_cursor;

    shared_value_cursor = *(unsigned int *)(base + 0x150);
    down_count_cursor = base + 8;

    base[0xEC] = 0;
    base[0xEE] = 0;
    base[0xEF] = 0;
    base[0xF0] = 0;
    base[0xF1] = 0;
    base[0xF4] = 0;
    base[0xF2] = 0;
    base[0xF4] = 0;
    *(unsigned short *)(base + 0x10C) = 0;
    *(unsigned short *)(base + 0x10E) = 0;
    *(unsigned int *)(base + 0x118) =
        (unsigned int)cursor_fill_offset_command;
    *(unsigned int *)(base + 0x11C) = 0;
    *(unsigned int *)(base + 0x120) = 0;
    *(unsigned int *)(base + 0x124) = 0;

    base[0x10B] = status_flags | 2;
    *(unsigned int *)(base + 0x128) = (unsigned int)shared_value_cursor;
    *(unsigned int *)(base + 0x12C) =
        (unsigned int)shared_value_cursor + 0x1400;
    *(unsigned int *)(base + 0x130) =
        (unsigned int)shared_value_cursor + 0x2800;

    do {
        *(unsigned int *)(down_count_cursor + 0x134) = 0;
        count_or_record_cursor--;
        down_count_cursor -= 4;
    } while ((signed long)count_or_record_cursor >= 0);

    count_or_record_cursor = 1;
    tail_cursor = base + 4;
    do {
        *(unsigned int *)(tail_cursor + 0x140) = 0;
        count_or_record_cursor--;
        tail_cursor -= 4;
    } while ((signed long)count_or_record_cursor >= 0);

    *(unsigned int *)(base + 0x148) = 0;
}
