/* room_m0273i — func_80198B1C, blob offset 0x9B34, 0x1B8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * CEE20 + D004C pair: short s0/s1 (sext at use), (unsigned)(b+0x1000)>>5 for srl, o/fence/four k==4 lever, pad[2]; first try. */

typedef struct { short x, y, z, w; } SV;
typedef struct { short s, c; } SC;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern SC D_800966EC[];
extern unsigned short D_800942EC;
extern char D_8019AB68[];
extern char D_8019AE18[];
extern char D_8019AB70[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();

int func_80198B1C(int a0, SV *a1)
{
    SV c;
    int pad[2];
    int t;
    int a;
    int b;
    short s0;
    short s1;
    int k;
    int o;
    int u;
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 0x20) {
            return 1;
        }
    } else if (a0 == 2) {
        t = (D_800E27EC - 1) << 5;
        b = D_800966EC[(t + 0x400) & 0xFFF].c;
        a = D_800966EC[t & 0xFFF].c;
        c = *a1;
        k = D_800F336C;
        s0 = (unsigned int)(b + 0x1000) >> 5;
        s1 = a * 2 + 0x1000;
        o = k << 1;
        asm volatile("");
        four = 4;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (k == four && D_800F3428 != 0) {
            u += 8;
        } else {
            u += 4;
        }
        func_800CEE20(&c, 0, s1, s1, 0x64, func_80077AA4(0, u), 1, s0, 0);
        c.y = D_800942EC;
        func_800D004C(&c, 0x180, 0x180, 8, D_8019AB68, 0x1000, 0x1000, D_8019AE18, D_8019AB70, s0, 1);
    }
    return 0;
}
