/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_80190148 — blob offset 0x1160, 0x110 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FFA0; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, pad; } SV;
typedef struct {
    MATRIX m;
    SV pos[6];
    SV vel[6];
    unsigned char pad80[0xC];
    short size[6];
    short act[6];
} Drops;
typedef struct { unsigned char pad[0xA]; short fA; } Lt;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800C42A4();
extern Lt D_80194700;
extern short D_800942EC;
void func_80190148(int a0, int a1, Drops *p)
{
    unsigned int i;

    func_800C2EAC(func_800C2B50()[0x10]);
    func_800C2FF0(16, 16);
    func_800C3098(16);
    func_800C3238(1);
    for (i = 0; i < 6; i++) {
        if (p->act[i] == 1) {
            p->m.t[0] = p->pos[i].x;
            p->m.t[1] = p->pos[i].y;
            p->m.t[2] = p->pos[i].z;
            D_80194700.fA = p->size[i];
            func_800C42A4(&D_80194700, p, 1);
            D_80194700.fA = p->size[i] >> 2;
            p->m.t[1] = D_800942EC;
            func_800C42A4(&D_80194700, p, 1);
        }
    }
}
