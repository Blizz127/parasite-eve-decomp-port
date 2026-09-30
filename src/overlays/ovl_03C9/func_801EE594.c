typedef struct { short x, y, w, h; } RECT;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct {
    short x, y, z, pad0;
    short vx, vy, vz, pad1;
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Fx;
extern RECT D_801ED7FC;
typedef struct { short s68, s6A; unsigned short s6C, s6E, s70, s72, s74; short s76, s78; } Ctl;
extern Ctl D_800F3368x asm("D_800F3368");
#define D_800F3368 D_800F3368x.s68
#define D_800F336A D_800F3368x.s6A
#define D_800F336C D_800F3368x.s6C
#define D_800F336E D_800F3368x.s6E
#define D_800F3370 D_800F3368x.s70
#define D_800F3376 D_800F3368x.s76
#define D_800F3378 D_800F3368x.s78
extern int D_800F3428;
extern int D_800E27EC;
extern unsigned short D_800E11E6;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern short D_800942EC[];
extern unsigned char D_801F1C28;
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern void func_800CEDA8();
extern void func_800CF3AC();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80071A54();

int func_801EE594(int mode, Fx *o)
{
    RECT r;
    unsigned char buf[8];
    SVECTOR p;
    RECT r2;
    int s;
    int t;
    int size;
    int c;
    int cy;
    int k;
    register int four asm("$3");

    r = D_801ED7FC;
    switch (mode) {
    case 1:
        switch (o->wait) {
        case 0:
            o->step++;
            o->x += o->vx;
            o->y += o->vy;
            o->z += o->vz;
            o->vx = o->vx * 127 / 128;
            o->vz = o->vz * 127 / 128;
            o->vy--;
            if (o->y >= D_800942EC[0]) {
                o->vy *= -1;
            }
            if (o->step < 0x10) break;
            return 1;
        case 1:
            o->step++;
            o->y += -5 - (func_80071A54() & 7);
            o->x += (func_80071A54() & 7) - 3;
            o->z += (func_80071A54() & 7) - 3;
            if (o->step < 0x10) break;
            return 1;
        case 2:
            if (++o->step < 0x12) break;
            return 1;
        default:
            return 0;
        }
        break;
    case 2:
        switch (o->wait) {
        case 0:
            s = o->step;
            func_800CF3AC(&D_801F1C28, buf, s * 2);
            r.w = o->x + (D_800E27EC << 5);
            size = func_80077CF4(s << 7);
            D_800F3368 = 0x10;
            D_800F336A = 1;
            D_800F3376 = 0x10;
            D_800F3378 = 0x10;
            D_800F3370 = D_800E2850[D_800E11E6];
            D_800F336C = 1;
            func_800CEDA8(1);
            D_800F336E = 0;
            cy = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) {
                cy += 4;
            }
            func_800CEE20(o, &r, size, size * 3 / 2, 0x8D, func_80077AA4(0x30, cy), 1, 0x80, buf);
            break;
        case 1:
            s = o->step << 6;
            c = func_80077DC4(s) / 32;
            r.w = D_800E27EC << 5;
            size = func_80077DC4(s);
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            D_800F3370 = D_800E2850[D_800E11E6];
            D_800F336C = 1;
            func_800CEDA8(1);
            D_800F336E = 0;
            cy = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) {
                cy += 4;
            }
            func_800CEE20(o, 0, size / 2, size / 2, D_800F336A * (o->step & 3) + 0x68, func_80077AA4(0x10, cy), 3, c, 0);
            break;
        case 2:
            t = (o->step << 10) / 18;
            c = func_80077CF4(t * 2) / 32;
            size = func_80077CF4(t);
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            D_800F3370 = D_800E2850[D_800E11F6];
            D_800F336C = 1;
            func_800CEDA8(1);
            D_800F336E = 1;
            r2.x = 0x400;
            r2.y = 0;
            r2.w = o->step * 48;
            r2.h = 1;
            p.vx = o->x;
            p.vz = o->z;
            p.vy = D_800942EC[0];
            k = D_800F336C;
            cy = D_800E1204[k];
            four = 4;
            if (k == four && D_800F3428 != 0) {
                cy += 7;
            } else {
                cy += 3;
            }
            func_800CEE20(&p, &r2, size * 3, size * 3, 0x44, func_80077AA4(0, cy), 1, c / 2, 0);
            break;
        default:
            return 0;
        }
        break;
    }
    return 0;
}
