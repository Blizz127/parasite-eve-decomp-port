/*
 * func_80062F9C — free-list pool initializer (Phase 5FI candidate, 20 words).
 *
 * Threads the 24-slot pool at D_800A22E0 (stride 0x90, 0x90*0x18 = 0xD80)
 * into a singly-linked free list: every slot's +0x00 word points at the next
 * slot, the final slot (D_800A2FD0 = D_800A22E0 + 23*0x90) is nulled by the
 * explicit overwrite, and three gp globals are seeded — D_8009D158 (+0x3E8)
 * to the pool base, D_8009D15C (+0x3EC) and D_8009D154 (+0x3E4) to zero.
 *
 * ROM: asm/disc1/5373C.s @ file 0x5379C, 20 words (0x50 bytes), no frame.
 */

extern unsigned char D_800A22E0[];
extern unsigned int *D_8009D158;
extern unsigned int D_8009D15C;
extern unsigned int D_8009D154;

void func_80062F9C(void) {
    unsigned char *p = D_800A22E0;
    unsigned char *end = D_800A22E0 + 0xD80;

    while (p < end) {
        *(unsigned int *)p = (unsigned int)(p + 0x90);
        p += 0x90;
    }
    *(unsigned int *)0x800A2FD0 = 0;
    D_8009D158 = (unsigned int *)D_800A22E0;
    D_8009D15C = 0;
    D_8009D154 = 0;
}
