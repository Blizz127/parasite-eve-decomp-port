extern short D_800C0E28[];
extern unsigned short D_800C0E08;
extern unsigned char D_800C0E0A;
extern int D_800A1B30;
extern int D_800A1B34;
extern int D_800A1B38;
extern int D_800A1B3C;
extern int D_800A1B40;
extern int D_800A1B44;
extern unsigned char **D_8009D254;
extern void func_8005B91C(int, int, int *, int);
extern unsigned char *func_8005DBAC(int);
extern void func_80052F24(int);

void func_8005218C(void)
{
    int buf[2];
    short *p;
    unsigned char *s1;
    unsigned char *r;
    int v;
    register int b asm("$2");

    short v2;
    register unsigned char **pp asm("$4");

    p = D_800C0E28;
    func_8005B91C(0, p[0], buf, 0);
    p++;
    r = func_8005DBAC(buf[0]);
    pp = D_8009D254;
    v = ((D_800A1B30 + 0x14) * *(unsigned short *)r) / 20;
    __asm__ __volatile__("" : "=r"(p) : "0"(p));
    *(short *)((char *)p - 0x24) = v;
    if (pp != 0) {
        s1 = *pp;
        if (s1 != 0) {
            v2 = v;
            *(short *)(s1 + 0x1C) = v2;
            if ((short)v < *(short *)(s1 + 0xC)) {
                *(short *)(s1 + 0xC) = v2;
            }
            if (*(short *)(s1 + 0x1C) < *(short *)(s1 + 0xE)) {
                *(short *)(s1 + 0xE) = *(short *)(s1 + 0x1C);
            }
            if (*(unsigned short *)((char *)p - 0x24) < D_800C0E08) {
                D_800C0E08 = *(unsigned short *)((char *)p - 0x24);
            }
            func_8005B91C(1, p[0], buf, 0);
            p++;
            r = func_8005DBAC(buf[0] + D_800A1B34);
            *(short *)(s1 + 0x1E) = *(unsigned short *)(r + 2);
            func_8005B91C(2, p[0], buf, 0);
            p++;
            r = func_8005DBAC(buf[0] + D_800A1B38);
            *(short *)(s1 + 0x20) = *(unsigned short *)(r + 4);
            func_8005B91C(3, p[0], buf, 0);
            p++;
            r = func_8005DBAC(buf[0] + D_800A1B3C);
            *(int *)(s1 + 0x28) = *(int *)(r + 8);
            *(int *)(s1 + 0x30) = *(int *)(r + 0xC);
            *(int *)(s1 + 0x2C) = *(int *)(r + 0x10);
            func_8005B91C(4, p[0], buf, 0);
            p++;
            r = func_8005DBAC(buf[0] + D_800A1B40);
            *(short *)(s1 + 0x3C) = *(unsigned short *)(r + 0x14);
            *(short *)(s1 + 0x3E) = *(unsigned short *)(r + 0x16);
            func_8005B91C(5, p[0], buf, 0);
            p++;
            r = func_8005DBAC(buf[0] + D_800A1B44);
            *(short *)(s1 + 0x22) = r[6];
            func_8005B91C(6, p[0], buf, 0);
            r = func_8005DBAC(buf[0]);
            b = r[7];
            *(short *)(s1 + 0x26) = b;
            func_80052F24(b);
            *(short *)(s1 + 4) = D_800C0E0A + 1;
        }
    }
}
