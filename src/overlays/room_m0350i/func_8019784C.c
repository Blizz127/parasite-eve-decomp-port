/* room_m0350i — func_8019784C, blob offset 0x8864, 0x1B8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Queued-vector flush controller; table+count as one struct (in_struct count load keeps the last store in the loop). */

extern unsigned char *D_800F33E0;
extern unsigned char D_8019A86E;
extern struct { struct { unsigned short x, y, z, pad; } t[5]; short n; } D_8019A834;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern void func_8019A014();
extern int func_800CE560();
extern short *func_800CE610();

int func_8019784C(int a0)
{
    short *e;
    int i;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 0x10, func_8019A014);
    case 1:
        if (D_8019A86E != 0) {
            return 2;
        }
        for (i = 0; i < D_8019A834.n; i++) {
            e = func_800CE610(((void **)D_800F33E0)[2]);
            if (e == 0) {
                break;
            }
            *e = D_8019A834.t[i].x;
            *(e + 1) = D_8019A834.t[i].y;
            *(e + 2) = D_8019A834.t[i].z;
        }
        D_8019A834.n = 0;
        return 0;
    case 2:
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
