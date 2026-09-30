typedef struct Task {
    unsigned char pad0[0x24];
    int f24;
    unsigned char pad28[0x1C];
    int f44;
    int f48;
    unsigned char pad4C[0xC];
    int f58;
} Task;

extern int D_8009D004;
extern unsigned int D_800B0CD8;
extern Task *func_80062CC4(void);
extern void func_8005BE1C(void);
extern void func_800525EC(void);
extern int func_8005BD94(void);
extern void func_80052634(void);
extern void func_800526C4(void);
extern unsigned char *func_8005BEDC(void);
extern void func_800512AC(int, int);
extern int func_8005BEE8(void);
extern void func_80052594(int);
extern int func_8005BCB0(void);
extern Task *func_80062A34(int, int);
extern void func_80062CB8(Task *);
extern void func_8005267C(void);

static inline void confirm(void)
{
    unsigned char *s;
    unsigned char *p;
    int ok;

    s = func_8005BEDC();
    ok = 1;
    p = s;
    while (*p != 0xFF) {
        ok &= (*p++ == 0xF);
    }
    if (s < p && !ok) {
        func_800512AC(9, 0);
        if (D_8009D004 == 0) {
            func_80052594(func_8005BEE8());
        }
        D_800B0CD8 &= ~0x8000;
        func_800525EC();
    } else {
        func_800526C4();
    }
}

int func_8004E2E4(int a0, unsigned int pad)
{
    Task *t;
    Task *u;
    int k;
    int v;
    int w;
    int d;

    t = func_80062CC4();
    if (pad & 0x10000) {
        switch (t->f24) {
        case 0x17:
            pad |= 0x2000;
            break;
        case 0x18:
            if (t->f48 != 0) {
                func_8005BE1C();
                func_800525EC();
            } else if (func_8005BD94() != 0) {
                func_80052634();
            } else {
                func_800526C4();
            }
            break;
        case 0x19:
            confirm();
            break;
        }
    }
    if (pad & 0x2000) {
        k = func_8005BCB0() * 3;
        w = t->f48;
        switch (t->f24) {
        case 0x18:
            d = k - 3;
            v = w - d;
            break;
        default:
            d = k - 5;
            v = w - d;
            break;
        case 0x17:
            v = w - k;
            break;
        }
        if (t->f24 != 0x17) {
            t->f44 = -1;
        }
        t = func_80062A34(2, 0x11);
        t->f44 = 0;
        if (v < 5 - k || t->f48 < 6 - k) {
            t->f48 = v;
        }
        func_80062CB8(t);
        func_8005267C();
        goto out;
    }
    if (pad & 0x1000) {
        if (t->f24 != 0x17) {
            t->f44 = -1;
        }
        t = func_80062A34(2, func_8005BCB0() + 0x17 < t->f24 ? t->f24 - 1 : 0x19);
        t->f44 = 0;
        t->f48 = t->f58 - 1;
        func_80062CB8(t);
        func_8005267C();
        goto out;
    }
    if (pad & 0x4000) {
        if (t->f24 != 0x17) {
            t->f44 = -1;
        }
        t = func_80062A34(2, t->f24 < 0x19 ? t->f24 + 1 : func_8005BCB0() + 0x17);
        t->f44 = 0;
        t->f48 = 0;
        func_80062CB8(t);
        func_8005267C();
        goto out;
    }
    if (pad & 0x800) {
        if (t->f24 != 0x19) {
            t->f44 = -1;
            u = func_80062A34(2, 0x19);
            u->f44 = 0;
            func_80062CB8(u);
            goto out;
        }
        confirm();
    }
out:
    return 1;
}
