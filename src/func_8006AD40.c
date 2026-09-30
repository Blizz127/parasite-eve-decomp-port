/*
 * func_8006AD40 — boot-spine overlay/image streamer.
 * VRAM 0x8006AD40, file 0x5B540, size 0x61C (391 words), frame 0x30.
 * ROM: asm/disc1/5B1E4.s.
 */

extern unsigned char D_800B0CD8[];
extern unsigned int D_800B0DD8;
extern unsigned short D_800930EA[];
extern unsigned short D_800930EC[];
extern unsigned short D_800930EE[];
extern unsigned short D_800930F0[];
extern unsigned short D_800930E0[];
extern unsigned short D_80093126[];
extern unsigned short D_80091648;
extern unsigned short D_8009164A;
extern unsigned short D_8009164C;
extern unsigned short D_8009164E;
extern unsigned short D_80091650;
extern unsigned short D_80091652;
extern int D_8009CDDC;
extern unsigned char D_800BCE80[][20];

extern int func_8006E6A8(int lba, unsigned char *dest, int sectors);
extern int func_8006E7E8(void);
extern void func_8006E1C0(unsigned char *a0, unsigned char *a1);
extern int func_8006E498(unsigned char *p, unsigned int code);
extern void func_8007506C(unsigned char *a0, unsigned char *a1);
extern void func_800718D0(unsigned char *p);
extern void func_80030894(void);
extern void func_80087024(void);
extern void func_80074DC0(int a0);
extern void func_80074A44(int a0);
extern void func_80073A44(int a0);
extern void func_800755F0(void *env);
extern void func_80074D28(int a0);

