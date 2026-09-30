/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/boot/Gte_StoreTableEntry.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Gte_StoreTableEntry` renamed to `func_8003EAC8`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
typedef unsigned int u32;
int func_80077DC4(int angle);
int func_80077CF4(int angle);
extern int D_800A76F0[];
void func_8003EAC8(u32 mask, int value) {
    int index;
    asm volatile("mtc2 %0,$30" : : "r"( mask )) ;
    index = 31;
    if (mask != 0x80000000) {
        asm volatile("swc2 $31,0(%0)" : : "r"( &mask ) : "memory") ;
        index -= mask;
    }
    D_800A76F0[index] = value;
}
