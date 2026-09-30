/*
 * func_800361F4 — VRAM 0x800361F4 / file 0x269F4 / size 0x60 (24 words).
 * Stores the actor pointer into the gp-resident current-actor global, then
 * walks three pointer slots at actor+0xA0; each non-null slot is published
 * to D_8009D300 and handed to func_80017018. D_8009D2F0 is gp-relative in
 * retail (0x580($gp)); D_8009D300 is lui/sw absolute, so it is declared as
 * an incomplete array to suppress small-data placement under -G8.
 * Build: era -O2 -G8 + ERA_ASPSX_VER=2.30 (nop_in_expansion off).
 */

extern int D_8009D2F0;
extern int D_8009D300[];
extern void func_80017018(void);

void func_800361F4(int actor) {
    unsigned int i;
    int *p;

    D_8009D2F0 = actor;
    i = 0;
    p = (int *)(actor + 0xA0);
    do {
        int v = *p;
        D_8009D300[0] = v;
        if (v != 0) {
            func_80017018();
        }
        i++;
        p++;
    } while (i < 3);
}
