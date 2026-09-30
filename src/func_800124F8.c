/*
 * func_800124F8 — clear the boot-time work tables.
 *
 * VRAM 0x800124F8 / file 0x2CF8 / size 0x7C (31 words). Leaf, no frame.
 * The first table is 0x48 rows of 0x0B words at D_8009D310; the two
 * following loops clear D_8009DF70 and the small gp-relative state fields.
 */
extern unsigned int D_8009D310[];
extern unsigned int D_8009DF70[];
extern unsigned int D_8009D300;
extern unsigned short D_8009D308;
extern unsigned int D_8009CDFC;
extern unsigned int D_8009CE00;
extern unsigned int D_8009CE04;

void func_800124F8(void) {
    register unsigned int row asm("$5");
    register unsigned int row_offset asm("$6");
    register unsigned int base_addr asm("$7");
    unsigned int *table;

    row = 0;
    base_addr = (unsigned int)D_8009D310;
    row_offset = 0;
    D_8009D300 = 0;
    D_8009D308 = 0;
    D_8009CDFC = 0;
    for (row = 0; row < 0x48; row++) {
        unsigned int col;
        register unsigned int offset asm("$3");
        col = 0;
        offset = row_offset;
        for (col = 0; col < 0x0B; col++) {
            *(unsigned int *)(offset + base_addr) = 0;
            offset += 4;
        }
        row_offset += 0x2C;
    }
    D_8009CE00 = 0;
    row = 0;
    table = D_8009DF70;
    for (; row < 0x10; row++) {
        *table++ = 0;
    }
    D_8009CE04 = 0;
}
