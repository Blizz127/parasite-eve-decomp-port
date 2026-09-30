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
extern Mov D_801D1464_b asm("D_801228CC");
extern unsigned char D_801228D4;
extern unsigned char D_801228E0;
extern unsigned char D_801228F2;
extern short D_801228F4;
extern short D_801228F6;
extern short D_801228F8;
extern short D_801228FA;
extern unsigned char D_801228FC;
typedef struct { unsigned char dba; signed char dbb; unsigned short dbc; unsigned char dbe, dbf; } G;
extern G D_800B0DBAx asm("D_800B0DBA");
#define D_800B0DBA D_800B0DBAx.dba
extern signed char D_800B0DBB;
extern unsigned short D_800B0DBC;
extern unsigned char D_801223F8;
extern unsigned char D_801223F5;
extern unsigned char D_801223F6;
extern CdlLOC D_801223FC;
extern CdlLOC D_80122414;
extern CdlLOC D_801D0DC4_b asm("D_801223FC");
extern CdlLOC D_801D0DC4_c asm("D_801223FC");
extern CdlLOC D_801D0DDC_b asm("D_80122414");
extern CdlLOC D_801D0DDC_c asm("D_80122414");
extern CdlLOC D_801D0DDC_d asm("D_80122414");
extern int D_80122430;
extern int D_8009CDDC;
typedef struct { char *name; unsigned char b4; unsigned char pad5; short h6; short pad8; unsigned short wA; unsigned short wC; short padE; int pad10; } Ent;
extern Ent D_80122438[];
extern Ent *D_801227E4;
extern unsigned char D_800B0DBF;
extern unsigned char D_800B0DBE;
extern char D_80120FF4[];
extern char D_80120FFC[];
extern void *D_801228D0;
extern void *D_801D1464w asm("D_801228CC");
extern void *D_801228D8;
extern void *D_801228DC;
extern short D_801228E2;
extern short D_801228E4;
extern short D_801228EA;
extern short D_801228EC;
extern void *D_80122420;
extern void *D_80122424;
extern void *D_80122428;
extern void *D_8012242C;
extern int D_80122434;
extern void func_80121004();
extern void func_800719F4();
extern int func_8007F72C();
extern int func_8007F778();
extern int func_80081414();
extern void func_8010BE3C();
extern void func_8010C0D8();
extern void func_801214D4();
extern void func_8007A214();
extern void func_8007C304();
extern void func_80080D5C();
extern int func_80081314();
extern void func_800870F0();
extern void func_8010BD4C();
extern int func_80121270();
extern void func_8010C89C();
extern void func_8007C394();

int func_80121C04(int id)
{
    RECT rc;
    char buf[32];
    volatile int cnt;
    int res;
    void **tab;
    void **pp;
    Ent *e;
    Mov *m;
    register int frame asm("$17");
    register int cp asm("$3");
    short n;
    short r;

    cp = id;
    if ((unsigned short)id >= 0x2F) {
        return 0;
    }
    D_800B0DBAx.dbf = cp;
    D_801227E4 = &D_80122438[(short)id];
    D_800B0DBB = D_801227E4->b4;
    func_80121004(0, D_800B0DBB);
    func_80121004(1, D_800B0DBB);
    buf[0] = 0;
    if ((unsigned short)id < 0x15) {
        func_800719F4(buf, D_80120FF4);
    } else {
        func_800719F4(buf, D_80120FFC);
    }
    func_800719F4(buf, D_801227E4->name);
again:
    res = 0;
    if (func_8007F72C() == 1 && func_8007F778() == 0) {
        res = func_80081414(&D_801223FC, buf);
    }
    if (res == 0) goto again;
    if (res == -1) goto again;
    D_80122414 = D_801223FC;
    e = D_801227E4;
    rc.x = e->wA;
    rc.y = e->wC;
    D_801D1464_b.ent[0].y = rc.y + 0xF0;
    D_801D1464_b.tabA[0] = D_80122420;
    D_801D1464_b.curA = 0;
    D_801D1464_b.tabA[1] = D_80122424;
    D_801D1464_b.tabB[0] = D_80122428;
    D_801D1464_b.tabB[1] = D_8012242C;
    D_801D1464_b.curB = 0;
    D_801D1464_b.ent[0].x = rc.x;
    D_801D1464_b.idx = D_8009CDDC;
    D_801D1464_b.ent[1].y = rc.y;
    D_801D1464_b.ent[1].x = rc.x;
    m = &D_801228CC;
    D_801D1464_b.r.x = m->ent[D_801D1464_b.idx].x;
    D_801D1464_b.r.y = m->ent[D_801D1464_b.idx].y;
    if (D_800B0DBB != 0) {
        D_801228F8 = 0x18;
    } else {
        D_801228F8 = 0x10;
    }
    D_801228FC = 0;
    func_8010BE3C(0);
    func_8010C0D8(func_801214D4);
    func_8007A214(D_80122434, 0x40);
    func_8007C304(1, D_801227E4->h6, -1, 0, 0);
wait:
    while (func_8007F72C() != 1 || func_8007F778() != 0) {
    }
    func_80080D5C(2, &D_801D0DC4_b, &cnt);
    if (func_80081314(&D_801D0DC4_c, 0x1E0) == 0) goto wait;
    func_800870F0(D_800B0DBE);
    func_8010BD4C(D_80122430);
    tab = D_801228CC.tabA;
    for (;;) {
        n = 2000;
    loop:
        frame = func_80121270(&D_801D1464_b);
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
        D_801D0DDC_b = D_801223FC;
        do {
            while (func_8007F72C() != 1 || func_8007F778() != 0) {
            }
            func_80080D5C(2, &D_801D0DDC_c, &cnt);
        } while (func_80081314(&D_801D0DDC_d, 0x1E0) == 0);
    }
    D_801223F5 = 0;
    D_800B0DBC = 1;
    D_800B0DBA++;
    return 1;
}
