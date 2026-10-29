/* room_m0152i - func_80193F64, blob offset 0x3d3c, 0x9C0 bytes. Body twin of
 * room_m0137i func_8019018C / room_m0152i func_80192D24 (only the two local
 * data symbols move: D_8018F1E4 / D_8018F1EC). Flags -O2 -G0, profile
 * room_m0419i_dispatch_8018F1F4_8018F20C_div (both switches fold onto the
 * retail jump tables; the variable divides use the expanded div form).
 * Set up alongside the m0152i copy (Day 4); not yet hit on a route.
 * (Notes from the m0126i copy.) Particle callback of the room's effect set (shared by the m0126i text group:
 * m0126i, m0129i, m0139i, m0149i, m0326i; reached from the battle effect dispatcher at 0x800D413C):
 * mode 1 integrates five particle kinds (kind 0 can trigger the room event and
 * spawns kind-4 sparks), mode 2 sets the effect palette and draws the particle.
 * p[] layout: x,y,z, vy2, vx,vy,vz, (unused), kind, frame; q[1] = scale.
 * Hand-matched (LINK_EXACT). Levers: the effect params at 0x800F3368 are one
 * struct (FxParam; in-struct stores keep retail's store order), the clut index
 * is read first and the clut loaded before the kind/f6E stores, both size
 * computations are two-step, and pos is filled vx, vz, vy. */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern SVECTOR D_8018F1E4;
extern CVECTOR D_8018F1EC;
extern short D_800942EC[];
typedef struct { short w, dw; unsigned short kind; short f6E; unsigned short clut; short f72, f74, h, v; } FxParam;
extern FxParam D_800F3368;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern unsigned short D_800E11E8;
extern unsigned short D_800E11EC;
extern int D_800E27EC;
extern int D_800F3428;
extern int *D_800BCFA4;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char **D_8009D254;
extern int func_80071A54();
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern unsigned short func_80077AA4(int, int);
extern short *func_800CE610();
extern int func_800C6B90(void *, int);
extern void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);
extern void func_800D1DEC(void *, void *, int, int);

int func_80193F64(int mode, short *p, int *q)
{
    SVECTOR sv;
    CVECTOR col;
    SVECTOR pos;
    SVECTOR dir;
    short *e;
    unsigned char *chan;
    int *w;
    int **matrix_slot;
    int scale;
    int size;
    int r;
    int palette;
    int a;
    int ix;
    unsigned short av;

    sv = D_8018F1E4;
    col = D_8018F1EC;
    scale = q[1];
    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[5] += 2;
            if (func_800C6B90(p, scale * 50 / 4096)) {
                if (D_800E2368[0xD]) {
                    chan = D_800F32D0;
                    if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                        *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                        w = **(int ***)(chan + 8);
                        *w = (*w & 0xC0FFFFFF) | 0x11000000;
                        w = **(int ***)(chan + 8);
                        *w |= 0x80000000;
                    }
                }
                p[8] = 1;
                p[9] = 0;
            }
            if (D_800E27EC & 1) {
                r = scale * 48 / 4096;
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[0] = p[0];
                    e[1] = p[1];
                    e[2] = p[2];
                    e[0] += func_80071A54() % r - r / 2;
                    e[1] += func_80071A54() % r - r / 2;
                    e[2] += func_80071A54() % r - r / 2;
                    e[8] = 4;
                    e[3] = 0;
                    e[9] = 0;
                }
            }
            if (p[1] >= D_800942EC[0]) {
                p[8] = 2;
                p[9] = 0;
            }
            break;
        case 1:
            p[9]++;
            p[1] += func_80077CF4(p[9] << 9) / 512;
            if (p[9] >= 8) return 1;
            break;
        case 2:
            p[9]++;
            if (p[9] >= 0x10) return 1;
            break;
        case 3:
            p[9]++;
            p[1] -= 2;
            if (p[9] >= 0x18) return 1;
            break;
        case 4:
            p[9]++;
            p[1] += p[3];
            p[3] += 2;
            if (p[1] >= D_800942EC[0]) {
                p[3] = 0;
                p[1] = D_800942EC[0];
            }
            if (p[9] >= 0x18) return 1;
            break;
        }
        break;
    case 2:
        D_800F3368.w = 0x20;
        D_800F3368.dw = 2;
        D_800F3368.h = 0x20;
        D_800F3368.v = 0x20;
        av = D_800E2850[D_800E11EC];
        D_800F3368.kind = 4;
        D_800F3368.f6E = 0;
        D_800F3368.clut = av;
        switch (p[8]) {
        case 0:
            size = (func_80077DC4(D_800E27EC << 8) / 8 + 0x800) / 2;
            size = scale * size / 4096;
            palette = D_800E1204[D_800F3368.kind];
            if (D_800F3368.kind == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(p, 0, size, size, 0x20, func_80077AA4(0x10, palette), 0xFF, 0x80, &col);
            dir.vx = 0x400;
            dir.vy = 0;
            dir.vz = 0;
            dir.pad = 1;
            pos.vx = p[0];
            pos.vz = p[2];
            pos.vy = D_800942EC[0];
            palette = D_800E1204[D_800F3368.kind];
            if (D_800F3368.kind == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(&pos, &dir, size, size, 0x20, func_80077AA4(0x10, palette), 2, 0x46, 0);
            break;
        case 1:
            palette = D_800E1204[D_800F3368.kind];
            if (D_800F3368.kind == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(p, 0, scale, scale, D_800F3368.dw * p[9], func_80077AA4(0, palette), 1, 0x80, &col);
            break;
        case 2:
            palette = D_800E1204[D_800F3368.kind];
            if (D_800F3368.kind == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(p, &sv, scale, scale, D_800F3368.dw * (p[9] / 2), func_80077AA4(0, palette), 0, 0x80, &col);
            break;
        case 3:
            D_800F3368.w = 0x10;
            D_800F3368.dw = 1;
            D_800F3368.h = 0x10;
            D_800F3368.v = 0x10;
            av = D_800E2850[D_800E11E8];
            D_800F3368.kind = 2;
            D_800F3368.f6E = 0;
            D_800F3368.clut = av;
            a = func_80077DC4((p[9] << 10) / 24) / 32;
            size = (p[9] << 11) / 24 + 0x1000;
            size = scale * size / 4096;
            palette = D_800E1204[D_800F3368.kind];
            if (D_800F3368.kind == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(p, &sv, size, size, D_800F3368.dw * 2 + 0xFD, func_80077AA4(0x80, palette), 1, a, 0);
            break;
        case 4:
            matrix_slot = &D_800BCFA4;
            asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
            gte_SetRotMatrix(*matrix_slot);
            asm volatile("" : : "r"(matrix_slot));
            func_800D1DEC(p, &col, func_80077DC4((p[9] << 10) / 24) / 32, 1);
            break;
        }
        break;
    }
    return 0;
}
