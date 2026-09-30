/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/task/Task_DispatchCmd.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Task_DispatchCmd` renamed to `func_800130B4`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
int func_80077DC4(int angle);
int func_80077CF4(int angle);
extern u32 D_8009D26C[];
extern u32 D_8009D1F4[];
extern u32 D_8009D1E4[];
extern u32 D_800A7770[];
int func_800130B4(int **args) {
    int mode;
    mode = *args[0];
    switch (mode) {
    case 0: {
        u32 mask;
        u32 state;
        state = D_8009D26C[0];
        mask = *args[1];
        if ((state & mask) == mask) {
            *args[2] = 1;
        } else {
            *args[2] = 0;
        }
        break;
    }
    case 1: {
        u32 mask;
        u32 state;
        state = D_8009D1F4[0];
        mask = *args[1];
        if ((state & mask) == mask) {
            *args[2] = 1;
        } else {
            *args[2] = 0;
        }
        break;
    }
    case 2: {
        u32 mask;
        u32 state;
        state = D_8009D1E4[0];
        mask = *args[1];
        if ((state & mask) == mask) {
            *args[2] = 1;
        } else {
            *args[2] = 0;
        }
        break;
    }
    case 3: {
        u32 mask;
        u32 state;
        int index;
        index = (int)args[1];
        state = D_8009D26C[0];
        mask = *(u32 *)index;
        state &= mask;
        if (state == mask) {
            asm volatile("mtc2 %0,$30" : : "r"( mask )) ;
            index = 31;
            if (state != 0x80000000) {
                asm volatile("swc2 $31,0(%0)" : : "r"( args[1] ) : "memory") ;
                index -= *args[1];
            }
            *args[2] = D_800A7770[index];
        } else {
            *args[2] = 0;
        }
        break;
    }
    }
    return 1;
}
