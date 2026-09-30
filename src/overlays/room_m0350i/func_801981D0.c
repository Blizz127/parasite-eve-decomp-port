/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_TransformedFlare.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_TransformedFlare` renamed to `func_801981D0`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
int func_80077DC4(int angle);
int func_80077CF4(int angle);
typedef struct { short x, y, z, pad; } Vector;
typedef struct { short rotation[3][3]; int position[3]; } Matrix;
typedef struct { short y, z, shade; } Particle;
typedef struct { char reserved[0x3A]; short yaw; } Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
extern Actor *D_800F32D0;
extern Matrix D_8019A870;
extern short D_8019A890, D_8019A892, D_8019A894;
extern int D_800E27EC, D_800966EC[], D_800F3428, D_8019A690[];
extern unsigned short D_800F336C, D_800E1204[];
extern short D_800F336A;
extern unsigned short func_80077AA4(int, int);
extern void func_800CEE20(Vector *, Vector *, int, int, int, int, int, int, void *);
int func_801981D0(int event, Particle *particle)
{
    Vector position, rotation;
    if (event == 1) {
        int timer = D_800E27EC;
        if (timer >= 8) return 1;
        particle->shade = (short)D_800966EC[((unsigned int)timer << 8) & 0xF00] >> 5;
    } else if (event == 2) {
        int kind, palette;
        unsigned short clut;
        position.x = 0;
        position.y = particle->y;
        position.z = particle->z;
        {
            register Matrix *matrix asm("$8") = &D_8019A870;
            Vector *out;
            asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( matrix ) : "$12", "$13", "$14") ;
            asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( matrix ) : "$12", "$13", "$14") ;
            out = &position;
            asm volatile("lwc2 $0,0(%0)"  "\n\t" "lwc2 $1,4(%0)"  : : "r"( out ) : "memory") ;
            asm volatile("nop\n\t" "nop\n\t" ".word 0x4A480012") ;
            asm volatile("" : "=r"(out) : "0"(out));
            {
                register int x asm("$12");
                register int y asm("$13");
                register int z asm("$14");
                asm volatile("mfc2 %0,$9" : "=r"( x )) ; asm volatile("mfc2 %0,$10" : "=r"( y )) ; asm volatile("mfc2 %0,$11" : "=r"( z )) ;
                out->x = x; out->y = y; out->z = z;
            }
        }
        position.x += D_8019A890;
        position.y += D_8019A892;
        position.z += D_8019A894;
        rotation.x = (unsigned int)D_800E27EC * 384;
        rotation.y = D_800F32D0->instance->yaw;
        rotation.z = 0;
        rotation.pad = 1;
        kind = D_800F336C;
        palette = D_800E1204[kind];
        if (kind == 4 && D_800F3428) palette += 4;
        clut = func_80077AA4(0, palette);
        {
            int timer = D_800E27EC;
            register int scale asm("$4") = D_800F336A;
            register int phase asm("$3") = timer & 7;
            register int product asm("$8") = scale * phase;
            int texture = product + 64;
            asm("" : : "r"(product), "r"(texture));
            func_800CEE20(&position, &rotation, 0x2200, 4096,
                texture, clut, 1, particle->shade, &D_8019A690[timer & 3]);
        }
    }
    return 0;
}
