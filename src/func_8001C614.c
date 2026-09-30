typedef struct {
    unsigned char pad0[0x18];
    unsigned char *f18;
} Hdr;

extern void *volatile D_8009D1D8;
extern Hdr *D_8009D1FC;

int func_8001C614(void *a0, int a1, int a2)
{
    register unsigned char *t3 asm("$11");
    register Hdr *t6 asm("$14");
    register unsigned int t0 asm("$8");
    register unsigned int t5 asm("$13");
    register unsigned short curz asm("$9");
    register unsigned short curx asm("$7");
    register int t4 asm("$12");
    register int t2 asm("$10");
    unsigned short idx;
    unsigned char *row;

    t3 = (unsigned char *)a0;
    if (D_8009D1D8) {
        idx = *(unsigned short *)(t3 + 12);
        row = (unsigned char *)((idx * 3 << 1) + (unsigned int)D_8009D1FC->f18);
        curz = *(unsigned short *)(row + 4);
        curx = *(unsigned short *)row;
        asm volatile("" : "=r"(curx) : "0"(curx));
        t5 = 0;
    } else {
        register unsigned char *rowf asm("$3");
        unsigned short idxf;

        idxf = *(unsigned short *)(t3 + 6);
        rowf = (unsigned char *)((idxf << 2) + (unsigned int)D_8009D1FC->f18);
        curz = *(unsigned short *)(rowf + 2);
        curx = *(unsigned short *)rowf;
        asm volatile("" : "=r"(curx) : "0"(curx));
        t5 = 0;
    }
    t0 = 0;
    t6 = D_8009D1FC;
    {
        register int sh asm("$2");

        sh = a2 << 16;
        t4 = sh >> 16;
        sh = a1 << 16;
        t2 = sh >> 16;
    }
    do {
        register unsigned short oldx asm("$6");
        register unsigned short oldz asm("$4");
        int c1;
        int nz;
        int ox;
        int oz;
        int nx;

        oldx = curx;
        asm volatile("" : "=r"(oldx) : "0"(oldx));
        oldz = curz;
        if (D_8009D1D8) {
            register unsigned char *rowt asm("$3");
            unsigned short idxt;

            idxt = *(unsigned short *)(t3 + 8);
            rowt = (unsigned char *)((idxt * 3 << 1) + (unsigned int)t6->f18);
            curz = *(unsigned short *)(rowt + 4);
            curx = *(unsigned short *)rowt;
            asm volatile("" : "=r"(curx) : "0"(curx));
        } else {
            idx = *(unsigned short *)(t3 + 2);
            row = (unsigned char *)((idx << 2) + (unsigned int)t6->f18);
            curz = *(unsigned short *)(row + 2);
            curx = *(unsigned short *)row;
            asm volatile("" : "=r"(curx) : "0"(curx));
        }
        nz = (short)curz;
        c1 = t4 < nz;
        if (c1)
            goto y_true;
        oz = (short)oldz;
        asm volatile("" : "=r"(oz) : "0"(oz));
        if (t4 < oz)
            goto x_body;
        if (c1 == 0)
            goto next;
    y_true:
        oz = (short)oldz;
        asm volatile("" : "=r"(oz) : "0"(oz));
        if (t4 < oz)
            goto next;
    x_body:
        nx = (short)curx;
        c1 = t2 < nx;
        if (c1 == 0)
            goto x_false;
        ox = (short)oldx;
        asm volatile("" : "=r"(ox) : "0"(ox));
        if (t2 < ox)
            goto toggle;
        if (c1)
            goto cross;
    x_false:
        ox = (short)oldx;
        asm volatile("" : "=r"(ox) : "0"(ox));
        if (!(t2 < ox))
            goto next;
    cross:
        {
            register int ozs asm("$2");
            register int nzs asm("$4");
            register int dy asm("$5");
            register int oxs asm("$3");

            ozs = (short)oldz;
            asm volatile("" : "=r"(ozs) : "0"(ozs));
            nzs = (short)curz;
            asm volatile("" : "=r"(nzs) : "0"(nzs));
            dy = ozs - nzs;
            asm volatile("" : "=r"(dy) : "0"(dy));
            oxs = (short)oldx;
            asm volatile("" : "=r"(oxs) : "0"(oxs));
            {
                register int nxs asm("$2");

                nxs = (short)curx;
                asm volatile("" : "=r"(nxs) : "0"(nxs));
                oxs = oxs - nxs;
                asm volatile("" : "=r"(oxs) : "0"(oxs));
                nzs = t4 - nzs;
                asm volatile("" : "=r"(nzs) : "0"(nzs));
                oxs = oxs * nzs;
                asm volatile("" : "=r"(oxs) : "0"(oxs));
                if (dy < 0) {
                    register int ok asm("$2");

                    asm volatile("" : "=r"(nxs) : "0"(nxs));
                    nxs = t2 - nxs;
                    dy = dy * nxs;
                    ok = oxs < dy;
                    if (!ok)
                        goto next;
                } else {
                    register int ok asm("$2");

                    asm volatile("" : "=r"(nxs) : "0"(nxs));
                    nxs = t2 - nxs;
                    dy = dy * nxs;
                    ok = dy < oxs;
                    if (!ok)
                        goto next;
                }
            }
        }
    toggle:
        t0 = t0 < 1;
    next:
        t5 += 1;
        asm volatile("" : "=r"(t3) : "0"(t3));
        t3 += 2;
    } while (t5 < 3);
    return (int)t0;
}
