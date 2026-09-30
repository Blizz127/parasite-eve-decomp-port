/*
 * func_80066800 — apply a 52-byte view record to the published GTE state.
 *
 * VRAM 0x80066800 / file 0x57000 / size 0x18C (99 words), inside 56438.s.
 *
 * Record = *D_800B1624 + <offset at container+0x1C> + index*52. Publishes the
 * record's leading halfword through *D_800BCFA8 and func_80079024, copies the
 * nine rotation halfwords (+0x02..+0x12) and the three translation words
 * (+0x14/+0x18/+0x1C) through *D_800BCFA4, records the byte index at
 * D_800BCFFD and sets bit 0x80 in D_800BCF88. Returns 0.
 *
 * era -O2 -G0. Levers:
 *  - Both pointer globals must be `*volatile`: retail performs two independent
 *    loads of D_800B1624 and RELOADS D_800BCFA4 before every store (a plain
 *    pointer makes cc1 hoist it and keep it in a register, losing ~19 words).
 *  - The nine halfword copies are written unrolled (a `for` loop is not
 *    unrolled by cc1 2.7.2). D_800BCFA4 is byte-addressed, so the three
 *    trailing word stores use explicit `*(int *)(...+0x14/0x18/0x1C)`.
 */
extern unsigned char *volatile D_800B1624;
extern int *D_800BCFA8;
extern unsigned char *volatile D_800BCFA4;
extern unsigned char D_800BCFFD;
extern unsigned int D_800BCF88;

extern void func_80079024();

int func_80066800(unsigned int index) {
    unsigned char *record = D_800B1624 + *(int *)(D_800B1624 + 0x1C) + index * 0x34;

    *(int *)D_800BCFA8 = *(unsigned short *)record;
    func_80079024(*(unsigned short *)record);
    *(unsigned short *)(D_800BCFA4 + 0x00) = *(unsigned short *)(record + 0x02);
    *(unsigned short *)(D_800BCFA4 + 0x02) = *(unsigned short *)(record + 0x04);
    *(unsigned short *)(D_800BCFA4 + 0x04) = *(unsigned short *)(record + 0x06);
    *(unsigned short *)(D_800BCFA4 + 0x06) = *(unsigned short *)(record + 0x08);
    *(unsigned short *)(D_800BCFA4 + 0x08) = *(unsigned short *)(record + 0x0A);
    *(unsigned short *)(D_800BCFA4 + 0x0A) = *(unsigned short *)(record + 0x0C);
    *(unsigned short *)(D_800BCFA4 + 0x0C) = *(unsigned short *)(record + 0x0E);
    *(unsigned short *)(D_800BCFA4 + 0x0E) = *(unsigned short *)(record + 0x10);
    *(unsigned short *)(D_800BCFA4 + 0x10) = *(unsigned short *)(record + 0x12);
    *(int *)(D_800BCFA4 + 0x14) = *(int *)(record + 0x14);
    *(int *)(D_800BCFA4 + 0x18) = *(int *)(record + 0x18);
    *(int *)(D_800BCFA4 + 0x1C) = *(int *)(record + 0x1C);
    D_800BCFFD = (unsigned char)index;
    D_800BCF88 |= 0x80;
    return 0;
}
