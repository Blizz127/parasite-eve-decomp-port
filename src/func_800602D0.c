/*
 * func_800602D0 - vram 0x800602D0, size 0x258. Draw a signed number with `digits` places:
 * negatives print leading blanks (glyph 0xF) down to the first digit, then the minus string
 * (func_8005DC4C(0x70)) with the draw position pushed/popped on the D_8009D12C stack, then
 * advance x by 9; digits print with leading-zero suppression (0xF blank).
 * era: cc1 2.7.2 -O2 -G8 + MASPSX_EXPAND_DIV=1 (profile era_o2_g8_expand_div).
 * Levers: push/pop block spelled as in func_8005F5B8 (pins $3/$5); x/y read into locals for the
 * `x += 9` rewrite (retail stores both words); separate int d for the digit (sharing the u8 print
 * variable adds an andi); `i >= digits - 1` left inline so loop.c hoists digits-1 into s5 after
 * the loop entry test and reorg puts `i = 0` in the blez slot.
 */
typedef struct {
    int x;
    int y;
} DrawPos;

extern int *D_8009D12C;
extern DrawPos D_8009D124;
extern int D_800A22B0[];
extern int D_800A2270[];
extern unsigned char *func_8005DC4C(int);
extern void func_800527C0(int);
extern void func_8005EED4(int);

void func_800602D0(int n, int digits)
{
    int i;
    int div;
    unsigned char *s;
    unsigned char c;
    register int *t asm("$3");
    register int *r asm("$5");
    int t0;
    int t1;
    int q;
    int d;

    div = 1;
    for (i = 1; i < digits; i++) {
        div *= 10;
    }
    if (n < 0) {
        n = -n;
        div /= 10;
        digits--;
        while (n < div) {
            func_8005EED4(0xF);
            div /= 10;
            digits--;
        }
        s = func_8005DC4C(0x70);
        if (s != 0) {
            r = D_8009D12C;
            if (r < D_800A22B0) {
                t0 = D_8009D124.x;
                t1 = D_8009D124.y;
                t = r + 2;
                D_8009D12C = t;
                r[0] = t0;
                r[1] = t1;
            } else {
                func_800527C0(2);
            }
            c = *s;
            while (c != 0xFF) {
                func_8005EED4(c);
                s++;
                c = *s;
            }
            t = D_8009D12C;
            if (D_800A2270 < t) {
                t0 = t[-2];
                t1 = t[-1];
                D_8009D12C = t - 2;
                D_8009D124.x = t0;
                D_8009D124.y = t1;
            } else {
                func_800527C0(3);
            }
        }
        {
            int x = D_8009D124.x;
            int y = D_8009D124.y;

            D_8009D124.x = x + 9;
            D_8009D124.y = y;
        }
    }
    for (i = 0; i < digits; i++) {
        q = n / div;
        if (i >= digits - 1 || q != 0) {
            d = (unsigned char)(q % 10);
        } else {
            d = 0xF;
        }
        func_8005EED4(d);
        div /= 10;
    }
}
