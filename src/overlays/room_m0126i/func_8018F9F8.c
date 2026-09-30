/* room_m0126i — func_8018F9F8, blob offset 0xA10, 0x2C8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * cutscene event handler (spawn/emitters/sound 0x586); levers: goto shared return 0, struct-typed D_800E2368, store order p0,p1,p2,p8,p9, case 2 breaks to shared return */

typedef struct { unsigned char b[8]; } B8;
typedef struct { short x, y, z, pad; } SV;
typedef struct { unsigned char pad0[0xD]; unsigned char fD; unsigned char pad0E[4]; short f12; } S2368;
extern B8 D_8018F030;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern S2368 *D_800E2368;
extern void *D_8009D254;
extern int D_800E27EC;
extern short D_800F3372, D_800F3374;
extern void func_8018F038();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern void func_800CE870();
extern void func_800CFAA8();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CFB7C();
extern int func_800D3FD8();
extern void func_800D3F64();

int func_8018F9F8(int a0, short *a1, short *a2)
{
    B8 t;
    SV s;
    unsigned char *e;
    unsigned char *y;
    short *p;

    t = D_8018F030;
    switch (a0) {
    case 0:
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x13, &t, a1);
        switch (D_800E2368->f12) {
        case 0:
            func_800CE9D4(*(void **)(D_800F32D0 + 8), 0x13, a1 + 4);
            break;
        case 1:
            func_800CE870(D_8009D254, 0, &s);
            func_800CFAA8(a1, &s, a1 + 4);
            a1[4] = 0x180;
            break;
        }
        if (D_800E2368->fD != 0) {
            e = *(unsigned char **)(D_800F32D0 + 8);
            if (e != 0 && *(unsigned char **)e != 0) {
                y = *(unsigned char **)(*(unsigned char **)e + 0x18);
                if (*y == 1) {
                    *y = 2;
                }
            }
        }
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 0x18, func_8018F038);
    case 1:
        if (D_800E27EC == 1) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = a1[0];
                p[1] = a1[1];
                p[2] = a1[2];
                func_800CFB7C(a1 + 4, *a2, p + 4);
                p[8] = 0;
                p[9] = 0;
            }
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = a1[0];
                p[1] = a1[1];
                p[2] = a1[2];
                p[8] = 3;
                p[9] = 0;
            }
            func_800D3F64(0x586, func_800D3FD8());
        }
        if (D_800E27EC < 2) {
            goto out;
        }
        return 2;
    case 2:
        D_800F3372 = 0;
        D_800F3374 = 8;
        break;
    }
    return 0;
out:
    return 0;
}
