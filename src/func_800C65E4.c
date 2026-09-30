typedef struct { short vx; short vy; short vz; } SVec;
typedef struct { int vx; int vy; int vz; int pad; } Vec;
typedef struct { int a; int b; int c; int d; } M16;

extern M16 D_800C213C;
extern void func_80079178(Vec *a0, M16 *a1, M16 *a2);
extern void func_80078120(M16 *a0, void *a1);

void func_800C65E4(SVec *a0, SVec *a1, unsigned char *a2) {
    M16 t1;
    M16 t2;
    Vec d;
    M16 m;

    m = D_800C213C;
    d.vx = a1->vx - a0->vx;
    d.vy = a1->vy - a0->vy;
    d.vz = a1->vz - a0->vz;
    func_80079178(&d, &m, &t1);
    func_80079178(&d, &t1, &t2);
    func_80078120(&t1, a2 + 0x14);
    func_80078120(&t2, a2 + 0x18);
    func_80078120((M16 *)&d, a2 + 0x1C);
}
