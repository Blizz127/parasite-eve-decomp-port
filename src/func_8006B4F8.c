/*
 * func_8006B4F8 - vram 0x8006B4F8, file 0x5BCF8, size 0x870 (540 words), frame 0x80.
 * Room load: resolve the room file index (func_8006E454), stream three blocks
 * (func_8006E6A8 + func_8006E7E8 poll), relocating the previous block while
 * the next one is in flight, then fix up the object/pointer tables.
 * era: cc1 2.7.2 -O2 -G8 + MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800B0DD8,D_800942E0.
 * Levers (mismatch ladder in docs/evidence/func-8006B4F8/REPORT.md):
 *  - file table as a struct array with a byte/word union for the packed
 *    lengths keeps the index in the memory operand (no hoisted giv);
 *  - one pointer variable per block group (tbl: 0xC/0x10/0x20/0x4 tables,
 *    ent: 0x2C/0x30/0x24 walkers, q: the 0x28 relocations) - one pseudo
 *    per variable reproduces retail's per-group register;
 *  - asm("" : : "r"(r), "r"(r)) after each poll raises r's global-alloc
 *    priority so r gets $s1 and i $s2 (a $17 pin makes cse substitute s1
 *    for the known-zero r in later `= 0` stores);
 *  - the 0xC table loop as `if (n) do {} while` (every folded `for` entry
 *    test leaves a dead 8-byte stack slot; retail has six);
 *  - `register ... row asm("$2")` for the 0x10 row pointer stops combine
 *    folding the +0x1C0 into the store displacement;
 *  - `t <<= 2; t + (unsigned int)D_800942E0` and the `k = t` copy for the
 *    handler-slot selection; `t = H(e, 0xA); v = t;` for the 0x2C copy.
 */
extern unsigned char D_800B0CD8[];
extern int D_800B0DD8;
extern unsigned char D_8009CDC8;
typedef struct {
    int lba;
    union {
        unsigned int w;
        unsigned char n0;
    } info;
} RoomFile;
extern RoomFile D_80093378[];
extern void **D_800942E0;
extern void *D_800E1044[];
extern unsigned short D_8009448C[4][4];

extern void func_80071A44(void *p, int c, int n);
extern void func_8006E2D0(void *a0, int a1);
extern int func_8006E454(void *a0);
extern int func_8006E6A8(int lba, unsigned char *dest, int sectors);
extern int func_8006E7E8(void);
extern void func_8006E1C0(unsigned char *a0, unsigned char *a1);
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);
extern void *func_80012574(unsigned char *p);
extern int func_8006CDA4(int, int, int, unsigned char *, int, int);

#define W(p, o) (*(unsigned int *)((unsigned char *)(p) + (o)))
#define H(p, o) (*(unsigned short *)((unsigned char *)(p) + (o)))
#define PTR(p, o) (*(unsigned char **)((unsigned char *)(p) + (o)))

