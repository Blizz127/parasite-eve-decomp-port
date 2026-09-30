/*
 * func_8003F3C4 — field-tick / main-loop body (boot-spine jal from
 * func_8001220C). VRAM 0x8003F3C4, file 0x2FBC4, size 0x394 (229 words).
 * Frame 0x28; era -O2 -G8 + MASPSX_FORCE_ABSOLUTE_SYMBOLS for in-range
 * words that retail keeps absolute.
 * ROM: asm/disc1/2F174.s.
 */

typedef struct { short x, y, w, h; } RECT16;

extern unsigned int D_8009D1C4; /* 0x454($gp) */
extern unsigned int D_8009D280; /* 0x510($gp) */
extern unsigned int D_8009D1A0; /* 0x430($gp) */
extern unsigned int D_8009CDA4; /* 0x34($gp) */
extern unsigned int D_8009CDD8;
extern unsigned int D_8009D1F4;
extern unsigned int D_8009D238;
extern int D_8009CDDC;
extern unsigned int D_8009D250;
extern unsigned int D_8009D26C;
extern unsigned char D_800B0CEA;
extern unsigned int D_800B0CD8;
extern unsigned char D_800BCFE8[];

extern void func_8003F074(void);
extern void func_8003EB04(void);
extern void func_800752AC(unsigned int a0, int a1);
extern void func_80065400(void);
extern void func_80035558(void);
extern int func_8006EC08(void);
extern int func_80122040(void);
extern void func_80121A00(void);
extern void func_8006E60C(void);
extern void func_80068CE0(void);
extern void func_80037870(void);
extern void func_800661A4(void);
extern void func_800E01BC(void);
extern void func_800661CC(void);
extern void func_80068E24(void);
extern void func_80070E54(void);
extern void func_80066C7C(int a0);
extern void func_80073A44(int a0);
extern void func_8006A25C(void);
extern void func_8006A0E8(void);
extern void func_80074F44(void *rect, int r, int g, int b);
extern void func_80074DC0(int a0);
extern void func_80087024(void);
extern void func_8003DFC8(int a0);
extern void func_800696F0(void);

void func_8003F3C4(void) {
    RECT16 rect;
    unsigned int a1;
    unsigned int a0;
    unsigned int flags;

    func_8003F074();
    if (D_8009D1C4 == D_8009D280) {
        register unsigned char *s2 asm("$18");
        register unsigned int *s0 asm("$16");
        register unsigned char *s1 asm("$17");
        unsigned char *q;

        s2 = &D_800B0CEA;
        asm volatile("" : "=r"(s2) : "0"(s2));
        s0 = (unsigned int *)(s2 - 0x12);
        s1 = D_800BCFE8;
        do {
            D_8009CDD8 = 0;
            *s2 = 0;
            func_8003EB04();

            {
                register unsigned int m asm("$3");
                m = ~0x30u;
                a1 = D_8009D1A0;
                flags = *(unsigned int *)(s2 - 0x12);
                a0 = a1 & m;
                D_8009D1A0 = a0;
            }
            if ((flags & 0x8000) == 0 && D_8009CDA4 != 0 && (D_8009D1F4 & 4) != 0
                && (D_8009D238 & 0xB0002380u) == 0) {
                if (a1 & 1)
                    D_8009D1A0 = (a0 | 0x20) ^ 1;
                else
                    D_8009D1A0 = (a0 | 0x10) ^ 1;
            }

            if ((D_8009D1A0 & 1) == 0) {
                if ((*s0 & 0x200) == 0) {
                    q = (unsigned char *)s0 + (D_8009CDDC << 2);
                    func_800752AC(*(unsigned int *)(q + 0x160), 0x1000);
                }
                D_8009D250 = D_8009D250 + 1;
                func_80065400();
                func_80035558();
                if ((*s0 & 0x100) == 0) {
                    if ((signed char)func_8006EC08() != 0) {
                        if ((signed char)func_80122040() == 0) {
                            func_80121A00();
                            func_8006E60C();
                            goto check_loop;
                        }
                    } else if ((*s0 & 0x200) == 0) {
                        func_80068CE0();
                        func_80037870();
                        func_800661A4();
                        func_800E01BC();
                        func_800661CC();
                        func_80068E24();
                    }
                    func_80070E54();
                    if (D_8009CDA4 == 0
                        && *(unsigned int *)s1 == 0xFF00FF
                        && *(short *)(s1 + 4) == 0xFF
                        && (s1[6] & 0x40) != 0)
                        func_80066C7C(0xF);
                }
            } else {
                func_80073A44(2);
            }

            if ((D_800B0CD8 & 0x4200) == 0) {
                if ((D_8009D26C & 0xF000006u) == 0xF000006u)
                    func_8006A25C();
            }
            if ((*s0 & 0x100) == 0)
                func_8006A0E8();
            D_8009CDA4 = D_8009CDA4 + 1;
            if ((D_8009D1A0 & 0x2000) != 0 && (*s0 & 0x800) != 0)
                break;
        check_loop:
            ;
        } while (D_8009D1C4 == D_8009D280);
    }

    if (D_800B0CD8 & 0x200) {
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x140;
        rect.h = 0x1C0;
        func_80074F44(&rect, 0, 0, 1);
    }
    func_80074DC0(0);
    func_80087024();
    func_8003DFC8(1);
    func_800696F0();
    {
        register unsigned int v0 asm("$2");
        register unsigned int v1 asm("$3");
        register unsigned int a00 asm("$4");

        a00 = ~0x3800u;
        v0 = D_8009D1A0;
        v1 = D_800B0CD8;
        v0 = (v0 | 0x40) & a00;
        v1 = v1 | 2;
        D_8009D1A0 = v0;
        v0 = v1 & ~0x800u;
        D_800B0CD8 = v0;
        if (v1 & 0x200)
            D_800B0CD8 = (v0 | 2) & 0xFFFF7DFFu;
    }
}
