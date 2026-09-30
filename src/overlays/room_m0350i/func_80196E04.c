/* room_m0350i — func_80196E04, blob offset 0x7E1C, 0x128 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Room state checker: short h (sext at use), nested pointer read without a named outer local, asm output barrier on p before the *p - 1 RMW (retail reload; the stray 8-byte frame disappears) (ovl park 37w -> 0). */

extern unsigned char *D_800F32D0;
extern short D_8019A828;
extern unsigned char D_8019A82E, D_8019A82F, D_8019A830;

int func_80196E04(int a0)
{
    unsigned char *e = *(unsigned char **)(D_800F32D0 + 8);
    unsigned char *x;
    short *p;
    short h;

    if (a0 == 0) {
        D_8019A828 = 4;
        D_8019A82E = 0;
        D_8019A82F = 0;
        D_8019A830 = 0;
    } else if (a0 == 1) {
        if (*(unsigned char **)e != 0) {
            x = *(unsigned char **)(*(unsigned char **)e + 0x18);
            if (*x == a0) {
                *x = 2;
            }
        }
        h = *(unsigned short *)(e + 0x16);
        if (e[0xE] == 9) {
            p = &D_8019A828;
            if (*p > 0) {
                if (h >= 0x16) {
                    *(int *)(e + 0x14) = 0x30000;
                    asm("" : "=r"(p) : "0"(p));
                    *p = *p - 1;
                }
            } else if (h >= e[0xF] - 1) {
                if (*(unsigned char **)e != 0) {
                    **(unsigned char **)(*(unsigned char **)e + 0x18) = 4;
                }
                D_8019A82E = 1;
                return 1;
            }
        }
    }
    return 0;
}
