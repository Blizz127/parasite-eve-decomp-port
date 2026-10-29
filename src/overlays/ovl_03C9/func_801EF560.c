/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801EF560 — blob offset 0x1D68, 0x418 bytes. Profile era_o2_g0 (default).
 * Effect handler (mode, obj), same family as func_801ED858/func_801F0BA0: mode 1 integrates
 * position by velocity for 24 steps (phase 0: 127/128 damping; phase 1: 31/32 damping on
 * vx/vz, vy += 1 gravity and a bounce (vy = -vy) once y reaches D_800942EC); mode 2 draws the
 * step-scaled sprite through func_800CEE20 (phase 0: rcos-style size * o->scale >> 12;
 * phase 1: size*2, func_80077CF4(2t)/32 colour) with the CLUT row from
 * D_800E1204[D_800F336C] (+4 more for area 4 when D_800F3428).
 * Levers: per-phase 'if (o->step < 0x18) break; return 1;' (shared cross-jumped tail);
 * D_800942EC as an array element so its load stays after the vy store; 'o->vy *= -1'
 * keeps retail's sll/sra sign-extension before negu; phase-0 CLUT as
 * 'k != 4 || !D_800F3428' with a separate row local (+6 lands in the bnez delay slot);
 * phase-1 k/four pinned to $4/$3 (retail's allocation, cf. func_801ED858's four pin). */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    short x, y, z, scale;
    short vx, vy, vz, pad1;
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Fx;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern short D_800942EC[];
extern unsigned short D_800E1204[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern int func_80077CF4();
extern int func_80077DC4();

int func_801EF560(int mode, Fx *o)
{
    RECT r;
    int t;
    int size;
    int c;
    int cy;
    int k;
    int row;
    register int k2 asm("$4");
    register int four asm("$3");

    switch (mode) {
    case 1:
        switch (o->wait) {
        case 0:
            o->step++;
            o->x += o->vx;
            o->y += o->vy;
            o->z += o->vz;
            o->vx = o->vx * 127 / 128;
            o->vy = o->vy * 127 / 128;
            o->vz = o->vz * 127 / 128;
            if (o->step < 0x18)
                break;
            return 1;
        case 1:
            o->step++;
            o->x += o->vx;
            o->y += o->vy;
            o->z += o->vz;
            o->vx = o->vx * 31 / 32;
            o->vz = o->vz * 31 / 32;
            o->vy++;
            if (o->y >= D_800942EC[0])
                o->vy *= -1;
            if (o->step < 0x18)
                break;
            return 1;
        default:
            return 0;
        }
        break;
    case 2:
        switch (o->wait) {
        case 0:
            t = (o->step << 10) / 24;
            size = func_80077CF4(t) + 0x800;
            size = size * o->scale / 4096;
            c = func_80077DC4(t) / 32 + 0x28;
            r.x = 0;
            r.y = 0;
            r.w = o->step * 32;
            r.h = 0;
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            k = D_800F336C;
            row = D_800E1204[k];
            if (k != 4 || D_800F3428 == 0) {
                cy = row + 2;
            } else {
                cy = row + 6;
            }
            func_800CEE20(o, &r, size, size, 6, func_80077AA4(0, cy), 1, c, 0);
            break;
        case 1:
            t = (o->step << 10) / 24;
            size = func_80077CF4(t) + 0x1000;
            c = func_80077CF4(t * 2) / 32;
            r.x = 0;
            r.y = 0;
            r.w = o->step * 14;
            r.h = 0;
            D_800F3368 = 0x40;
            D_800F336A = 4;
            D_800F3376 = 0x40;
            D_800F3378 = 0x40;
            k2 = D_800F336C;
            cy = D_800E1204[k2];
            four = 4;
            if (k2 == four && D_800F3428 != 0) {
                cy += 5;
            } else {
                cy += 1;
            }
            func_800CEE20(o, &r, size * 2, size * 2, 0, func_80077AA4(0, cy), 1, c, 0);
            break;
        default:
            return 0;
        }
        break;
    }
    return 0;
}
