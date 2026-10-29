/* room_m0418i — func_801935A8, blob offset 0x45C0, 0x354 bytes. Flags -O2 -G0.
 * 10-slot quad draw loop (flag byte a2[0x24+i] == 1): build five SVECTORs from
 * the D_8009D254 actor at +0x238, a per-slot D_801994D8 record and a slot-chosen
 * q (+0x240/+0x300/+0x380), then func_800D3114. Sibling of func_801919B0. */

typedef struct { short vx, vy, vz, pad; } SV;
typedef struct { unsigned char b[8]; } CV;
typedef struct { unsigned short x, y, z, w; } REC;
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
extern REC D_801994D8[];
extern unsigned short D_80199658[];
extern char *D_8009D254;
extern short D_800F3374;
extern unsigned char D_801989BC[], D_801989D0[];
extern unsigned char *func_800C2B50();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238(), func_800C3134();
extern void func_800D3114();
extern int func_80077CF4(), func_80077DC4();
extern unsigned short func_80077A64();
extern unsigned short func_80077AA4();

void func_801935A8(int a0, int a1, char *a2)
{
    SV pts[10];
    CV o1;
    CV o2;
    unsigned char *r0;
    char *r;
    char *q;
    unsigned int i;
    int f;
    int c;

    r0 = func_800C2B50();
    q = *(char **)(D_8009D254 + 0x238) + 0x40;
    func_800C2EAC(r0[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    D_800F3374 = 0x1F4;
    for (i = 0; i < 10; i++) {
        f = (a2 + i)[0x24];
        if (f == 1) {
            r = *(char **)(D_8009D254 + 0x238);
            if (i == 0) q = r + 0x240;
            if (i == 1) q = r + 0x300;
            if (i == 2) q = r + 0x380;
            if (i == 3) q = r + 0x300;
            if (i == 4) q = r + 0x380;
            pts[0].vx = W(r, 0x54) + (func_80077CF4(D_80199658[i] + D_801994D8[i].w) >> 5);
            pts[0].vy = W(r, 0x58);
            pts[0].vz = W(r, 0x5C) + (func_80077DC4(D_80199658[i] + D_801994D8[i].w) >> 5);
            pts[1].vx = D_801994D8[i].x + W(r, 0x54);
            pts[1].vy = D_801994D8[i].x + W(r, 0x58);
            pts[1].vz = D_801994D8[i].x + W(r, 0x5C);
            pts[2].vx = D_801994D8[i].y + ((W(r, 0x54) + W(q, 0x14)) >> 1);
            pts[2].vy = D_801994D8[i].y + ((W(r, 0x58) + W(q, 0x18)) >> 1);
            pts[2].vz = D_801994D8[i].y + ((W(r, 0x5C) + W(q, 0x1C)) >> 1);
            pts[3].vx = D_801994D8[i].z + W(q, 0x14);
            pts[3].vy = D_801994D8[i].z + W(q, 0x18);
            pts[3].vz = D_801994D8[i].z + W(q, 0x1C);
            pts[4].vx = W(q, 0x14);
            pts[4].vy = W(q, 0x18);
            pts[4].vz = W(q, 0x1C);
            func_800C3134(D_801989BC, SH(a2 + i * 2, 0xC), &o1);
            func_800C3134(D_801989D0, SH(a2 + i * 2, 0xC), &o2);
            c = func_80077A64(0, 0, 0x340, 0x100);
            func_800D3114(pts, 4, 100, 0x40, 0x80, 0x9C, 0x10, c,
                          func_80077AA4(0, 0x1D7), 0x80, &o2, &o2, 1);
        }
    }
}
