typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    unsigned int w4;
    unsigned char b8;
    unsigned char b9;
    unsigned short hA;
} Rec12;
typedef struct { int f; } SW;
typedef struct { short f; } SH;
typedef struct { unsigned char f; } SB;
#define W(p, o) (((SW *)((unsigned char *)(p) + (o)))->f)
#define H(p, o) (((SH *)((unsigned char *)(p) + (o)))->f)
#define B(p, o) (((SB *)((unsigned char *)(p) + (o)))->f)

extern int D_8009CE00;
extern unsigned char *D_8009D300;
extern unsigned char *D_8009D2F0;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D20C;
extern unsigned char *volatile D_800B0E64;
extern unsigned char *E64a[2] asm("D_800B0E64");
extern int D_800B0E00[];
typedef struct {
    unsigned char ce;
    unsigned char cf;
    short d0;
    short d2;
} DCE;
extern DCE D_800B0DCE;
extern unsigned char D_800B0CEA;
extern unsigned char D_800B0CE9[];
extern unsigned char D_800944A8[];
extern unsigned char D_800B0DBE;
extern unsigned char D_800B0DB8;
extern unsigned char D_800B0DB9;
extern unsigned char *D_800B0DFC;

extern void func_800867E4(int);
extern void func_8008682C(int);
extern int func_8006D2B8(int, int, int, int *, int);
extern int func_80086464(int);
extern void func_80086C1C(int, int);
extern void func_8006DB48(int, int, int, int);
extern int func_800864F8(int, int);
extern int func_8006DB9C(int);
extern void func_80086498(int);
extern void func_80086770(int);
extern int func_8006DBE0(int);
extern void func_80086C5C(int, int, int);
extern void func_80086CA4(int, int, int, int);
extern void func_80086DAC(int);
extern void func_80086DE4(int, int);
extern void func_80086E24(int, int, int);
extern void func_80086E70(int);
extern void func_80086EA8(int, int);
extern void func_80086EE8(int, int, int);
extern void func_80086F34(int);
extern void func_80086F6C(int, int);
extern void func_80086FAC(int, int, int);
extern void func_8006D24C(void);
extern int func_8006DF50(unsigned char *, int, int, int, int);
extern void func_800866A4(int, int);
extern void func_800868F0(int, int, int);
extern void func_80086948(int, int, int, int);
extern void func_80086874(int);
extern void func_800868AC(int, int);
extern void func_80086A28(int, int, int);
extern void func_80086A80(int, int, int, int);
extern void func_800869AC(int);
extern void func_800869E4(int, int);
extern void func_80086B60(int, int, int);
extern void func_80086BB8(int, int, int, int);
extern void func_80086AE4(int);
extern void func_80086B1C(int, int);
extern void func_80087024(void);
extern int func_8006DCE4(int, int, int, int, int);
extern void func_80080AC4(unsigned char *);
extern void func_800867B0(int);

