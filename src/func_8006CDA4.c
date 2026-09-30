typedef struct {
    int base;
    unsigned short off[64];
} FileTbl;

extern FileTbl D_8009317C;
extern unsigned char D_800B0CD8[];
extern int D_8009D170;
extern int D_8009D174;
extern int D_8009D178;
extern int D_8009D17C;
extern int func_80087198(void);
extern int func_80087414(void);
extern int func_8006E6D4(int, int, int);
extern int func_8006E7E8(void);
extern int func_80087090(int, int);
extern int func_800871AC(int, int);
extern int func_800875FC(int, int);
extern int func_80087428(int, int, int);
extern int func_800870E0(void);

int func_8006CDA4(int mode, int idx, int buf, int dst, unsigned int maxchunk, int block_)
{
    register int block asm("$19") = block_;
    register unsigned char *st asm("$18");
    int r;
    register int done asm("$17");
    register int stop asm("$16");
    int n;
    int rem;
    unsigned short *base;

    st = D_800B0CD8;
    base = (unsigned short *)&D_8009317C;
    r = -1;
    done = 0;
    stop = 0;
    do {
        switch (st[0xF0]) {
        case 0:
            {
            unsigned short *p = base + idx;
            register int nn asm("$5");
            nn = p[3];
            D_8009D170 = *(int *)(st + 0x100) + (D_8009317C.base + D_8009317C.off[idx]);
            nn = nn - D_8009317C.off[idx];
            D_8009D174 = nn;
            D_8009D178 = nn;
            }
            if (mode == 0) {
                r = func_80087198();
            } else if (mode == 3) {
                r = func_80087414();
            } else {
                goto set7;
            }
            if (r == -1) {
                done = 1;
                stop = (block ^ 1) & 1;
            }
        set7:
            st[0xF0] = 7;
            break;
        case 7:
            rem = D_8009D178;
            if (rem != 0) {
                {
                register int ch asm("$7");
                asm("" : "=r"(ch) : "0"(rem));
                if (maxchunk < (unsigned int)ch) {
                    ch = maxchunk;
                }
                D_8009D17C = ch;
                }
                r = func_8006E6D4(D_8009D170, D_8009D174 - rem, dst);
                if (r != -1) {
                    st[0xF0] = 8;
                }
                goto busy;
            }
            st[0xF0] = 0;
            done = 0;
            stop = 1;
            break;
        case 8:
            r = func_8006E7E8();
            if (r == -1) {
                st[0xF0] = 7;
                goto busy;
            }
            if (r != 0) {
                goto busy;
            }
            st[0xF0] = 9;
            break;
        case 9:
            if (mode == 1) {
                goto m1;
            }
            if (mode == 0) {
                goto m0;
            }
            if (mode == 2) {
                goto m2;
            }
            if (mode == 3) {
                goto m3;
            }
            goto mdone;
        m0:
            r = func_800871AC(dst, D_8009D17C << 11);
            goto mdone;
        m1:
            r = func_80087090(dst, 0);
            goto mdone;
        m2:
            r = func_800875FC(buf, dst);
            goto mdone;
        m3:
            r = func_80087428(buf, dst, D_8009D17C << 11);
        mdone:
            if (r == -1) {
                asm volatile("");
                st[0xF0] = 0;
                goto busy;
            }
            st[0xF0] = 10;
            break;
        case 10:
            r = func_800870E0();
            if (r == -1) {
                st[0xF0] = 0;
                goto busy;
            }
            if (r != 0) {
            busy:
                done = 1;
                stop = (block ^ 1) & 1;
                break;
            }
            done = 1;
            stop = (block ^ 1) & 1;
            {
                int left = D_8009D178 - D_8009D17C;
                register int seven asm("$3") = 7;

                st[0xF0] = seven;
                D_8009D178 = left;
            }
            break;
        }
    } while (stop == 0);
    return done;
}
