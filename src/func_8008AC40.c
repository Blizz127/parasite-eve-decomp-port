extern unsigned char D_800B8968[];
extern unsigned char *D_8009D2C8;
extern unsigned char D_800B6B80[];
extern unsigned char D_800B8AC0[];
extern unsigned int D_8009CDE8;
extern int D_800B8994;
extern unsigned int D_8009D2C4;
extern unsigned char D_8009B8F4[];
extern unsigned char D_800BA560[];
extern unsigned int D_800BCD50;
extern unsigned int D_800BCD5C;
extern unsigned short D_800B89BC;
extern unsigned int D_8009D2DC;

extern void func_8008D820(unsigned int *a0, unsigned int *a1, unsigned int a2);
extern unsigned int func_80089FE0(unsigned char *a0, unsigned int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_8008AC40(int arg0)
{
    register int delta asm("$17");
    register unsigned char *b asm("$16");
    register unsigned int bit asm("$7");
    register unsigned int n asm("$8");
    register unsigned int c4 asm("$10");
    register unsigned int omask asm("$9");
    register unsigned char *ctl asm("$6");
    register unsigned char *tmp asm("$2");
    unsigned char *p;
    register unsigned int flags asm("$4");
    unsigned int cde;
    unsigned int w0;
    unsigned int w14;
    unsigned int r;
    unsigned int w;

    delta = arg0;
    func_8008D820((unsigned int *)D_800B8968, (unsigned int *)D_8009D2C8, 0x68);
    func_8008D820((unsigned int *)D_800B6B80, (unsigned int *)D_800B8AC0, 0x1AA0);
    b = D_800B8AC0;

    n = 0x18;
    bit = 1;
    omask = 0x1FF93;
    c4 = 4;
    asm volatile("" : "=r"(c4) : "0"(c4));
    p = b + 0x58;
    tmp = D_8009D2C8;
    cde = D_8009CDE8;
    asm volatile("" : "=r"(tmp) : "0"(tmp));
    ctl = tmp;
    w0 = *(unsigned int *)ctl;
    w14 = *(unsigned int *)(ctl + 0x14);
    cde = cde & 0x100;
    asm volatile("" : "=r"(cde) : "0"(cde));
    *(int *)(ctl + 0x2C) = delta;
    asm volatile("" : "=r"(w0) : "0"(w0));
    *(unsigned int *)ctl = w0 | cde;
    *(unsigned int *)(ctl + 0x10) = w14;
    delta = delta - D_800B8994;
    D_8009D2C4 |= 0x90;
    flags = *(unsigned int *)(ctl + 4);
loop:
    if (flags & bit) {
            *(int *)b += delta;
            *(int *)(p - 0x44) += delta;
            *(int *)(p - 0x54) += delta;
            *(int *)(p - 0x50) += delta;
            *(int *)(p - 0x4C) += delta;
            *(int *)(p - 0x48) += delta;
            *(unsigned short *)(p - 2) += 2;
            *(unsigned short *)p += 2;
            *(unsigned int *)(p + 0x9C) |= omask;
            if (*(unsigned int *)ctl & 0x100) {
                unsigned short hv;

                hv = *(unsigned short *)(p + 2);
                if (hv >= 0x20) {
                    *(unsigned short *)(p + 2) = hv + 0x30;
                }
            }
    } else {
        unsigned short *hp;
        int addr;

        hp = (unsigned short *)p;
        *hp = 2;
        addr = (int)D_8009B8F4;
        *(unsigned short *)(p - 2) = c4;
        *(int *)b = addr;
    }
    n--;
    p += 0x11C;
    b += 0x11C;
    bit <<= 1;
    if (n != 0) {
        goto loop;
    }

    {
        unsigned char *c3;

        c3 = D_8009D2C8;
        r = func_80089FE0(D_800BA560,
                          *(unsigned int *)(c3 + 0x6C) & *(unsigned int *)(c3 + 0x70));
    }
    {
        register unsigned int m asm("$5");
        register unsigned int inv asm("$2");
        unsigned int nd;
        unsigned char *c2;

        m = 0xFFFFFF;
        c2 = D_8009D2C8;
        inv = ~r;
        *(unsigned int *)(c2 + 0x18) = 0;
        nd = ~D_800BCD50 & m;
        inv = inv & nd;
        D_800BCD5C |= inv;
    }
    func_80089960();
    func_80089B28();
    func_80089CF0();
    D_800B89BC = 0;
    if (D_8009D2DC & 1) {
        unsigned char *c3;

        c3 = D_8009D2C8;
        w = *(unsigned int *)(c3 + 4);
        *(unsigned int *)(c3 + 4) = 0;
        *(unsigned int *)(c3 + 0x1C) = w;
    }
}
