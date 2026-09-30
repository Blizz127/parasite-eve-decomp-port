/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m350/RoomEffect_RadialBurst.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_RadialBurst` renamed to `func_80197B98`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* vendor markers: MASPSX_FLAGS: --expand-div */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_80197B98` (the TU spans several of this repo's yaml functions). */
typedef struct { char reserved[0xF4]; int position[3]; } Transform;
typedef struct { char reserved[0x238]; Transform *transform; } Instance;
typedef struct { int reserved[2]; Instance *instance; } Actor;
extern Actor *D_800F32D0;
extern int D_800E27EC, D_800F3428, D_800966EC[], D_8019A634[];
extern unsigned short D_800F336C, D_800E1204[];
extern short D_800F336A;
extern int func_80077AA4(int, int);
extern void func_800CEE20(void *, int, int, int, int, unsigned int, int, int, void *);
int func_80197A04(int event, short *object);
typedef struct { int reserved[2]; void *pool; } Emitter;
typedef struct { short rotation[3][3]; int translation[3]; } Matrix;
typedef struct { short x, y, z, pad; } Vector;
extern Emitter *D_800F33E0;
extern unsigned char D_8019A86E, D_8019A859;
extern unsigned short D_800E11E8, D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336E;
extern unsigned short D_800F336C, D_800F3370;
extern short D_800F3372, D_800F3374;
extern volatile short D_800F3376, D_800F3378;
extern int func_80197A04(int, short *);
extern int func_800CE560(void *, int, int, int (*)(int, short *));
extern short *func_800CE610(void *);
extern int func_80052B2C(void);
extern Matrix *func_80079754(Vector *, Matrix *);
extern Vector *func_80078C34(Matrix *, Vector *, Vector *);
int func_80197B98(int event)
{
    if (event == 1) goto update;
    if (event < 2) {
        if (event == 0) goto setup;
        goto done;
    }
    if (event == 2) goto configure;
    goto done;
setup:
    return func_800CE560(D_800F33E0->pool, 8, 16, func_80197A04);
update:
    if (D_8019A86E) return 2;
    if (D_8019A859) {
        register int i asm("$18") = 0;
        do {
            Matrix matrix;
            Vector vector;
            register int random asm("$16");
            int divisor;
            short *effect = func_800CE610(D_800F33E0->pool);
            if (!effect) break;
            i++;
            vector.x = func_80052B2C() << 4;
            vector.y = func_80052B2C() << 4;
            vector.z = 0;
            func_80079754(&vector, &matrix);
            asm volatile("" : : "i"(&&offset));
offset:
            vector.x = 0;
            vector.y = 0;
            random = func_80052B2C();
            random = (random << 8) | func_80052B2C();
            divisor = 64;
            vector.z = random % divisor + 256;
            func_80078C34(&matrix, &vector, (Vector *)effect);
        } while (i < 2);
        {
            unsigned char *request = &D_8019A859;
            *request = *request - 1;
        }
    }
    goto done;
configure:
    {
        int unit = 16;
        int index = D_800E11E8;
        int palette;
        D_800F3372 = 0;
        D_800F3368 = unit;
        D_800F336A = 1;
        D_800F3376 = unit;
        D_800F3378 = unit;
        D_800F3376 = unit;
        D_800F3378 = unit;
        asm volatile("" : "=r"(index) : "0"(index));
        palette = D_800E2850[index];
        D_800F336C = 2;
        D_800F336E = 0;
        D_800F3374 = 0;
        D_800F3370 = palette;
    }
done:
    return 0;
}
