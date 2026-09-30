/* VRAM 0x80040F80 / file 0x31780 / size 0x188. */
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
    int fd;
    unsigned char pad10[4];
    unsigned short h14;
    unsigned short h16;
    unsigned char pad18[4];
    CardEntry e[15];
} CardRec;

extern CardRec D_800A0ED4[2];
extern char D_8009EE70;
extern char *D_80092224;
extern char D_80010F4C[];
extern int D_800A185C;
extern int D_800A1854;
extern int D_800A1838;

extern void func_80042228(void);
extern void func_80072774(int);
extern int func_80072734(char *, int);
extern void func_800727A4(char *);
extern void func_8004D5CC(int);
extern void func_80062CE4(void);
extern void func_80071A84();

void func_80040F80(CardRec *c) {
    char buf[0x20];
    int i;
    int fd;
    int slot;

    if (D_800A185C != 0) {
        func_80042228();
        return;
    }
    if (c->fd >= 0) {
        func_80072774(c->fd);
        c->fd = -1;
    }
    if (c->f1 == 9) {
        slot = c - D_800A0ED4;
        func_80071A84(&D_8009EE70, D_80092224, D_800A0ED4 < c, c->e[c->f3].f29 + '0', c->f3 + 'A');
        func_80071A84(buf, D_80010F4C, slot, &D_8009EE70);
        for (i = 0; i < 10; i++) {
            fd = func_80072734(buf, 1);
            if (fd != -1) {
                break;
            }
        }
        if (i < 10) {
            func_80072774(fd);
            func_800727A4(buf);
        }
    }
    func_8004D5CC(c - D_800A0ED4);
    c->f1 = 0;
    c->f4 = 0;
    D_800A1854 = 0;
    D_800A1838 = 0;
    func_80062CE4();
}
