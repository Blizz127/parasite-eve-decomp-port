/*
 * func_80069B08 — boot-spine disc-read / title-prep state machine.
 * VRAM 0x80069B08, file 0x5A308, size 0x5E0 (376 words), frame 0x58.
 * Jump table jtbl_80011388 in prefix rodata (cases 0..9).
 * ROM: asm/disc1/58518.s.
 */

typedef struct { short x, y, w, h; } RECT16;

extern unsigned char D_800B0DD8[];
extern unsigned short D_800930D8[];
extern unsigned short D_800930DA[];
extern unsigned char *D_800B0E6C;
extern unsigned char *D_800B0E64;
extern int D_8009CDDC;
extern unsigned char D_800BCE80[][20];
extern unsigned char D_800BCDC8[][92];
extern unsigned char *D_800B0E38;
extern unsigned char D_800B0DCD;
extern unsigned short D_800B0DD4;

extern void func_80073A44(int a0);
extern void func_80074D28(int a0);
extern int func_8006E6A8(int lba, unsigned char *dest, int sectors);
extern int func_8006E7E8(void);
extern void func_8003E974(void);
extern void func_800371B0(unsigned char *p);
extern void func_80074F44(void *rect, int r, int g, int b);
extern void func_800718D0(unsigned char *p);
extern void func_80074DC0(int a0);
extern void func_800755F0(void *env);
extern void func_800752AC(unsigned char *ot, int n);
extern int func_8007F72C(void);
extern int func_8007F778(void);
extern int func_8007EE84(int a0, int a1, int a2, int a3);
extern int func_8007F418(int a0, void *a1);
extern int func_8007F788(void);
extern void func_800374E8(void);
extern void func_800375E0(int a0, int a1, short *a2);
extern int func_800698D4(void);
extern void func_80037870(void);
extern void func_80074A44(int a0);
extern void func_80075424(void *env);
extern void func_8007512C(void *rect, int a1, int a2);
extern void func_800753B4(unsigned char *p);
extern int func_8007F7A8(void);

