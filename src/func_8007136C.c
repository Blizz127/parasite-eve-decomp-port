typedef struct {
    int x;
    int y;
    int z;
} VEC;

extern unsigned int D_8009D1A0;
extern unsigned short D_800BE9A0;
extern unsigned char D_800BE9A6;
extern unsigned char D_800BE9A7;
extern unsigned short D_800BD020;
extern unsigned short D_800BD022;
extern unsigned int D_8009D26C;
extern int **D_8009D254;
extern unsigned int D_8009D2E8;
extern int D_800BD000[];
extern void func_8001A680(unsigned char *, int);
extern int func_8003708C(int, int);
extern int func_80079FB4(int, int);
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern void func_80078934(int *, VEC *, int *);

void func_8007136C(unsigned char *e, int state)
{
    VEC v;
    int sp;
    int d;
    register int ang asm("$17");
    int r;
    int a;
    unsigned int f;
    unsigned int g;

    sp = 0x168000;
    if (D_8009D1A0 & 2) {
        g = *(unsigned int *)(*(unsigned char **)e + 0x4C);
        if ((g & 0xC0) == 0x40) {
            sp = 0xB4000;
        } else if (g & 0x100) {
            sp = 0x21C000;
        }
        if (*(int *)state != (*(unsigned char **)e)[0x13]) {
            func_8001A680(e, (*(unsigned char **)e)[0x13]);
            *(int *)state = (*(unsigned char **)e)[0x13];
        }
    } else if (*(int *)state != 0x17) {
        func_8001A680(e, 0x17);
        *(int *)state = 0x17;
    }
    sp = func_8003708C(sp, *(int *)(e + 0x20));
    sp = func_8003708C(sp, *(unsigned short *)(e + 0x26) << 4);
    if ((D_800BE9A0 & 0xF000) == 0x7000) {
        ang = func_80079FB4(D_800BE9A7 - 0x80, D_800BE9A6 - 0x80) - 0x400;
        asm("" : "=r"(ang) : "0"(ang));
        asm("" : "=r"(ang) : "0"(ang));
        if (ang < 0) {
            ang += 0x1000;
        }
        *(unsigned short *)(e + 0x3A) = D_800BD022 + ang;
        d = func_80077CF4(ang);
        asm volatile("");
        sp = -sp;
        v.x = func_8003708C(sp, d << 4);
        v.y = 0;
        v.z = func_8003708C(sp, func_80077DC4(ang) << 4);
    } else {
        d = func_8003708C(sp, 0xB504);
        f = D_8009D26C;
        a = D_800BD020;
        *(short *)(e + 0x3A) = a;
        if (f & 8) {
            if (f & 0x40) {
                *(short *)(e + 0x3A) = a + 0x600;
                v.x = -d;
                v.y = 0;
                v.z = d;
            } else if (f & 0x10) {
                *(short *)(e + 0x3A) = a + 0xA00;
                v.x = d;
                v.y = 0;
                v.z = d;
            } else {
                *(short *)(e + 0x3A) = a + 0x800;
                v.x = 0;
                v.y = 0;
                v.z = sp;
            }
        } else if (f & 0x20) {
            if (f & 0x40) {
                *(short *)(e + 0x3A) = a + 0x200;
                v.x = -d;
                v.y = 0;
                v.z = -d;
            } else if (f & 0x10) {
                *(short *)(e + 0x3A) = a + 0xE00;
                v.x = d;
                v.y = 0;
                v.z = -d;
            } else {
                *(short *)(e + 0x3A) = a;
                v.x = 0;
                v.y = 0;
                v.z = -sp;
            }
        } else if (f & 0x40) {
            *(short *)(e + 0x3A) = a + 0x400;
            v.x = -sp;
            v.y = 0;
            v.z = 0;
        } else if (f & 0x10) {
            *(short *)(e + 0x3A) = a + 0xC00;
            v.x = sp;
            v.y = 0;
            v.z = 0;
        }
    }
    r = *(short *)(e + 0x3A);
    if (D_8009D254 != 0 && (D_8009D2E8 & 0x10)) {
        r += 0x800;
        r += ((unsigned int)(*D_8009D254)[0x4C / 4] >> 7) & 0xC00;
    }
    r &= 0xFFF;
    *(short *)(e + 0x3A) = r;
    func_80078934(D_800BD000, &v, (int *)(e + 0x68));
    if (D_8009D2E8 & 0x10) {
        *(short *)(e + 0x3A) = (0x1000 - *(unsigned short *)(e + 0x3A)) & 0xFFF;
        *(int *)(e + 0x68) = -*(int *)(e + 0x68);
    }
}
