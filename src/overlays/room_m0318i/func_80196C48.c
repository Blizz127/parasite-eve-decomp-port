/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m318/RoomEffect_DriftingPalettePulse.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_DriftingPalettePulse` renamed to `func_80196C48`, and vendor symbol
 * names mapped to this repo's address names from the vendor sym tables. */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef struct RoomM318Pulse {
    u16 x, y, z;
    u8 pad06[10];
    s16 delay, age;
} RoomM318Pulse;
extern u16 D_800E11EA, D_800E120A, D_800E2850[];
extern u16 D_800F3368, D_800F336A, D_800F336C, D_800F336E;
extern u16 D_800F3370, D_800F3376, D_800F3378;
int func_80071A54(void);
int func_80077DC4(int);
u16 func_80077AA4 (int, int);
void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);
int func_80196C48(int mode, RoomM318Pulse *pulse) {
    int angle, sine, cosine, size, clut, random;
    register int value asm("$3");
    s16 *settings;
    int clutX;
    register int clutY asm("$5");
    int width;
    int paletteIndex;
    switch (mode) {
    case 1: {
        if (pulse->delay) return 0;
        ++pulse->age;
        random = func_80071A54();
        value = pulse->y - 5;
        pulse->y = value - (random & 7);
        random = func_80071A54();
        value = pulse->x - 3;
        pulse->x = value + (random & 7);
        random = func_80071A54();
        value = pulse->z - 3;
        pulse->z = value + (random & 7);
        if (pulse->age >= 6) return 1;
        break;
    }
    case 2: {
        if (pulse->delay) return 0;
        angle = (pulse->age << 10) / 6;
        sine = func_80077DC4(angle);
        if (sine < 0) sine += 31;
        cosine = func_80077DC4(angle);
        clutX = 16;
        asm("" : "=r"(clutX) : "0"(clutX));
        settings = (s16 *)&D_800F3368;
        asm("" : "=r"(settings) : "0"(settings));
        clutY = D_800E120A;
        width = 32;
        settings[0] = width;
        paletteIndex = D_800E11EA;
        D_800F336A = 2;
        D_800F3376 = width;
        D_800F3378 = width;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[paletteIndex];
        clut = func_80077AA4 (clutX, clutY);
        size = (int)cosine / 2;
        func_800CEE20(pulse, 0, size, size, 104 + settings[1] * (pulse->age & 3), clut, 1, (sine >> 5) / 2, 0);
    }
    }
    return 0;
}
