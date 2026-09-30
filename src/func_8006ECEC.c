/*
 * func_8006ECEC — boot CD-stream orchestrator (VRAM 0x8006ECEC, file
 * 0x5F4EC, 214 words / 0x358).
 *
 * Four table-driven read stages, each: issue a CD read through
 * func_8006E6D4 (retrying while it returns -1), then poll func_800811E4
 * until it returns 0; a -1 poll restarts the same stage, any other value
 * retries the poll.  Between stages the loaded arena pointer (fields
 * +0x15C / +0x16C of the D_800B0CD8 block, i.e. D_800B0E34 / D_800B0E44)
 * is walked as an int offset table calling func_800718D0, whose offset-0
 * entries are handler pointers.  Stage 2 runs only when D_800A77FC bit
 * 0x2000 is set; otherwise stage 3 runs instead.  Stage 4 finishes with
 * func_80074DC0 and the three display/teardown calls and returns 0.
 *
 * Field loads go through one pointer into the D_800B0CD8 block
 * (`lw $a2,0x15C($s2)`), while the flag clear is written on the scalar
 * global so cc1 emits its absolute lui/lw + lui/at/sw form.  The poll
 * result uses the proven func_8006E834 $v1-backup / $v0-restore split.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern unsigned char *D_800B0DD8;   /* mount base (written by 698D4) */
extern unsigned int D_800B0CD8;     /* flags word (and-masked) */
extern unsigned char *D_800B0E6C;   /* arena pointer argument to 6CDA4 */
extern unsigned char *D_80011614;   /* read buffer pointer */
extern int D_800A77FC;              /* status word tested with 0x2000 */
extern unsigned short D_80093168[]; /* offset/size pair table */

extern int func_80073A44(int a); /* VSync (SDK) */
extern void func_80074D28(int a); /* SetDispMask (SDK) */
extern void func_8006CDA4(int a, int b, int c, unsigned char *d, int e, int f);
extern int func_8006E6D4(int lba, int off, unsigned char *dest, int sectors);
extern int func_800811E4(void *p);
extern void func_800718D0(int target);
extern void func_80074DC0(int mode); /* DrawSync (SDK) */
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);

int func_8006ECEC(void) {
    unsigned int *flags = &D_800B0CD8;
    unsigned char *base = D_800B0DD8;
    unsigned short *tbl;
    unsigned char buf[8];
    int p;
    int i;
    int r;

    func_80073A44(0);
    func_80074D28(0);
    func_8006CDA4(1, 0xCC, 0, D_800B0E6C, 0x21, 1);

stageA:
    tbl = D_80093168;
    do {
        r = func_8006E6D4((int)(base + tbl[0]), 0, (unsigned char *)flags[0x57],
                          tbl[1] - tbl[0]);
    } while (r == -1);
    for (;;) {
        register int t asm("$3"); /* $v1: retail backup/range home */
        r = func_800811E4(buf);
        t = r;
        asm volatile("" : "=r"(t) : "0"(t));
        if ((unsigned)(t + 1) < 2) {
            D_800B0CD8 &= 0xFEFFBFFF;
        }
        {
            register int rt asm("$2"); /* $v0: retail equality-test home */
            rt = t;
            asm volatile("" : "=r"(rt) : "0"(rt));
            if (rt == 0) {
                break;
            }
            if (rt == -1) {
                goto stageA;
            }
        }
    }

    p = (int)flags[0x57];
    for (i = 0; i < 3; i++) {
        func_800718D0(p + *(int *)(p + (short)i * 4));
    }

    if (D_800A77FC & 0x2000) {
stageB:
        tbl = D_80093168 + 1;
        do {
            r = func_8006E6D4((int)(base + tbl[0]), 0,
                              (unsigned char *)flags[0x5B], tbl[1] - tbl[0]);
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
                    goto stageB;
                }
            }
        }
    } else {
stageC:
        tbl = D_80093168 + 2;
        do {
            r = func_8006E6D4((int)(base + tbl[0]), 0,
                              (unsigned char *)flags[0x5B], tbl[1] - tbl[0]);
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
                    goto stageC;
                }
            }
        }
    }

stageD:
    p = (int)flags[0x5B];
    for (i = 0; i < 0x106; i++) {
        func_800718D0(p + *(int *)(p + (short)i * 4));
    }
    func_80074DC0(0);

stageDread:
    tbl = D_80093168 + 3;
    do {
        r = func_8006E6D4((int)(D_800B0DD8 + tbl[0]), 0, D_80011614,
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
                goto stageDread;
            }
        }
    }

    func_80072714();
    func_800726C4();
    func_80072724();
    return 0;
}
