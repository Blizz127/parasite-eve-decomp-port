/* room_m0418i — func_801905F8, blob offset 0x1610, 0xC4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * 4-slot timer/fade state machine; lever: post-increment via named old value c */

typedef struct {
    unsigned char pad0[0x28];
    short h[4];
    unsigned char pad30[8];
    unsigned char cnt[4];
    unsigned char st[4];
    unsigned char tm[4];
    unsigned char n;
} W;

void func_801905F8(void *a0, unsigned char *a1, W *a2)
{
    unsigned int i;
    short t;
    unsigned char c;

    for (i = 0; i < 4; i++) {
        if (a2->tm[i] != 0) {
            if (--a2->tm[i] == 0) {
                a2->st[i] = 1;
            }
        }
        if (a2->st[i] == 1) {
            t = a2->h[i];
            if (t > 0x10) {
                a2->h[i] = t - 0x10;
            }
            c = a2->cnt[i];
            a2->cnt[i] = c + 1;
            if (c == 8) {
                a2->st[i] = 0;
                if (--a2->n == 0) {
                    a1[1] = 2;
                }
            }
        }
    }
}
