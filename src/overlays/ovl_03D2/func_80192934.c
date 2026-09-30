typedef struct { short x, y, w, h; } RECT;
typedef struct { unsigned char minute, second, sector, track; } CdlLOC;
typedef struct {
    void *tabA[2];         /* 0x00 D_801D1464 */
    unsigned char curA;    /* 0x08 D_801D146C */
    unsigned char pad9[3];
    void *tabB[2];         /* 0x0C D_801D1470 */
    unsigned char curB;    /* 0x14 D_801D1478 */
    unsigned char pad15;
    RECT ent[2];           /* 0x16 D_801D147A */
    unsigned char idx;     /* 0x26 D_801D148A */
    unsigned char pad27;
    RECT r;                /* 0x28 D_801D148C */
    unsigned char flag;    /* 0x30 D_801D1494 */
} Mov;
extern Mov D_801D1464;
extern Mov D_801D1464_b asm("D_801D1464");
extern unsigned char D_801D146C;
extern unsigned char D_801D1478;
extern unsigned char D_801D148A;
extern short D_801D148C;
extern short D_801D148E;
extern short D_801D1490;
extern short D_801D1492;
extern volatile unsigned char D_801D1494;
typedef struct { unsigned char dba; signed char dbb; unsigned short dbc; } G;
extern G D_800B0DBAx asm("D_800B0DBA");
#define D_800B0DBA D_800B0DBAx.dba
extern signed char D_800B0DBB;
extern unsigned short D_800B0DBC;
extern unsigned char D_801D0DC0;
extern unsigned char D_801D0DBD;
extern unsigned char D_801D0DBE;
extern CdlLOC D_801D0DC4;
extern CdlLOC D_801D0DDC;
extern CdlLOC D_801D0DDC_b asm("D_801D0DDC");
extern CdlLOC D_801D0DDC_c asm("D_801D0DDC");
extern CdlLOC D_801D0DDC_d asm("D_801D0DDC");
extern int D_801D0DF8;
extern int D_8009CDDC;
extern void func_801918F8();
extern void func_8010BFA0();
extern void func_8010C01C();
extern int func_80191B64();
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

int func_80192934(void)
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
        p = &D_801D0DC0;
        if (*p == 2) {
            func_801918F8((signed char)(D_8009CDDC ^ 1), D_800B0DBB);
            *p = 0;
        }
        D_801D0DDC = D_801D0DC4;
        q = &D_801D1464.curA;
        func_8010BFA0(D_801D1464.tabA[*q], D_801D0DBE);
        tab = D_801D1464.tabA;
        func_8010C01C(D_801D1464.tabB[D_801D1478], (D_801D1490 * D_801D1492) / 2);
        for (;;) {
            n = 2000;
        loop:
            frame = func_80191B64(&D_801D1464_b);
            if (frame == 0) {
                if (--n != 0) {
                    goto loop;
                }
                r = -1;
            } else {
                D_801D146C ^= 1;
                D_800B0DBC++;
                pp = (void **)(D_801D146C * 4 + (int)tab);
                func_8010C89C(frame, *pp, D_801D0DF8);
                func_8007C394(frame);
                r = 0;
            }
            if (r != -1) break;
            func_8007C2A0(&D_801D0DDC_b);
            do {
                while (func_8007F72C() != 1 || func_8007F778() != 0) {
                }
                func_80080D5C(2, &D_801D0DDC_c, &cnt);
            } while (func_80081314(&D_801D0DDC_d, 0x1E0) == 0);
        }
        cnt = 0x800000;
        m = &D_801D1464;
        while (D_801D1494 == 0) {
            if (--cnt == 0) {
                m->flag = 1;
                D_801D148A ^= 1;
                e = (short *)((char *)m + D_801D148A * 8);
                D_801D148C = *(short *)((char *)e + 0x16);
                D_801D148E = *(short *)((char *)e + 0x18);
            }
        }
        D_801D1494 = 0;
        if (D_801D0DBD == 1) {
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
