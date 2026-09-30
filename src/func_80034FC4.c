/*
 * func_80034FC4 — VRAM 0x80034FC4 / file 0x257C4 / size 0x74 (29 words).
 * Initialises the 14-slot actor freelist at D_800BEA90: publishes the base
 * as head (gp+0x53C = D_8009D2AC), chains the +4 next-pointers of slots
 * 1..13 to the following slot (stride 0x280), clears the gp halfword/word
 * flags, zeroes 16 qwords at D_800A7624 and the D_800C0B14 gate.
 * D_800BEA94 is the real D_800BEA90+4 symbol; a byte-offset induction
 * variable keeps retail's 3-word lui/addu/sw %lo(byte-offset) store, and
 * D_800C0B14 is an incomplete array so it stays absolute under -G8.
 * Build: era -O2 -G8 + ERA_ASPSX_VER=2.30.
 */

extern short D_8009D2A6;
extern int D_8009D20C;
extern int D_8009D2AC;
extern int D_8009D254;
extern int D_800BEA90[];
extern char D_800BEA94[];
extern int D_800C0B14[];
extern char D_800A7624[];

void func_80034FC4(void) {
    unsigned int i;
    char *p;
    char *base;
    unsigned int off;

    i = 0;
    base = (char *)&D_800BEA90[0];
    p = base + 0x280;
    off = 0;
    D_8009D2A6 = 0;
    D_8009D20C = 0;
    D_8009D2AC = (int)base;
    do {
        *(int *)(D_800BEA94 + off) = (int)p;
        p += 0x280;
        i++;
        off += 0x280;
    } while (i < 0xDu);
    D_800C0B14[0] = 0;
    off = 0;
    do {
        *(int *)(D_800A7624 + off) = 0;
        off += 8;
    } while (off < 0x80);
    D_8009D254 = 0;
}
