/*
 * func_8006E834 — post-mount image loader + display env setup (Boot Rung 1,
 * main's callee). Resets six status bytes, reads from the mounted \PE.IMG
 * (D_800B0DD8 = the mount result written by parked func_800698D4) via a
 * table-driven offset/size pair (D_80093164), polls func_800811E4 for
 * completion (retrying the whole read on -1), then brings up the display:
 * five-arg func_800749D8 (PROBABLE SetDefDispEnv — same struct passed to
 * PutDispEnv; 5th arg 0xF0 on the stack, first five-arg call in this
 * project), env byte +0x11 = 1, PutDispEnv.
 * ROM: asm/disc1/5B1E4.s @ file 0x5F034, 91 words (0x16C), frame 0x48.
 * era gcc-2.7.2-psx -O2 -G0.
 *
 * Register-binding note (phase 5FK, per standing policy 7): the natural form
 * leaves the poll result in $v0 while cc1 coalesces the equality tests onto
 * the $v1 backup and drops both restore copies (retail 0x5F0F8/0x5F114), a
 * 90-word output with the range test running in the wrong register. Retail
 * keeps a $v1 backup across the range test and restores it to $v0 for the
 * equality tests. The two `register ... asm(...)` locals below pin that
 * split ($v1 backup, $v0 test home); the two empty asm volatile "=r":"0"
 * barriers emit zero instructions — they only stop copy-propagation from
 * folding the copies away. The reorg pass then threads the idempotent merge
 * copy into the beqz delay slot, reproducing retail's two restores.
 */

extern signed char D_800B0DB2, D_800B0DB3, D_800B0DB4, D_800B0DB5, D_800B0DB6, D_800B0DB7;
extern unsigned int D_800B0CD8;     /* flags (and-masked) */
extern int D_800B0DD8;              /* mount base (written by 698D4) */
extern unsigned char *D_80011614;  /* read buffer pointer (type from func_8006A8D4.c) */
extern unsigned short D_80093164[]; /* offset/size pair table */
extern void func_80086FF8(void);
extern int  func_8006E6D4(int a0, int a1, unsigned char *a2, int a3);
extern int  func_800811E4(void *p);
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);
extern void func_80073A44(int a);   /* VSync (SDK) */
extern void func_80074D28(int a);   /* SetDispMask (SDK) */
extern void func_800749D8(void *env, int x, int y, int w, int h);  /* PROBABLE SetDefDispEnv */
extern void func_800755F0(void *env);  /* PutDispEnv (SDK) */

int func_8006E834(void) {
    char env[0x18];
    char local30[8];
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
retry:
    tbl = D_80093164;
    do {
        r = func_8006E6D4(D_800B0DD8 + tbl[0], 0, D_80011614, tbl[1] - tbl[0]);
    } while (r == -1);
    for (;;) {
        register int t asm("$3");            /* $v1: retail backup/range home */
        r = func_800811E4(local30);
        t = r;
        asm volatile("" : "=r"(t) : "0"(t));  /* zero-code: keep the $v1 backup */
        if ((unsigned)(t + 1) < 2) {
            D_800B0CD8 &= 0xFEFFBFFF;
        }
        {
            register int rt asm("$2");       /* $v0: retail equality-test home */
            rt = t;                          /* retail restore copy */
            asm volatile("" : "=r"(rt) : "0"(rt)); /* zero-code: decouple $v0 from $v1 */
            if (rt == 0) {
                break;
            }
            if (rt == -1) {
                goto retry;
            }
        }
    }
    func_80072714();
    func_800726C4();
    func_80072724();
    func_80073A44(0);
    func_80074D28(0);
    func_800749D8(env, 0, 0, 0x140, 0xF0);
    env[0x11] = 1;
    func_800755F0(env);
    return 0;
}
