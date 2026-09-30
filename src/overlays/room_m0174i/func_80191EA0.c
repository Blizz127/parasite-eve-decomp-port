/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80191EA0 — blob offset 0x2EB8, 0x110 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Init 16 rain streaks (copy the model block, colours from func_800C2B10/2B28, heights and fades) and register via func_800C6800(o, 1389, pos). */

typedef struct { short x, y, z, pad; } SV;
typedef struct { int w[8]; } Blk;
typedef struct {
    unsigned char flag[16];
    unsigned char pad10[0x10];
    short size[16];
    short alpha[16];
    SV pos[16];
    SV dir[16];
    Blk src;
    short f180, f182, f184;
} Fx;
extern char *func_800C2B50();
extern int *func_800C2B10();
extern int *func_800C2B28();
extern void func_800C6800();
void func_80191EA0(void *o, int a1, Fx *p)
{
    char *r = func_800C2B50();
    unsigned int i;

    p->src = *(Blk *)(r + 4);
    p->f180 = *func_800C2B10(1);
    p->f182 = *func_800C2B28(8);
    p->f184 = 0;
    for (i = 0; i < 16; i++) {
        p->pos[i].x = 0;
        p->pos[i].y = -300;
        p->pos[i].z = i * 120;
        p->dir[i].x = 0;
        p->dir[i].y = 0;
        p->dir[i].z = 0;
        p->alpha[i] = 2048;
        p->size[i] = 96 - i * 3;
        p->flag[i] = 1;
    }
    func_800C6800(o, 1389, p->pos);
}
