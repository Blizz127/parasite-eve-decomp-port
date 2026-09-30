typedef struct { short x, y, w, h; } RECT;
typedef struct { unsigned char minute, second, sector, track; } CdlLOC;
typedef struct {
    void *tabA[2];         /* 0x00 D_801228CC */
    unsigned char curA;    /* 0x08 D_801228D4 */
    unsigned char pad9[3];
    void *tabB[2];         /* 0x0C D_801228D8 */
    unsigned char curB;    /* 0x14 D_801228E0 */
    unsigned char pad15;
    RECT ent[2];           /* 0x16 D_801228E2 */
    unsigned char idx;     /* 0x26 D_801228F2 */
    unsigned char pad27;
    RECT r;                /* 0x28 D_801228F4 */
    unsigned char flag;    /* 0x30 D_801228FC */
} Mov;
extern Mov D_801228CC;
extern Mov D_801228CC_b asm("D_801228CC");
extern unsigned char D_801228D4;
extern unsigned char D_801228E0;
extern unsigned char D_801228F2;
extern short D_801228F4;
extern short D_801228F6;
extern short D_801228F8;
extern short D_801228FA;
extern volatile unsigned char D_801228FC;
typedef struct { unsigned char dba; signed char dbb; unsigned short dbc; } G;
extern G D_800B0DBAx asm("D_800B0DBA");
#define D_800B0DBA D_800B0DBAx.dba
extern signed char D_800B0DBB;
extern unsigned short D_800B0DBC;
extern unsigned char D_801223F8;
extern unsigned char D_801223F5;
extern unsigned char D_801223F6;
extern CdlLOC D_801223FC;
extern CdlLOC D_80122414;
extern CdlLOC D_80122414_b asm("D_80122414");
extern CdlLOC D_80122414_c asm("D_80122414");
extern CdlLOC D_80122414_d asm("D_80122414");
extern int D_80122430;
extern int D_8009CDDC;
extern void func_80121004();
extern void func_8010BFA0();
extern void func_8010C01C();
extern int func_80121270();
extern void func_8010C89C();
extern void func_8007C394();
extern void func_8007C2A0();
extern int func_8007F72C();
extern int func_8007F778();
extern void func_80080D5C();
extern int func_80081314();
extern void func_8010C0D8();
extern void func_8007A2A4();
extern void func_80080DC4();

int func_80122040(void)
{
    volatile int cnt;
    int end;
    unsigned char *p;
    void **tab;
    unsigned char *q;
    Mov *m;
    short *e;
    int k;
    void **pp;
    register int frame asm("$17");
    short n;
    short r;

    if (D_800B0DBA >= 2) {
        end = 0;
        p = &D_801223F8;
        if (*p == 2) {
            func_80121004((signed char)(D_8009CDDC ^ 1), D_800B0DBB);
            *p = 0;
        }
        D_80122414 = D_801223FC;
        q = &D_801228CC.curA;
        func_8010BFA0(D_801228CC.tabA[*q], D_801223F6);
        tab = D_801228CC.tabA;
        func_8010C01C(D_801228CC.tabB[D_801228E0], (D_801228F8 * D_801228FA) / 2);
        for (;;) {
            n = 2000;
        loop:
            frame = func_80121270(&D_801228CC_b);
            if (frame == 0) {
                if (--n != 0) {
                    goto loop;
                }
                r = -1;
            } else {
                D_801228D4 ^= 1;
                D_800B0DBC++;
                pp = (void **)(D_801228D4 * 4 + (int)tab);
                func_8010C89C(frame, *pp, D_80122430);
                func_8007C394(frame);
                r = 0;
            }
            if (r != -1) break;
            func_8007C2A0(&D_80122414_b);
            do {
                while (func_8007F72C() != 1 || func_8007F778() != 0) {
                }
                func_80080D5C(2, &D_80122414_c, &cnt);
            } while (func_80081314(&D_80122414_d, 0x1E0) == 0);
        }
        cnt = 0x800000;
        m = &D_801228CC;
        while (D_801228FC == 0) {
            if (--cnt == 0) {
                m->flag = 1;
                D_801228F2 ^= 1;
                e = (short *)((char *)m + D_801228F2 * 8);
                D_801228F4 = *(short *)((char *)e + 0x16);
                D_801228F6 = *(short *)((char *)e + 0x18);
            }
        }
        D_801228FC = 0;
        if (D_801223F5 == 1) {
            end = 1;
        }
        k = end;
        asm("" : "=r"(k) : "0"(k));
        if (k == 0) {
            return 1;
        }
        D_800B0DBA--;
        func_8010C0D8(0);
        func_8007A2A4();
        func_80080DC4(9, 0, 0);
    }
    return 0;
}