int func_80015DAC(int **a)
{
    int buf;
    int r;
    unsigned char *p;

    switch (*a[0]) {
    case 100:
        func_800867E4(*a[1]);
        break;
    case 101:
        func_8008682C(*a[1]);
        break;
    case 200:
    case 408:
        r = func_8006D2B8(*a[1], 1, 0, &buf, *a[2] == 0);
        if (r == 1) {
            goto suspend;
        }
        if (buf == -1 || buf == -2) {
            break;
        }
        *a[5] = func_80086464(D_800B0E00[buf]);
        func_80086C1C(0, 0x7F);
        func_8006DB48(buf, *a[1], *a[5], 0x7F);
        break;
    case 203:
        r = func_8006D2B8(*a[1], 1, 1, &buf, *a[3] == 0);
        if (r == 1) {
            goto suspend;
        }
        if (buf == -1 || buf == -2) {
            break;
        }
        *a[5] = func_800864F8(D_800B0E00[buf], *a[2]);
        func_8006DB48(buf, *a[1], *a[5], *a[2]);
        break;
    case 201:
        buf = func_8006DB9C(*a[1]);
        if (buf == -1) {
            break;
        }
        func_80086498(buf);
        r = func_8006D2B8(*a[1], 0, 0, &buf, 1);
        if (r != 1) {
            break;
        }
        goto suspend;
    case 204:
        func_80086770(*a[1]);
        break;
    case 205:
        buf = func_8006DB9C(*a[1]);
        if (buf == -1) {
            break;
        }
        func_80086C1C(buf, *a[2]);
        {
        int s = func_8006DBE0(*a[1]);
        func_8006DB48(s, *a[1], func_8006DB9C(*a[1]), *a[2]);
        }
        break;
    case 206:
        buf = func_8006DB9C(*a[1]);
        if (buf == -1) {
            break;
        }
        func_80086C5C(buf, *a[2] * 2, *a[3]);
        {
        int s = func_8006DBE0(*a[1]);
        func_8006DB48(s, *a[1], func_8006DB9C(*a[1]), *a[3]);
        }
        break;
    case 207:
        buf = func_8006DB9C(*a[1]);
        if (buf == -1) {
            break;
        }
        func_80086CA4(buf, *a[2] * 2, *a[3], *a[4]);
        {
        int s = func_8006DBE0(*a[1]);
        func_8006DB48(s, *a[1], func_8006DB9C(*a[1]), *a[4]);
        }
        break;
    case 208:
        func_80086DAC(*a[1]);
        break;
    case 209:
        func_80086DE4(*a[1] * 2, *a[2]);
        break;
    case 210:
        func_80086E24(*a[1] * 2, *a[2], *a[3]);
        break;
    case 211:
        func_80086E70(*a[1]);
        break;
    case 212:
        func_80086EA8(*a[1] * 2, *a[2]);
        break;
    case 213:
        func_80086EE8(*a[1] * 2, *a[2], *a[3]);
        break;
    case 214:
        func_80086F34(*a[1]);
        break;
    case 215:
        func_80086F6C(*a[1] * 2, *a[2]);
        break;
    case 216:
        func_80086FAC(*a[1] * 2, *a[2], *a[3]);
        break;
    case 217:
        func_8006D24C();
        break;
    case 300:
        *a[5] = func_8006DF50(E64a[0], *a[1], *a[2], *a[3], *a[4]);
        break;
    case 301:
        func_800866A4(*a[1], *a[2]);
        break;
    case 302:
        func_800868F0(*a[1], *a[2], *a[3]);
        break;
    case 303:
        func_80086948(*a[1], *a[2], *a[3] * 2, *a[4]);
        break;
    case 304:
        func_80086874(*a[1]);
        break;
    case 305:
        func_800868AC(*a[1] * 2, *a[2]);
        break;
    case 306:
        func_80086A28(*a[1], *a[2], *a[3]);
        break;
    case 307:
        func_80086A80(*a[1], *a[2], *a[3] * 8, *a[4]);
        break;
    case 308:
        func_800869AC(*a[1]);
        break;
    case 309:
        func_800869E4(*a[1] * 2, *a[2]);
        break;
    case 310:
        func_80086B60(*a[1], *a[2], *a[3]);
        break;
    case 311:
        func_80086BB8(*a[1], *a[2], *a[3] * 2, *a[4]);
        break;
    case 312:
        func_80086AE4(*a[1]);
        break;
    case 313:
        func_80086B1C(*a[1] * 2, *a[2]);
        break;
    case 314:
        func_80087024();
        break;
    case 350:
        D_800B0DCE.d0 = *a[1];
        D_800B0DCE.ce = *a[2];
        D_800B0DCE.d2 = *a[3];
        D_800B0DCE.cf = *a[4];
        break;
    case 351:
        *a[5] = func_8006DCE4(*a[1], 0, H(D_8009D2F0, 0x2A), H(D_8009D2F0, 0x2E), H(D_8009D2F0, 0x32));
        break;
    case 352:
        if (*a[2] == 0) {
            {
                register unsigned char *q asm("$2") = D_8009D254;
                if (q == 0) {
                    break;
                }
                p = q;
            }
            goto found;
        }
        for (p = D_8009D20C; p != 0; p = (unsigned char *)W(p, 4)) {
            if (B(p, 0xC) == *a[2] && B(p, 0xD) == *a[3] && !(W(p, 0x98) & 0x10)) {
                break;
            }
        }
        if (p == 0) {
            return 1;
        }
    found:
        *a[5] = func_8006DCE4(*a[1], 0, H(p, 0x2A), H(p, 0x2E), H(p, 0x32));
        break;
    case 353:
        *a[5] = func_8006DCE4(*a[1], 0, ((short *)a[2])[1], ((short *)a[3])[1], ((short *)a[4])[1]);
        break;
    case 400:
        {
            unsigned int hdr;
            unsigned int n;
            unsigned int i;
            Rec12 *q;
            int off;
            unsigned char *base;

            off = W(D_800B0E64, 4);
            base = D_800B0E64;
            hdr = W(base + off, 0x30);
            q = (Rec12 *)(base + (hdr & 0x3FFFFF));
            for (i = 0; i < hdr >> 22; i++) {
                Rec12 *e = &q[i];
                register int h asm("$5");
                if ((e->b3 & 0x10) && (h = e->hA) == *a[1]) {
                    register unsigned int m asm("$4");
                    register unsigned int w asm("$2");
                    register unsigned char b asm("$2");
                    m = 0xFFFFFF;
                    D_800B0DB8 = h;
                    b = e->b8;
                    D_800B0DB9 = b;
                    w = e->w4 & m;
                    D_800B0DFC = D_800B0E64 + w;
                    return 1;
                }
            }
        }
        break;
    case 401:
        r = func_8006D2B8(*a[1], 1, 0, &buf, *a[2] == 0);
        if (r == 1) {
            goto suspend;
        }
        goto store;
    case 402:
        r = func_8006D2B8(*a[1], 1, 1, &buf, *a[2] == 0);
        if (r == 1) {
        suspend:
            D_8009CE00 -= 0x20;
            W(D_8009D300, 0x10) = r;
            return 0;
        }
    store:
        *a[5] = buf;
        break;
    case 405:
        D_800B0CEA = 1;
        break;
    case 406:
    case 407:
        if (D_800B0CE9[0] < 16) {
            p = &D_800944A8[D_800B0CE9[0] * 8];
            p[0] = B(D_8009D2F0, 0xC);
            p[1] = B(D_8009D2F0, 0xD);
            p[2] = *a[1];
            p[3] = *a[2];
            *(short *)(p + 4) = *a[3];
            *(short *)(p + 6) = *(*a[0] == 406 ? a[3] : a[4]);
            asm volatile("");
            D_800B0CE9[0]++;
        }
        break;
    case 409:
        {
        unsigned char c[4];
        int v = *a[1];
        c[3] = 0;
        c[1] = 0;
        D_800B0DBE = v;
        c[2] = v;
        c[0] = v;
        func_80080AC4(c);
        }
        break;
    case 410:
        func_800867B0(*a[1]);
        break;
    }
    return 1;
}
