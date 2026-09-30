/* room_m0273i — func_80199F84, blob offset 0xAF9C, 0xE4 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * func_800D0E88 emitter: a1+8/a1+0xA read into locals then asm volatile("") before the s.x store (sched order), *(int)>>21 narrowed by K3 (park 9w -> 0). */

typedef struct { short x, y, z; } SV3;
typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern int D_800966EC[];
extern char D_8019AB70[];
extern void func_800D0E88();

int func_80199F84(int a0, unsigned char *a1)
{
    SV3 s;
    SV r;
    int *v;
    int x;
    int b, c;

    if (a0 == 1) {
        if (D_800E27EC >= 9) {
            return 1;
        }
        *(short *)(a1 + 0xC) += *(short *)(a1 + 0xE);
    } else {
        if (a0 != 2) {
            return 0;
        }
        v = *(int **)(a1 + 4);
        x = v[0];
        s.y = v[1];
        s.z = v[2];
        r.x = 0;
        r.y = *(short *)(a1 + 0xC);
        r.z = 0;
        b = *(short *)(a1 + 8);
        c = *(short *)(a1 + 0xA);
        asm volatile("");
        s.x = x;
        func_800D0E88(&s, &r, b, c, a1, D_8019AB70, D_8019AB70,
                      D_800966EC[((D_800E27EC - 1) << 7) & 0xF80] >> 21, 1);
    }
    return 0;
}
