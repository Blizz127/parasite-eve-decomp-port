/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/gpu/Gte_LightingState.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Gte_SetLightColor` renamed to `func_8003F798`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_8003F798` (the TU spans several of this repo's yaml functions). */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
void func_8003F758(u16 *dst, int r, int g, int b);
void func_8003F798(u8 *state, int index, int r, int g, u16 b) {
    u8 *byte_slot;
    register u32 m0 asm("t4");
    register u32 m1 asm("t5");
    register u32 m2 asm("t6");
    if ((unsigned int)index < 3) {
        *(u16 *)(state + 0x20 + index * 2) = r;
        *(u16 *)(state + 0x26 + index * 2) = g;
        *(u16 *)(state + 0x2C + index * 2) = b;
    }
    byte_slot = state + index * 4;
    byte_slot[0x40] = r;
    byte_slot[0x41] = g;
    byte_slot[0x42] = b;
    asm volatile("" : : : "memory");
    {
        register u32 *matrix asm("v0") = (u32 *)(state + 0x20);
        asm volatile("" : "=r"(matrix) : "0"(matrix));
        m0 = matrix[0];
        m1 = matrix[1];
        asm volatile("ctc2 %0,$16" : : "r"(m0));
        asm volatile("ctc2 %0,$17" : : "r"(m1));
        m0 = matrix[2];
        m1 = matrix[3];
        m2 = matrix[4];
        asm volatile("ctc2 %0,$18" : : "r"(m0));
        asm volatile("ctc2 %0,$19" : : "r"(m1));
        asm volatile("ctc2 %0,$20" : : "r"(m2));
    }
}
