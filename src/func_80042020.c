/* VRAM 0x80042020 / file 0x32820 / size 0x150. */
typedef struct {
    unsigned char state;
    unsigned char f1;
    unsigned char pad2[0x27];
    unsigned char f29;
    unsigned char pad2A[0x1A];
} CardEntry;

typedef struct {
    unsigned char sel;
    unsigned char f1;
    unsigned char f2;
    unsigned char f3;
    unsigned char f4;
    unsigned char f5;
    unsigned char f6;
    unsigned char f7;
    unsigned char f8;
    unsigned char f9;
    unsigned char count;
    unsigned char fB;
    unsigned char padC[8];
    unsigned short h14;
    unsigned short h16;
    unsigned char pad18[4];
    CardEntry e[15];
} CardRec;

extern CardRec D_800A0ED4[2];
extern char D_8009EE70;
extern char *D_80092224;
extern CardRec *D_800A1854;
extern int D_800A1858;

extern void func_80042798(void);
extern void func_8005C25C(void);
extern void func_80071A84(char *, char *, int, int, int);
extern void func_80040B80(char *);

void func_80042020(int slot, int n) {
    CardRec *c;
    CardEntry *e;
    int st;

    c = &D_800A0ED4[slot];
    st = c->sel;
    if (st != 1) {
        return;
    }
    e = &c->e[n];
    func_80042798();
    func_8005C25C();
    {
        int m = c->count;
        c->count = (e->state != st) ? m + 1 : m;
    }
    c->f3 = n;
    func_80071A84(&D_8009EE70, D_80092224, D_800A0ED4 < c, c->e[c->f3].f29 + '0', c->f3 + 'A');
    func_80040B80(&D_8009EE70);
    c->f5 = n;
    c->f6 = 0;
    e->f1 = 0;
    c->f1 = 1;
    c->f7 = 1;
    c->h14 = 0x2000;
    c->h16 = 10;
    D_800A1854 = c;
    D_800A1858 = 0x2000;
    c->fB = (e->state == 1) ? 0xB : 4;
    e->state = 1;
}
