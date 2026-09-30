/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC50 — blob offset 0xc68, 0xc4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018FAAC; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

typedef struct { short x, y, z, pad; } SV;
typedef struct {
    SV pos[16];
    SV dir[16];
    short f100, f102;
    unsigned char f104, f105;
    short f106, f108;
} Fx;
extern int *func_800C2B50();
extern short func_80077CF4();
extern short func_80077DC4();
void func_8018FC50(int a0, int a1, Fx *p)
{
    int *r = func_800C2B50();
    unsigned int i;

    for (i = 0; i < 16; i++) {
        p->pos[i].x = r[6];
        p->pos[i].y = r[7] - 100;
        p->pos[i].z = r[8];
        p->dir[i].x = func_80077CF4(i << 9);
        p->dir[i].y = 0;
        p->dir[i].z = func_80077DC4(i << 9);
    }
    p->f102 = 128;
    p->f100 = 1024;
    p->f108 = -60;
    p->f105 = 0;
    p->f106 = 800;
}
