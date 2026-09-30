/* room_m0350i — func_801996BC, blob offset 0xA6D4, 0x1D0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Twin of room_m0428i func_80193A58 (four $4 / o $3 pins); constants 0x80/1/0x180. */

typedef struct { short x, y, z, w; } SV;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned char *D_800F32D0;
extern char D_8019A738[];
extern char D_8019A734[];
extern int func_80077CF4();
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();

int func_801996BC(int a0, short *a1)
{
    SV s;
    short v[4];
    int u;
    int k;
    register int o asm("$3");
    register int four asm("$4");

    if (a0 == 1) {
        a1[9] = (func_80077CF4(D_800E27EC << 4) * a1[10]) >> 12;
        if (D_800E27EC >= 8) {
            a1[8] -= 8;
            if (a1[8] < 0x10) {
                return 1;
            }
        }
    } else if (a0 == 2) {
        k = D_800F336C;
        four = 4;
        o = k << 1;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (k == four && D_800F3428 != 0) {
            u += 7;
        } else {
            u += 3;
        }
        func_800CEE20(a1, &a1[4], a1[9], a1[9], 0x80, func_80077AA4(0, u), 1, a1[8], 0);
        s.x = a1[0];
        s.y = (*(int **)(D_800F32D0 + 8))[0x200 / 4];
        s.z = a1[2];
        v[0] = 0x400;
        v[1] = D_800E27EC * 192;
        v[3] = 1;
        v[2] = 0;
        func_800D004C(&s, 0x180, 0x180, 0xA, v, a1[9], a1[9], D_8019A738, D_8019A734, a1[8], 1);
    }
    return 0;
}