int func_8006AD40(void) {
    register unsigned char *s5 asm("$21");
    register unsigned int s6 asm("$22");
    register int s0 asm("$16");
    register int s1 asm("$17");
    register int s2 asm("$18");
    register unsigned char *s4 asm("$20");
    register unsigned char *s3 asm("$19");
    register int z asm("$0");

    s5 = D_800B0CD8;
    if (*(unsigned int *)s5 & 1) {
    s6 = D_800B0DD8;

read0:
    s0 = (int)D_800930EA;
    s1 = -1;
    while (func_8006E6A8((int)(s6 + ((unsigned short *)s0)[0]),
                         *(unsigned char **)(s5 + 0x160),
                         ((unsigned short *)s0)[1] - ((unsigned short *)s0)[0]) == s1)
        ;
    s2 = 1;
    s0 = -1;
poll0:
    if (s2 == s0)
        goto read0;
    s2 = func_8006E7E8();
    if (s2 != 0)
        goto poll0;
    asm volatile("" : "=r"(s2) : "0"(s2));
    s0 = 0;

read1:
    s1 = (int)D_800930EC;
    s2 = -1;
    while (func_8006E6A8((int)(s6 + ((unsigned short *)s1)[0]),
                         *(unsigned char **)(s5 + 0x174),
                         ((unsigned short *)s1)[1] - ((unsigned short *)s1)[0]) == s2)
        ;
    s2 = 1;
work1:
    if (s0 != 0)
        goto poll1;
    s4 = *(unsigned char **)(s5 + 0x160);
    {
        unsigned int w;
        register unsigned int mask asm("$3");
        register unsigned char *a0 asm("$4");
        mask = 0x3FFFFF;
        w = *(unsigned int *)(s4 + 4);
        s3 = s4 + w;
        w = *(unsigned int *)(s3 + 0x28);
        s1 = z;
        mask = w & mask;
        a0 = s4 + mask;
        {
            register unsigned int v0 asm("$2");
            v0 = w >> 22;
            v0 = ((unsigned)s0 < v0);
            if (v0) {
                s0 = (int)a0;
                do {
                    func_8006E1C0((unsigned char *)s0, s4);
                    w = *(unsigned int *)(s3 + 0x28);
                    s1 = s1 + 1;
                    s0 = s0 + 0x14;
                } while ((unsigned)s1 < (w >> 22));
            }
        }
    }
    {
        register int off asm("$5");
        register unsigned int a asm("$4");
        register unsigned int b asm("$2");
        register unsigned int c asm("$3");
        off = 0x20;
        do {
            a = *(unsigned short *)((unsigned char *)&D_8009164A + off);
            b = *(unsigned short *)((unsigned char *)&D_80091648 + off);
            c = (a & 0x100) >> 4;
            b = ((b & 0x3FF) >> 6) | 0x20;
            c = c | b;
            a = (a & 0x200) << 2;
            c = c | a;
            *(unsigned short *)((unsigned char *)&D_80091650 + off) = c;
            c = *(unsigned short *)((unsigned char *)&D_8009164E + off);
            b = *(unsigned short *)((unsigned char *)&D_8009164C + off);
            c = (c << 6) | ((b >> 4) & 0x3F);
            *(unsigned short *)((unsigned char *)&D_80091652 + off) = c;
            off = off + 0x10;
        } while ((unsigned)off < 0x40);
    }
    s0 = func_8006E498(s4, 0xABADC06Cu);
    asm volatile("" : "=r"(s0) : "0"(s0));
    if (*(unsigned int *)s0 != 0) {
        do {
            unsigned int w;
            func_8007506C((unsigned char *)s0 + 4, (unsigned char *)s0 + 0xC);
            w = *(unsigned int *)s0;
            s0 = s0 + ((int)(w >> 2) << 2);
        } while (*(unsigned int *)s0 != 0);
    }
    s0 = 1;
poll1:
    if (s2 == -1)
        goto read1;
    s2 = func_8006E7E8();
    if (s2 != 0)
        goto work1;
    asm volatile("" : "=r"(s2) : "0"(s2));
    s0 = 0;

read2:
    s1 = (int)D_800930EE;
    s2 = -1;
    while (func_8006E6A8((int)(s6 + ((unsigned short *)s1)[0]),
                         *(unsigned char **)(s5 + 0x180),
                         ((unsigned short *)s1)[1] - ((unsigned short *)s1)[0]) == s2)
        ;
    s2 = 1;
work2:
    if (s0 != 0)
        goto poll2;
    s4 = *(unsigned char **)(s5 + 0x174);
    func_800718D0(s4);
    {
        register int off asm("$5");
        register unsigned int a asm("$4");
        register unsigned int b asm("$2");
        register unsigned int c asm("$3");
        off = z;
        do {
            a = *(unsigned short *)((unsigned char *)&D_8009164A + off);
            b = *(unsigned short *)((unsigned char *)&D_80091648 + off);
            c = (a & 0x100) >> 4;
            b = ((b & 0x3FF) >> 6) | 0x20;
            c = c | b;
            a = (a & 0x200) << 2;
            c = c | a;
            *(unsigned short *)((unsigned char *)&D_80091650 + off) = c;
            c = *(unsigned short *)((unsigned char *)&D_8009164E + off);
            b = *(unsigned short *)((unsigned char *)&D_8009164C + off);
            c = (c << 6) | ((b >> 4) & 0x3F);
            *(unsigned short *)((unsigned char *)&D_80091652 + off) = c;
            off = off + 0x10;
        } while ((unsigned)off < 0x20);
    }
    s0 = 1;
poll2:
    if (s2 == -1)
        goto read2;
    s2 = func_8006E7E8();
    if (s2 != 0)
        goto work2;
    asm volatile("" : "=r"(s2) : "0"(s2));
    s0 = 0;
read3:
    s1 = (int)D_800930F0;
    s2 = -1;
    while (func_8006E6A8((int)(s6 + ((unsigned short *)s1)[0]),
                         *(unsigned char **)(s5 + 0x14C),
                         ((unsigned short *)s1)[1] - ((unsigned short *)s1)[0]) == s2)
        ;
    s2 = 1;
    s1 = -1;
work3:
    if (s0 == 0) {
        func_800718D0(*(unsigned char **)(s5 + 0x180));
        s0 = 1;
        func_80030894();
    }
    if (s2 == s1)
        goto read3;
    s2 = func_8006E7E8();
    if (s2 != 0)
        goto work3;
    asm volatile("" : "=r"(s2) : "0"(s2));
    s0 = 0;

read4:
    s1 = (int)D_800930E0;
    s2 = -1;
    while (func_8006E6A8((int)(s6 + ((unsigned short *)s1)[0]),
                         *(unsigned char **)(s5 + 0x16C),
                         ((unsigned short *)s1)[1] - ((unsigned short *)s1)[0]) == s2)
        ;
    s2 = 1;
    s1 = -1;
work4:
    if (s0 == 0) {
        unsigned int r;
        register unsigned int a1 asm("$5");
        register unsigned char *a0 asm("$4");
        a1 = 0xC4B5BA04u;
        asm volatile("" : "=r"(a1) : "0"(a1));
        a0 = *(unsigned char **)(s5 + 0x14C);
        r = func_8006E498(a0, a1);
        s0 = 1;
        *(unsigned int *)(s5 + 0x11C) = r;
        a1 = 0xCAAD0704u;
        a0 = *(unsigned char **)(s5 + 0x14C);
        r = func_8006E498(a0, a1);
        *(unsigned int *)(s5 + 0x120) = r;
        a1 = 0x5EAF6804u;
        a0 = *(unsigned char **)(s5 + 0x14C);
        r = func_8006E498(a0, a1);
        *(unsigned int *)(s5 + 0x124) = r;
    }
    if (s2 == s1)
        goto read4;
    s2 = func_8006E7E8();
    if (s2 != 0)
        goto work4;
    asm volatile("" : "=r"(s2) : "0"(s2));
    s0 = 0;

read5:
    s1 = (int)D_80093126;
    s2 = -1;
    while (func_8006E6A8((int)(s6 + ((unsigned short *)s1)[0]),
                         *(unsigned char **)(s5 + 0x188),
                         ((unsigned short *)s1)[1] - ((unsigned short *)s1)[0]) == s2)
        ;
    s2 = 1;
work5:
    if (s0 != 0)
        goto poll5;
    s4 = *(unsigned char **)(s5 + 0x16C);
    {
        unsigned int w;
        register unsigned int mask asm("$3");
        register unsigned char *a0 asm("$4");
        mask = 0x3FFFFF;
        w = *(unsigned int *)(s4 + 4);
        s3 = s4 + w;
        w = *(unsigned int *)(s3 + 0x28);
        s1 = z;
        mask = w & mask;
        a0 = s4 + mask;
        {
            register unsigned int v0 asm("$2");
            v0 = w >> 22;
            v0 = ((unsigned)s0 < v0);
            if (v0) {
                s0 = (int)a0;
                do {
                    func_8006E1C0((unsigned char *)s0, s4);
                    w = *(unsigned int *)(s3 + 0x28);
                    s1 = s1 + 1;
                    s0 = s0 + 0x14;
                } while ((unsigned)s1 < (w >> 22));
            }
        }
    }
    s0 = 1;
poll5:
    if (s2 == -1)
        goto read5;
    s2 = func_8006E7E8();
    if (s2 != 0)
        goto work5;

    s4 = *(unsigned char **)(s5 + 0x188);
    {
        unsigned int w;
        register unsigned int mask asm("$2");
        register unsigned char *a0 asm("$4");
        mask = 0x3FFFFF;
        w = *(unsigned int *)(s4 + 4);
        s3 = s4 + w;
        w = *(unsigned int *)(s3 + 0x28);
        s1 = z;
        mask = w & mask;
        w = w >> 22;
        a0 = s4 + mask;
        if (w != 0) {
            s0 = (int)a0;
            {
                register unsigned int t asm("$2");
                do {
                    func_8006E1C0((unsigned char *)s0, s4);
                    t = *(unsigned int *)(s3 + 0x28);
                    s1 = s1 + 1;
                    s0 = s0 + 0x14;
                } while ((unsigned)s1 < (t >> 22));
            }
        }
    }

    func_80087024();
    func_80074DC0(0);
    func_80074A44(1);
    func_80073A44(0);
    func_800755F0(D_800BCE80[D_8009CDDC]);
    func_80074D28(1);
    {
        register unsigned int v0 asm("$2");
        register unsigned int v1 asm("$3");
        register int a0 asm("$4");
        v1 = -1;
        asm volatile("" : "=r"(v1) : "0"(v1));
        v0 = *(unsigned int *)s5;
        a0 = -1;
        *(unsigned short *)(s5 + 6) = (unsigned short)v1;
        s5[0xB] = 0;
        s5[0xC] = (unsigned char)a0;
        s5[9] = (unsigned char)a0;
        *(unsigned short *)(s5 + 0xE8) = (unsigned short)v1;
        s5[0xEB] = 0;
        s5[0xEA] = 0;
        if ((v0 & 0x40) == 0) {
            s5[0xDA] = (unsigned char)a0;
            s5[0xDD] = (unsigned char)a0;
            s5[0xDC] = (unsigned char)a0;
        }
        v0 = *(unsigned int *)s5;
        if ((v0 & 0x80) == 0) {
            v0 = 0;
            asm volatile("" : "=r"(v0) : "0"(v0));
            s5[0xDB] = (unsigned char)a0;
            s5[0xDF] = (unsigned char)a0;
            s5[0xDE] = (unsigned char)a0;
        }
        v1 = *(unsigned int *)s5;
        a0 = -2;
        *(unsigned int *)s5 = v1 & a0;
    }
    }
    return 0;
}
