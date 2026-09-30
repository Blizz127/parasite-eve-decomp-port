/* func_800536B8 — VRAM 0x800536B8, file 0x43EB8, size 0x174.
 *
 * Two-stage record lookup on a signed 16-bit index table.  Stage 1 resolves a
 * secondary record pointer (a1) from D_8009D048[arg0]: an inline record when
 * the id is in 0x100..0x17F (base D_800BEEAC + id*0x20), otherwise an accessor
 * result (func_8005DC9C) for ids below 0x100 and for 0x200..0x208 (base
 * D_8009D03C).  Stage 2, only while arg0 is inside D_8009D050, resolves the
 * primary record (v0) the same way (func_8005DB44 for the sub-0x100 range,
 * absolute table D_8009DE64 for 0x200..0x208), then hands both records to
 * func_800534E4.  Returns 0 when out of range.
 *
 * era_o2_g8: D_8009D03C (0x2CC), the D_8009D048 pointer (0x2D8) and
 * D_8009D050 (0x2E0) are gp-relative scalars; D_800BEEAC and D_8009DE64 are
 * absolute (incomplete arrays).  Home pins reproduce the original allocator's
 * choices: $17 carries the stage-1 result across the stage-2 calls, $5 carries
 * the private copy of the stage-2 id, and $3/$2 hold the id and shift scratch.
 * The copy of the stage-2 id into $5 must be issued before the 0x100 range
 * test so the scheduler drops it into that branch's delay slot.
 */
extern short *D_8009D048;
extern int D_8009D03C;
extern int D_8009D050;
extern char D_800BEEAC[];
extern char D_8009DE64[];

int func_8005DC9C(int);
int func_8005DB44(int, int);
int func_800534E4(void *, void *);

int func_800536B8(int arg0) {
    register void *a1 asm("$17");
    register void *t asm("$5");
    register int v1 asm("$3");
    register int sh1 asm("$2");
    register int sh asm("$3");
    char *p;
    void *v0;

    v1 = D_8009D048[arg0];
    t = 0;
    if ((unsigned int)(v1 - 0x100) < 0x80U) {
        sh1 = v1 << 5;
        p = D_800BEEAC + sh1;
        if (p[5] & 0x10) {
            t = D_800BEEAC + 0x31F8;
            if (p[6] == 9) t = D_800BEEAC + 0x3208;
        } else { t = (void *)func_8005DC9C(p[4] - 1); }
    } else {
        int a0 = v1 - 1;
        if ((unsigned int)a0 < 0xFFU) { t = (void *)func_8005DC9C(a0); }
        else if ((unsigned int)(v1 - 0x200) < 9U) { t = (void *)func_8005DC9C(D_8009D03C + v1 - 0x201); }
    }
    a1 = t;

    if (arg0 >= 0 && arg0 < D_8009D050) {
        v1 = D_8009D048[arg0];
        t = (void *)(int)v1;
        if ((unsigned int)(v1 - 0x100) < 0x80U) { v0 = D_800BEEAC + (v1 << 5); }
        else {
            int a0 = v1 - 1;
            if ((unsigned int)a0 < 0xFFU) { v0 = (void *)func_8005DB44(a0, (int)t); }
            else if ((unsigned int)((int)t - 0x200) < 9U) { sh = (int)t << 5; v0 = D_8009DE64 + sh; }
            else { v0 = 0; }
        }
    } else { v0 = 0; }
    if (v0 != 0) v0 = (void *)func_800534E4(v0, a1);
    return (int)v0;
}
