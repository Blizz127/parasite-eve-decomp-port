/* room_m0318i — func_80192620, blob offset 0x3638, 0xF8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event callback: jitter vector / project and draw via func_800CF3AC+func_800D1DEC */

typedef struct { short x, y, z; } SV3;
typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern char D_80199464[];
extern int func_80071A54();
extern void func_800CF3AC();
extern void func_800D1DEC();

int func_80192620(int a0, short *a1)
{
    SV3 v;
    SV w;

    switch (a0) {
    case 1:
        a1[0] += (func_80071A54() & 7) - 3;
        a1[1] += (func_80071A54() & 3) - 7;
        a1[2] += (func_80071A54() & 7) - 3;
        if (D_800E27EC >= 0x12) {
            return 1;
        }
        break;
    case 2:
        v.x = a1[0];
        v.y = a1[1];
        v.z = a1[2];
        func_800CF3AC(D_80199464, &w, D_800E27EC);
        func_800D1DEC(&v, &w, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
