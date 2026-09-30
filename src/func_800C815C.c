/* VRAM 0x800C815C / file 0xB895C / size 0x10C.
 *
 * The destination is typed as a struct pointer (`struct Sprite *`): with a
 * plain `unsigned char *` gcc 2.7.2 colours the reloaded D_800E279C global
 * into $v0 and emits an extra load-delay nop; the aggregate destination makes
 * it use $a0 and schedule the reload into the lhu slot, as in retail.
 */
extern unsigned char *D_8009D254;
extern unsigned char D_800E08E8[];
extern unsigned short D_800E2348;
extern unsigned short D_800E234A;
extern unsigned short D_800E234C;

struct Eight {
    unsigned int w[8];
};

struct Inner {
    unsigned char pad[0x260];
    struct Eight eight;
};

struct Outer {
    unsigned char pad[0x238];
    struct Inner *p;
};

struct Sprite {
    unsigned char pad0[4];
    unsigned short f4;
    unsigned short f6;
    unsigned short f8;
    unsigned short fA;
    unsigned short fC;
    unsigned char padE[2];
    struct Eight eight;
};

extern struct Outer *D_800E279C;
extern void func_80078C34(unsigned char *a0, unsigned char *a1, unsigned char *a2);

void func_800C815C(int a0, int a1, struct Sprite *a2) {
    struct Sprite *s0 = a2;
    unsigned char *p = *(unsigned char **)D_8009D254;
    unsigned char *q = *(unsigned char **)(p + 0x68);
    unsigned short n = *(unsigned short *)(q + 6);
    unsigned short buf[3];

    func_80078C34((unsigned char *)D_800E279C->p + 0x260,
                  D_800E08E8 + (short)(n - 1) * 8, (unsigned char *)buf);
    s0->f8 = D_800E2348 + buf[0];
    s0->fA = D_800E234A + buf[1];
    s0->fC = D_800E234C + buf[2];
    s0->eight = D_800E279C->p->eight;
    s0->f4 = 0x7F;
    s0->f6 = 0x224;
}
