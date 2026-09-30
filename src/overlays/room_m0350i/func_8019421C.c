/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_ModelShellLayers.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_ModelShellLayers` renamed to `func_8019421C`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* vendor markers: MASPSX_FLAGS: --expand-div */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_8019421C` (the TU spans several of this repo's yaml functions). */
typedef struct { short x, y, z, pad; } Vector;
typedef struct { short rotation[3][3]; int position[3]; } Matrix;
typedef struct { int color; short x, y, z, size; } Particle;
typedef struct { char reserved[0x238]; Matrix *transforms; } Instance;
typedef struct { signed int size : 16; signed int shade : 16; } TrigEntry;
typedef struct { int reserved[2]; void *pool; } Emitter;
extern Emitter *D_800F33E0;
extern Instance *D_8019A7F8;
extern int D_800E27EC, D_800F3428, D_800966EC[];
extern unsigned short D_800F336C, D_800E1204[], D_800E2850[];
extern short D_800F336A, D_800966EE[];
extern int D_8019A3CC[], D_8019A3C8[];
extern volatile short D_800F3368, D_800F3376, D_800F3378;
extern volatile short D_800F336E, D_800F3372, D_800F3374;
extern volatile unsigned short D_800E11E8, D_800F3370;
extern unsigned short func_80077AA4(int, int);
extern void func_800CEE20(void *, int, int, int, int, int, int, int, void *);
extern void func_800D004C(void *, int, int, int, void *, int, int, void *, void *, int, int);
extern int func_800CE560(void *, int, int, int (*)(int, Particle *));
extern Particle *func_800CE610(void *);
extern int func_80052B2C(void);
extern void func_80079754(Vector *, Matrix *);
extern void func_80078C34(Matrix *, Vector *, Vector *);
int func_8019404C(int event, Particle *particle);
int func_8019421C(int event)
{
    Vector offset;
    Matrix matrix;
    Vector vector;
    if (event == 1) goto update;
    if (event < 2) { if (event == 0) goto setup; goto done; }
    if (event == 2) goto configure;
    goto done;
setup:
    return func_800CE560(D_800F33E0->pool, 12, 64, func_8019404C);
update:
    {
        int i = 0;
        Vector *out;
        if (D_800E27EC >= 41) return 2;
        out = &offset;
        for (; i < 2; i++) {
            register int random asm("$16");
            int r, g, b, divisor;
            int j, *position;
            Particle *particle = func_800CE610(D_800F33E0->pool);
            if (!particle) return 0;
            vector.x = (unsigned int)func_80052B2C() << 4;
            vector.y = (unsigned int)func_80052B2C() << 4;
            vector.z = 0;
            func_80079754(&vector, &matrix);
            asm volatile("" : : "i"(&&offset_start));
offset_start:
            vector.x = 0; vector.y = 0;
            random = func_80052B2C();
            random = ((unsigned int)random << 8) | (unsigned int)func_80052B2C();
            divisor = 192;
            vector.z = random % divisor + 128;
            func_80078C34(&matrix, &vector, out);
            position = D_8019A7F8->transforms[i ? 11 : 15].position;
            for (j = 0; j < 3; j++) (&particle->x)[j] = ((short *)out)[j] + (unsigned int)position[j];
            particle->size = (unsigned int)D_800966EC[((int)((unsigned int)D_800E27EC << 11) / 40) & 4095] + 4096;
            r = func_80052B2C(); g = func_80052B2C(); b = func_80052B2C();
            particle->color = (r >> 1) | ((unsigned int)(g >> 1) << 8) | ((unsigned int)(b >> 1) << 16);
        }
    }
    goto done;
configure:
    {
        int index = D_800E11E8;
        int palette;
        D_800F3368 = 16; D_800F336A = 1;
        D_800F3376 = 16; D_800F3378 = 16;
        D_800F3376 = 16; D_800F3378 = 16;
        palette = D_800E2850[index];
        D_800F336C = 2; D_800F336E = 0;
        D_800F3372 = 0; D_800F3374 = 0; D_800F3370 = palette;
    }
done:
    return 0;
}
