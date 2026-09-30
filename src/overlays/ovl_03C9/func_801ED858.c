/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801ED858 — blob offset 0x60, 0x3EC bytes. Profile era_o2_g0 (default).
 * Effect handler (mode, obj): mode 1 integrates position by a 63/64-damped velocity for
 * 44 steps; mode 2 draws two sprite phases through func_800CEE20. Same family as
 * func_801F1070/func_801F15DC; the phase-1 CLUT compare uses the `register int four asm("$3")`
 * constant (phase 0 reads D_800F336C directly with the natural `li 4`). Evidence:
 * docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    short x, y, z, pad0;
    short vx, vy, vz, pad1;
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Fx;
extern RECT D_801ED7FC;
extern unsigned short D_800F336C;
extern unsigned short D_800F336E;
extern unsigned short D_800F3370;
extern unsigned short D_800F3372;
extern int D_800F3428;
extern int D_800E27EC;
extern unsigned short D_800E11E6;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern unsigned char D_801F1BB0;
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern void func_800CEDA8();
extern void func_800CF3AC();
extern int func_80077CF4();
extern int func_80077DC4();

int func_801ED858(int mode, Fx *o)
{
    RECT r;
    unsigned char buf[8];
    int t;
    int size;
    int c;
    int cy;
    int k;
    register int four asm("$3");

    r = D_801ED7FC;
    switch (mode) {
    case 1:
        if (o->wait >= 2) break;
        if (o->wait < 0) return 0;
        o->step++;
        o->x += o->vx;
        o->y += o->vy;
        o->z += o->vz;
        o->vx = o->vx * 63 / 64;
        o->vy = o->vy * 63 / 64;
        o->vz = o->vz * 63 / 64;
        if (o->step < 0x2C) break;
        return 1;
    case 2:
        switch (o->wait) {
        case 0:
            t = (o->step << 10) / 44;
            c = 0x80 - ((D_800E27EC & 1) << 5);
            func_800CF3AC(&D_801F1BB0, buf, o->step);
            r.w = -D_800E27EC << 4;
            size = func_80077DC4(t) + 0x1000;
            D_800F336C = 1;
            D_800F3370 = D_800E2850[D_800E11E6];
            func_800CEDA8(1);
            D_800F336E = 0;
            D_800F3372 = 0;
            cy = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) {
                cy += 4;
            }
            func_800CEE20(o, &r, size, size, 0x80, func_80077AA4(0x20, cy), 1, c, buf);
            break;
        case 1:
            t = (o->step << 10) / 44;
            c = func_80077CF4(t * 2) / 32;
            r.w = -D_800E27EC << 4;
            size = func_80077CF4(t) / 2 + 0x1000;
            D_800F336C = 1;
            D_800F3370 = D_800E2850[D_800E11F6];
            func_800CEDA8(1);
            D_800F336E = 1;
            D_800F3372 = 5;
            k = D_800F336C;
            cy = D_800E1204[k];
            four = 4;
            if (k == four && D_800F3428 != 0) {
                cy += 7;
            } else {
                cy += 3;
            }
            func_800CEE20(o, &r, size * 3 / 2, size * 3 / 2, 0x40, func_80077AA4(0, cy), 1, c / 2, 0);
            break;
        default:
            return 0;
        }
        break;
    }
    return 0;
}
