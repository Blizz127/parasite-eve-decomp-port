/* VRAM 0x800425DC / file 0x32DDC / size 0x194. */
typedef struct {
    unsigned char sel;
    unsigned char pad[0x417];
} CardRec;

typedef struct {
    unsigned char state;
    unsigned char pad[0x417];
} CardState;

extern CardRec D_800A0ED4[2];
extern CardState D_800A0EDC[2];
extern int D_800A1860;
extern int D_800A1864;
extern int D_800A1868;

extern void func_800405A4(int);
extern void func_8004D9D8(void);
extern void func_8004CC50(int, int);
extern void func_8004D024(void (*)(void));
extern void func_80042910(void);
extern void func_80042928(void);
extern void func_8004DC84(void);
extern void func_8004CDAC(void);
extern void func_80041108(int);

void func_800425DC(void) {
    int i;
    int st;
    CardRec *c;

    func_800405A4(1);
    func_800405A4(0);
    if (D_800A1864 != 0) {
        st = D_800A0EDC[D_800A1860 - 1].state;
        if (st != 4 && st != 1) {
            D_800A1864 = -2;
        }
        D_800A1864 -= (D_800A1864 > 0);
        if (D_800A1864 <= 0) {
            func_8004D9D8();
            switch (D_800A1864) {
            case 0:
                func_8004CC50(0x52, 0);
                func_8004D024(func_80042928);
                D_800A1868 = 1;
                break;
            case -1:
                func_8004CC50(0x3C, 0);
                func_8004D024(func_80042910);
                D_800A1868 = 1;
                break;
            default:
                func_80042910();
                break;
            }
            D_800A1864 = 0;
        }
    }
    for (i = 1, c = &D_800A0ED4[1]; i >= 0; i--, c--) {
        if (D_800A1860 == i + 1 && !(c->sel & 1)) {
            func_8004DC84();
            func_8004CDAC();
            func_80042910();
        }
        func_80041108(i);
    }
}
