/* func_8008FC78 — VRAM 0x8008FC78, size 0x3C, file 0x80478-0x804B4.
 *
 * Cursor post-increment: reads the byte at the cached cursor into +0x82
 * (rewriting 0 as 0x100), clears +0xE6 and +0x80, then writes 1 to +0x84
 * and returns that same value.
 *
 * era_o1_g0. At -O2 the scheduler hoists the +0x84 store ahead of the two
 * zero stores, so the constant is materialised twice and the +0x80 store
 * (not the +0x84 store) fills the jr delay slot. -O1 keeps the retail
 * order: li v0,1 / sh +0xE6 / sh +0x80 / jr / [delay] sh v0,+0x84.
 */
int func_8008FC78(unsigned char *obj) {
    unsigned char *cursor;
    unsigned int v;
    int r;

    cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    v = *cursor;
    *(unsigned short *)(obj + 0x82) = (unsigned short)v;
    if (v == 0)
        *(unsigned short *)(obj + 0x82) = 0x100;
    r = 1;
    *(unsigned short *)(obj + 0xE6) = 0;
    *(unsigned short *)(obj + 0x80) = 0;
    *(unsigned short *)(obj + 0x84) = r;
    return r;
}
