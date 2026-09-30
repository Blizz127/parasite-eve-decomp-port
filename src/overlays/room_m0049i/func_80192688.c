/* room_m0049i — func_80192688, blob offset 0x36A0, 0x10C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * particle spawn at model node offset (func_80078C34 transform of D_8018F01C); levers: statement order, rand() result held before the +0x12 store */

typedef struct { unsigned char b[8]; } B8;
typedef struct { short x, y, z, pad; } SV;
extern B8 D_8018F01C;
extern unsigned char **func_800C2B50();
extern void func_80078C34();
extern int func_80071A54();

void func_80192688(void *a0, void *a1, short *a2)
{
    unsigned char *p;
    B8 t;
    SV o;

    p = *(unsigned char **)(*func_800C2B50() + 0x238);
    t = D_8018F01C;
    func_80078C34(p + 0x1A0, &t, &o);
    if (a2[8] != 0) {
        return;
    }
    a2[10] = 0;
    a2[0] = o.x + *(int *)(p + 0x1B4);
    a2[1] = o.y + *(int *)(p + 0x1B8);
    a2[2] = o.z + *(int *)(p + 0x1BC);
    a2[4] = 0;
    a2[5] = -8;
    a2[6] = 0;
    {
        int r = func_80071A54();
        a2[9] = 150;
        a2[8] = r % 80 + 0x50;
    }
}
