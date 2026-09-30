/*
 * func_8006F044 — second boot image-load stage (VRAM 0x8006F044, file
 * 0x5F844, 120 words / 0x1E0).
 *
 * Same shape as func_8006E834 twice over: reset the six status bytes
 * D_800B0DB2..D_800B0DB7, clear the low nibble of the D_800B0CD8 flags,
 * run func_80086FF8, then for each of two tables (D_8009315E with
 * destination D_8001160C, then D_80093166 with destination D_80011610)
 * issue func_8006E6D4 (retrying while -1), poll func_800811E4 until 0
 * (-1 restarts the read), and finish the stage with the
 * func_80072714/726C4/72724 trio.  Returns 0.
 *
 * Poll uses the func_8006E834 $v1-backup / $v0-restore split.
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern signed char D_800B0DB2, D_800B0DB3, D_800B0DB4, D_800B0DB5, D_800B0DB6,
    D_800B0DB7;
extern unsigned int D_800B0CD8;     /* flags (and-masked) */
extern int D_800B0DD8;              /* mount base (written by 698D4) */
extern unsigned char *D_8001160C;   /* stage-1 read buffer pointer */
extern unsigned char *D_80011610;   /* stage-2 read buffer pointer */
extern unsigned short D_8009315E[]; /* stage-1 offset/size pair table */
extern unsigned short D_80093166[]; /* stage-2 offset/size pair table */

extern int func_8006E6D4(int lba, int off, unsigned char *dest, int sectors);
extern int func_800811E4(void *p);
extern void func_80086FF8(void);
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);

int func_8006F044(void) {
    char buf[8];
    unsigned short *tbl;
    int r;

    D_800B0DB5 = -1;
    D_800B0DB4 = -1;
    D_800B0DB7 = -1;
    D_800B0DB6 = -1;
    D_800B0DB3 = -1;
    D_800B0DB2 = -1;
    D_800B0CD8 &= ~0xF0;
    func_80086FF8();

retry1:
    tbl = D_8009315E;
    do {
        r = func_8006E6D4(D_800B0DD8 + tbl[0], 0, D_8001160C,
                          tbl[1] - tbl[0]);
    } while (r == -1);
    for (;;) {
        register int t asm("$3");
        r = func_800811E4(buf);
        t = r;
        asm volatile("" : "=r"(t) : "0"(t));
        if ((unsigned)(t + 1) < 2) {
            D_800B0CD8 &= 0xFEFFBFFF;
        }
        {
            register int rt asm("$2");
            rt = t;
            asm volatile("" : "=r"(rt) : "0"(rt));
            if (rt == 0) {
                break;
            }
            if (rt == -1) {
                goto retry1;
            }
        }
    }
    func_80072714();
    func_800726C4();
    func_80072724();

retry2:
    tbl = D_80093166;
    do {
        r = func_8006E6D4(D_800B0DD8 + tbl[0], 0, D_80011610,
                          tbl[1] - tbl[0]);
    } while (r == -1);
    for (;;) {
        register int t asm("$3");
        r = func_800811E4(buf);
        t = r;
        asm volatile("" : "=r"(t) : "0"(t));
        if ((unsigned)(t + 1) < 2) {
            D_800B0CD8 &= 0xFEFFBFFF;
        }
        {
            register int rt asm("$2");
            rt = t;
            asm volatile("" : "=r"(rt) : "0"(rt));
            if (rt == 0) {
                break;
            }
            if (rt == -1) {
                goto retry2;
            }
        }
    }
    func_80072714();
    func_800726C4();
    func_80072724();
    return 0;
}