int func_80069B08(int a0) {
    struct {
        RECT16 rect;
        unsigned char buf18[8];
        short t20;
        char pad[12];
    } loc;
    register unsigned int s2 asm("$18");
    register int s6 asm("$22");
    register int s7 asm("$23");
    register int s1 asm("$17");
    register unsigned char *s3 asm("$19");
    register int s0 asm("$16");
    register int s5 asm("$21");
    int v1;
    int a2;

    (void)loc.pad;
    s2 = 0;
    s6 = 0;
    s7 = -1;
    s1 = 0xB4;
    loc.t20 = -1;
    func_80073A44(0);
    func_80074D28(0);

z1_again:
    s3 = D_800B0DD8;
    s0 = (int)D_800930D8;
    s5 = -1;
    while (func_8006E6A8(*(int *)s3 + ((unsigned short *)s0)[0],
                         *(unsigned char **)(s3 + 0x8C),
                         ((unsigned short *)s0)[1] - ((unsigned short *)s0)[0]) == s5)
        ;
    s0 = -1;
z1_poll:
    v1 = func_8006E7E8();
    if (v1 == 0)
        goto z2_again;
    if (v1 == s0)
        goto z1_again;
    goto z1_poll;

z2_again:
    s3 = D_800B0DD8;
    s0 = (int)D_800930DA;
    s5 = -1;
    while (func_8006E6A8(*(int *)s3 + ((unsigned short *)s0)[0],
                         *(unsigned char **)(s3 + 0x94),
                         ((unsigned short *)s0)[1] - ((unsigned short *)s0)[0]) == s5)
        ;
    s0 = -1;
z2_poll:
    v1 = func_8006E7E8();
    if (v1 == 0)
        goto after_reads;
    if (v1 == s0)
        goto z2_again;
    goto z2_poll;

after_reads:
    s0 = 0x140;
    func_8003E974();
    func_800371B0(D_800B0E6C);
    loc.rect.x = 0;
    loc.rect.y = 0;
    loc.rect.w = s0;
    loc.rect.h = 0x1C0;
    func_80074F44(&loc.rect, 0, 0, 1);
    func_800718D0(D_800B0E64);
    loc.rect.y = 0x100;
    loc.rect.x = s0;
    loc.rect.w = s0;
    loc.rect.h = 0xE0;
    func_80074DC0(0);
    func_800755F0(D_800BCE80[D_8009CDDC]);
    func_80074D28(1);
    if (s6)
        goto done_loop;
    s0 = 1;

state_loop:
    func_800752AC(*(unsigned char **)((unsigned char *)&D_800B0E38 + (D_8009CDDC << 2)), 0x1000);
    switch (s2) {
    case 0: goto case0;
    case 1: goto case1;
    case 2: goto case2;
    case 3: goto case3;
    case 4: goto case4;
    case 5: goto case5;
    case 6:
    case 7:
    case 8: goto case678;
    case 9: goto case9;
    }
    goto merge;
case0:
    if (func_8007F72C() != s0)
        goto merge;
    if (func_8007F778() != 0)
        goto merge;
    s7 = func_8007EE84(8, 0, 0, -1);
    s2 = 1;
    goto merge;

case1:
    v1 = func_8007F418(s7, loc.buf18);
    if (v1 == 2)
        goto f2c;
    if (v1 < 3)
        goto merge;
    if (v1 >= 7)
        goto merge;
    if (v1 < 5)
        goto merge;
    s2 = 0;
    goto merge;

case2:
    if ((func_8007F788() & 0x10) == 0)
        goto merge;
    s2 = 3;
    goto merge;

case3:
    v1 = func_8007F72C();
    if (v1 == 2)
        goto merge;
    if (v1 < 3) {
        if (v1 == s0)
            goto case3_one;
        goto merge;
    }
    if (v1 == 3)
        goto case3_three;
    goto merge;

case3_one:
    func_800374E8();
    func_800375E0(a0 == s0 ? 3 : 4, 0, &loc.t20);
    s1 = 0xB4;
    s2 = 5;
    goto merge;

case3_three:
    s1 = 0xB4;
    s2 = 6;
    goto merge;

case5:
    if (s1 != 0)
        goto dec_s1;
    s2 = 4;
    goto merge;

case4:
    v1 = func_800698D4();
    if (v1 == -1)
        goto case4_m1;
    if (v1 >= 0)
        goto case4_nonneg;
    if (v1 == -2)
        goto case4_m2;
    goto merge;
case4_nonneg:
    if (v1 == 0)
        goto case4_zero;
    goto merge;

case4_m1:
    func_800374E8();
    s1 = 0xB4;
    func_800375E0(5, 0, &loc.t20);
    s2 = 7;
    goto merge;

case4_m2:
    func_800374E8();
    s1 = 0xB4;
    func_800375E0(5, 0, &loc.t20);
    s2 = 8;
    goto merge;

case4_zero:
    if (a0 == s0) {
        if (D_800B0DCD & 1)
            goto set_s6;
    }
    if (a0 == 2) {
        if (D_800B0DCD & 2) {
        set_s6:
            s6 = 1;
            goto merge;
        }
    }
    s1 = 0xB4;
    s2 = 9;
    goto merge;

case678:
    if (s1 == 0)
        goto f24;
    s1 = s1 - 1;
    goto merge;

case9:
    if (s1 == 0)
        goto f24;
dec_s1:
    asm("");
    s1 = s1 - 1;
    goto merge;

f24:
    func_800374E8();
f2c:
    func_800375E0(a0 == s0 ? 1 : 2, 0, &loc.t20);
    s2 = 2;

merge:
    func_80037870();
    func_80074DC0(0);
    func_80073A44(0);
    func_80074A44(1);
    func_800755F0(D_800BCE80[D_8009CDDC]);
    func_80075424(D_800BCDC8[D_8009CDDC]);
    a2 = D_8009CDDC != 0 ? 0xE0 : 0;
    func_8007512C(&loc.rect, 0, a2);
    func_800753B4(*(unsigned char **)((unsigned char *)&D_800B0E38 + (D_8009CDDC << 2)) + 0x3FFC);
    D_8009CDDC = D_8009CDDC ^ 1;
    if (s6 == 0)
        goto state_loop;

done_loop:
    func_80073A44(0);
    func_80074D28(0);
    loc.rect.w = 0x140;
    loc.rect.x = 0;
    loc.rect.y = 0;
    loc.rect.h = 0x1C0;
    func_80074F44(&loc.rect, 0, 0, 1);
    func_80074DC0(0);
    D_800B0DCD = (a0 == 1) ? 1 : 2;
    s0 = 1;
    while (func_8007F72C() != s0)
        func_80073A44(0);
    D_800B0DD4 = func_8007F7A8();
    return 0;
}
