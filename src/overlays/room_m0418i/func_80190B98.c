/* room_m0418i — func_80190B98, blob offset 0x1BB0, 0x374 bytes. Flags -O2 -G0.
 * Countdown at +0xA8 starts state +0xA9; state 12 spawns (func_800C2B90 mode 3)
 * at the object position and sets a1[1] = 2. Then interpolates the +0x68..+0x94
 * shorts between +0x10 and +0x0 by the state step and jitters 8 points at +0x28. */

typedef struct { short vx, vy, vz, pad; } SV;
typedef struct {
    short x, y, z;
    unsigned char pad06[0x0A];
    short x0, y0, z0;
    unsigned char pad16[0x12];
    SV pt[8];
    short a68, a6A, a6C, a6E;
    short a70, a72, a74, a76;
    short a78, a7A, a7C, a7E;
    short a80, a82, a84, a86;
    short a88, a8A, a8C, a8E;
    short a90, a92, a94, a96;
    unsigned char pad98[0x0C];
    int cnt;
    unsigned char timer;
    unsigned char state;
} OBJ;
extern short D_800942EC[];
extern unsigned char D_80198754[], D_80198718[];
extern char *func_800C2B50();
extern short *func_800C2B90();
extern int func_800C6B90();
extern int func_80071A54();

void func_80190B98(int a0, unsigned char *a1, OBJ *o)
{
    SV d;
    char *r;
    short *p;
    unsigned int i;

    r = func_800C2B50();
    if (o->timer != 0) {
        if (--o->timer == 0) {
            o->state = 1;
            func_800C2B90(a0, 1, D_80198754, D_80198718);
        }
    }
    if (o->state != 0) {
        o->cnt++;
        if (++o->state == 12) {
            p = func_800C2B90(a0, 3, D_80198754, D_80198718);
            if (p != 0) {
                p[0] = o->x;
                p[1] = o->y;
                p[2] = o->z;
                if (func_800C6B90(p, 200) != 0) {
                    *(short *)(r + 0x2C) = 1;
                }
            }
            a1[1] = 2;
        }
    }
    d.vx = (o->x - o->x0) >> 4;
    d.vy = (o->y - o->y0) >> 4;
    d.vz = (o->z - o->z0) >> 4;
    o->a78 = o->x0 + d.vx * (o->state - 2);
    o->a7A = o->y0 + d.vy * (o->state - 2);
    o->a7C = o->z0 + d.vz * (o->state - 2);
    o->a68 = o->a78 + d.vx * (o->state - 1);
    o->a6A = o->a7A + d.vy * (o->state - 1);
    o->a6C = o->a7C + d.vz * (o->state - 1);
    o->a70 = (o->a68 + o->a78) >> 1;
    o->a72 = (o->a6A + o->a7A) >> 1;
    o->a74 = (o->a6C + o->a7C) >> 1;
    o->a80 = o->a68;
    o->a82 = D_800942EC[0];
    o->a84 = o->a6C;
    o->a88 = o->a70;
    o->a8A = D_800942EC[0];
    o->a8C = o->a74;
    o->a90 = o->a78;
    o->a92 = D_800942EC[0];
    o->a94 = o->a7C;
    for (i = 0; i < 8; i++) {
        o->pt[i].vx = o->a78 - (d.vx << 2) + func_80071A54() % 100 - 50;
        o->pt[i].vy = o->a7A;
        o->pt[i].vz = o->a7C - (d.vz << 2) + func_80071A54() % 100 - 50;
    }
}
