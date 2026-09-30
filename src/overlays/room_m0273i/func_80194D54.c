/* room_m0273i — func_80194D54, blob offset 0x5D6C, 0x118 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event poll: area-type 9/0x10 proximity tests set D_8019AE9A, else func_8006DCE4 trigger; lever: if(b!=9){...}else-chain layout */

extern unsigned char *D_800F32D0;
extern unsigned char D_8019AE9A;
extern void func_8006DCE4();

int func_80194D54(int a0)
{
    unsigned char *e;
    unsigned char *x;
    unsigned char *m;
    unsigned short h;
    unsigned short g;

    if (a0 != 1) {
        if (a0 < 2) {
            if (a0 == 0) {
                D_8019AE9A = 0;
                return 0;
            }
            return 0;
        }
    } else {
        e = *(unsigned char **)(D_800F32D0 + 8);
        x = *(unsigned char **)(*(unsigned char **)e + 0x18);
        h = *(unsigned short *)(e + 0x16);
        g = *(unsigned short *)(e + 0x1A);
        if (*x == a0) {
            *x = 2;
        }
        if (e[0xE] != 9) {
            if (e[0xE] == 0x10) {
                if (h >= 0x23 && h <= 0x3B) {
                    D_8019AE9A = 1;
                }
            }
        } else if ((short)h > 0x20) {
            D_8019AE9A = 1;
        } else if ((short)h > 0 && (short)g <= 0) {
            m = *(unsigned char **)(e + 0x238);
            func_8006DCE4(0x5CF, *(void **)(*(unsigned char **)e + 8), *(short *)(m + 0x594),
                          *(short *)(m + 0x598), *(short *)(m + 0x59C));
        }
        if (D_8019AE9A != 0) {
            return 1;
        }
    }
    return 0;
}