int func_8006B4F8(int arg)
{
    unsigned char buf[8];
    unsigned char *st;
    int base;
    int idx;
    int r;
    int done;
    unsigned int i;
    unsigned int flags;
    unsigned char *p;
    unsigned char *h;
    unsigned char *q;
    unsigned char *tbl;
    unsigned char *ent;

    buf[0] = D_8009CDC8;
    func_80071A44(buf + 1, 0, 6);
    flags = 0;
    st = D_800B0CD8;
    base = D_800B0DD8;
    func_8006E2D0(buf, arg);
restart:
    idx = func_8006E454(buf) - 1;
    while (func_8006E6A8(base + D_80093378[idx].lba, PTR(st, 0x194),
                         D_80093378[idx].info.n0) == -1)
        ;
    r = 1;
    do {
        if (r == -1)
            goto restart;
        r = func_8006E7E8();
        asm("" : : "r"(r), "r"(r));
    } while (r != 0);

    done = 0;
read2:
    while (func_8006E6A8(base + D_80093378[idx].lba + D_80093378[idx].info.n0,
                         PTR(st, 0x168), (D_80093378[idx].info.w >> 8) & 0xFFF) == -1)
        ;
    r = 1;
    do {
        if (!done) {
            p = PTR(st, 0x194);
            h = p + W(p, 4);
            q = p + (W(h, 0x28) & 0x3FFFFF);
            for (i = 0; i < W(h, 0x28) >> 22; i++) {
                func_8006E1C0(q + i * 0x14, p);
            }
            done = 1;
        }
        if (r == -1)
            goto read2;
        r = func_8006E7E8();
        asm("" : : "r"(r), "r"(r));
    } while (r != 0);

    done = 0;
read3:
    while (func_8006E6A8(base + D_80093378[idx].lba + D_80093378[idx].info.n0
                             + ((D_80093378[idx].info.w >> 8) & 0xFFF),
                         PTR(st, 0x18C), D_80093378[idx].info.w >> 20) == -1)
        ;
    r = 1;
    do {
        if (!done) {
            p = PTR(st, 0x168);
            h = p + W(p, 4);
            q = p + (W(h, 0x28) & 0x3FFFFF);
            for (i = 0; i < W(h, 0x28) >> 22; i++) {
                func_8006E1C0(q + i * 0x14, p);
            }
            done = 1;
        }
        if (r == -1)
            goto read3;
        r = func_8006E7E8();
        asm("" : : "r"(r), "r"(r));
    } while (r != 0);

    func_80072714();
    func_800726C4();
    func_80072724();

    p = PTR(st, 0x18C);
    h = p + W(p, 4);
    st[0xA] = h[1];
    st[0x8] = h[3];
    tbl = p + (W(h, 0xC) & 0x3FFFFF);
    {
        unsigned char *e = tbl;
        i = 0;
        if (W(h, 0xC) >> 22) do {
            PTR(st, 0x198 + e[i * 0xC + 7] * 4) = p + (W(e + i * 0xC, 4) & 0xFFFFFF);
        } while (++i < W(h, 0xC) >> 22);
    }
    tbl = p + (W(h, 0x10) & 0x3FFFFF);
    {
        unsigned char *e = tbl;
        for (i = 0; i < W(h, 0x10) >> 22; ) {
            register unsigned char **row asm("$2") = (unsigned char **)(e[i * 0xC + 0xB] * 0xC0 + (int)st + 0x1C0);
            row[e[i * 0xC + 7]] = p + (W(e + i * 0xC, 4) & 0xFFFFFF);
            i++;
        }
    }
    PTR(st, 0x944) = func_80012574(p + (W(p + (W(h, 0x14) & 0x3FFFFF), 4) & 0xFFFFFF));
    PTR(st, 0x948) = p + (W(p + (W(h, 0x18) & 0x3FFFFF), 4) & 0xFFFFFF);
    PTR(st, 0x94C) = p + (W(p + (W(h, 0x1C) & 0x3FFFFF), 4) & 0xFFFFFF);
    tbl = p + (W(h, 0x20) & 0x3FFFFF);
    {
        unsigned char *e = tbl;
        for (i = 0; i < W(h, 0x20) >> 22; i++) {
            PTR(st, 0x950 + e[i * 8 + 7] * 4) = p + (W(e + i * 8, 4) & 0xFFFFFF);
        }
    }
    tbl = p + (W(h, 0x4) & 0x3FFFFF);
    {
        unsigned char *e = tbl;
        for (i = 0; i < W(h, 0x4) >> 22; i++) {
            unsigned int t = e[i * 0xC + 7];
            unsigned int k = t;
            void **slot;
            if (t - 8 < 0x4D) {
                t <<= 2;
                slot = (void **)(t + (unsigned int)D_800942E0);
            } else if (k >= 0x55) {
                unsigned int j = k - 0x55;
                slot = &D_800E1044[j];
            } else {
                continue;
            }
            if (*slot == 0) {
                *slot = *(void **)(e + i * 0xC + 8);
            }
        }
    }
    if (W(h, 0x2C) & 0xFFC00000) {
        ent = p + (W(h, 0x2C) & 0x3FFFFF);
        for (i = 0; i < W(h, 0x2C) >> 22; i++) {
            unsigned char *e = ent + i * 0xC;
            if (e[3] & 0x20) {
                unsigned int t = H(e, 0xA);
                unsigned short v = t;
                if (t & 1) {
                    D_8009448C[1][e[7]] = v;
                    D_8009448C[3][e[7]] = H(e, 0xA);
                } else {
                    D_8009448C[0][e[7]] = v;
                    D_8009448C[2][e[7]] = H(e, 0xA);
                }
            }
        }
    }
    if (W(h, 0x30) & 0xFFC00000) {
        ent = p + (W(h, 0x30) & 0x3FFFFF);
        for (i = 0; i < W(h, 0x30) >> 22; i++) {
            unsigned char *e = ent + i * 0xC;
            if (e[3] & 0x10) {
                st[0xE0] = H(e, 0xA);
                st[0xE1] = e[8];
                PTR(st, 0x124) = PTR(st, 0x18C) + (W(e, 4) & 0xFFFFFF);
            }
        }
    }
    if (W(h, 0x24) & 0xFFC00000) {
        ent = p + (W(h, 0x24) & 0x3FFFFF);
        for (i = 0; i < W(h, 0x24) >> 22; i++) {
            unsigned char *e = ent + i * 8;
            if (e[3] & 0x10) {
                continue;
            }
            if (e[3] & 0x20) {
                if (H(e, 6) == 0) {
                    flags |= 0x10;
                    if (st[0xEA] != H(e, 4)) {
                        flags |= 1;
                        st[0xEA] = e[4];
                    }
                } else if (H(e, 6) == 1) {
                    flags |= 0x20;
                    if (st[0xEB] != H(e, 4)) {
                        flags |= 2;
                        st[0xEB] = e[4];
                    }
                }
            } else {
                unsigned short v = H(e, 4);
                flags |= 0x40;
                if (*(short *)(st + 0xE8) != v) {
                    flags |= 4;
                    *(short *)(st + 0xE8) = v;
                }
            }
        }
    }
    done = 1;
    if ((flags & 1) && st[0xEA] != 0) {
        func_8006CDA4(2, st[0xEA], 0, PTR(st, 0x194), 0x21, done);
    } else if (!(flags & 0x10)) {
        st[0xEA] = 0;
    }
    if ((flags & 2) && st[0xEB] != 0) {
        func_8006CDA4(2, st[0xEB], 1, PTR(st, 0x194), 0x21, 1);
    } else if (!(flags & 0x20)) {
        st[0xEB] = 0;
    }
    if ((flags & 4) && *(short *)(st + 0xE8) != -1) {
        func_8006CDA4(1, *(short *)(st + 0xE8), 0, PTR(st, 0x194), 0x21, 1);
    } else if (!(flags & 0x40)) {
        *(short *)(st + 0xE8) = -1;
    }
    return 0;
}
