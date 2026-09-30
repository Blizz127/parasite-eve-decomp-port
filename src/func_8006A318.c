/*
 * func_8006A318 - vram 0x8006A318, size 0x2A4. Positional sound-trigger scan: advances the
 * object position window (+/-1 + step) along its direction, then for the 4 player triggers
 * (when o is the controlled object and no pause/cutscene flag) and the room triggers
 * (D_800B0CD8[0x11] extra entries) whose ids match and whose position lies in the swept
 * window, plays the table sound for the current mode via func_8006DCE4.
 * era: cc1 2.7.2 -O2 -G0 (default profile).
 * Levers: 8-byte Ent table struct; block-local int t for the lo +/- 1 step (retail keeps
 * (lo + 1) + step order; sharing the sound local moves it to $a0).
 */
typedef struct {
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char pos;
    unsigned short snd[2];
} Ent;

extern Ent D_80094488[];
extern unsigned char D_800B0CD8[];
extern unsigned char *D_8009D254;
extern unsigned int D_8009D1A0;
extern void func_8006DCE4(int, int, int, int, int);

int func_8006A318(unsigned char *o)
{
    unsigned char *base;
    int lo;
    int hi;
    int dir;
    int i;
    int p;
    int s;

    if (o == 0) {
        return -1;
    }
    base = D_800B0CD8;
    lo = *(unsigned short *)(o + 0x16);
    dir = *(int *)(o + 0x1C);
    hi = *(unsigned short *)(o + 0x1A);
    if (dir > 0 && lo < hi) {
        int t = lo + 1;

        lo = t + o[0xF];
    } else if (dir < 0 && hi < lo) {
        int t = lo - 1;

        lo = t - o[0xF];
    }
    if (o == D_8009D254 && !(D_8009D1A0 & 2) && !(*(int *)base & 0x800000)) {
        for (i = 0; i < 4; i++) {
            if (D_80094488[i].c == o[0xE]) {
                p = D_80094488[i].pos;
                if ((dir > 0 && p >= hi && p < lo) || (dir < 0 && lo < p && p <= hi)) {
                    s = D_80094488[i].snd[base[0x12]];
                    if (s != 0) {
                        func_8006DCE4(s, 0, *(short *)(o + 0x2A), *(short *)(o + 0x2E), *(short *)(o + 0x32));
                    }
                }
            }
        }
    }
    for (i = 4; i < base[0x11] + 4; i++) {
        if (D_80094488[i].a == o[0xC] && D_80094488[i].b == o[0xD] && D_80094488[i].c == o[0xE]) {
            p = D_80094488[i].pos;
            if ((dir > 0 && p >= hi && p < lo) || (dir < 0 && lo < p && p <= hi)) {
                s = D_80094488[i].snd[base[0x12]];
                if (s != 0) {
                    func_8006DCE4(s, 0, *(short *)(o + 0x2A), *(short *)(o + 0x2E), *(short *)(o + 0x32));
                }
            }
        }
    }
    return 0;
}
