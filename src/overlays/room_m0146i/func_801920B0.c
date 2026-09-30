/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m146/func_801920B0.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_801920B0` renamed to `func_801920B0`, and vendor symbol
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
extern char *D_800F32D0;
extern char *D_800E2368;
extern s16 D_800942EC  ;
extern int D_80192648 ;
extern int D_8019264C ;
extern int D_80192650 ;
extern int D_80192654 ;
extern int D_80192658 ;
extern int D_8019265C ;
extern int D_80192660 ;
extern int D_80192664 ;
int func_8003010C(void *object, int id);
int func_801920B0  (int mode, char *state) {
    char *slot;
    register char *workState asm("$16");
    register unsigned char *phaseState asm("$4");
    unsigned char *completionState;
    register int frame asm("$3");
    unsigned int flags;
    register int activity asm("$2");
    register int motion asm("$4");
    register unsigned short phase asm("$2");
    register unsigned short phaseCopy asm("$3");
    register int reversedMotion asm("$2");
    register int lastConfig asm("$2");
    slot = *(char **)(D_800F32D0 + 8);
    workState = state;
    if (mode == 0) {
        *(char **)(D_800E2368 + 0x14) = workState;
        **(unsigned int **)slot |= 0x40000000;
        frame = D_800942EC  ;
        *(s16 *)(workState + 0x32) = 0x1A;
        *(s16 *)(workState + 0x30) = 0;
        *(s16 *)(workState + 0x34) = 0;
        *(u8 *)(workState + 0x3A) = 0;
        *(u8 *)(workState + 0x3B) = 0;
        *(int *)(workState + 0x28) = -1;
        *(int *)(workState + 0x0C) = frame;
        *(int *)(workState + 0x10) = D_80192648 ;
        *(int *)(workState + 0x1C) = D_8019264C ;
        *(int *)(workState + 0x20) = D_80192650 ;
        *(int *)(workState + 0x24) = D_80192654 ;
        *(int *)(workState + 0x14) = D_80192658 ;
        *(int *)(workState + 0x18) = D_8019265C ;
        *(s16 *)(workState + 0x36) = D_80192660 ;
        lastConfig = D_80192664 ;
        *(s16 *)(workState + 0x38) = lastConfig;
        return 0;
    }
    if (mode != 1) {
        return 0;
    }
    if (*(u8 *)(workState + 0x3A) == 0) {
        if (*(void **)slot == 0 || func_8003010C(slot, 0x2C) <= 0) {
            *(u8 *)(workState + 0x3A) = 1;
        } else {
            flags = **(unsigned int **)slot;
            if ((flags & 0x1800) != 0) {
                *(u8 *)(workState + 0x3A) = 1;
            } else {
                activity = (flags >> 1) & 7;
                if (activity > 0) {
                    *(u8 *)(workState + 0x3A) = 1;
                }
            }
        }
    }
    if (*(s16 *)(workState + 0x2E) == 0 &&
        *(u8 *)(workState + 0x3B) != 0) {
        completionState = *(unsigned char **)(*(char **)slot + 0x18);
        if (*completionState == 0) {
            *(u8 *)(workState + 0x3A) = 1;
        }
    }
    if (*(u8 *)(workState + 0x3A) != 0) {
        if (*(s16 *)(workState + 0x2E) != 0) {
            return *(s16 *)(workState + 0x30) != 0;
        }
        *(s16 *)(workState + 0x2E) = -1;
        return 1;
    }
    phaseState = *(unsigned char **)(*(char **)slot + 0x18);
    if (*phaseState == 1) {
        *phaseState = 2;
        *(u8 *)(workState + 0x3B) = 1;
    }
    if (*(s16 *)(workState + 0x30) != 0) {
        *(s16 *)(workState + 0x30) += 1;
    }
    if (*(u8 *)(slot + 0xE) != 9) {
        return 0;
    }
    phase = *(u16 *)(slot + 0x16);
    asm volatile("" : : "r"( phase )) ;
    motion = *(int *)(slot + 0x1C);
    if (motion >= 0) {
        asm("" : "=r"(phaseCopy) : "0"(phase));
        if ((s16)phase >= 0x1C) {
            if (*(s16 *)(workState + 0x30) < 0x18) {
                reversedMotion = -motion;
                goto store_reversed_motion;
            }
        }
        if ((s16)phaseCopy < *(u8 *)(slot + 0xF) - 1) {
            return 0;
        }
        phaseState = *(unsigned char **)(*(char **)slot + 0x18);
        if (*phaseState == 2) {
            *phaseState = 4;
        }
        return 1;
    }
    if ((s16)phase >= 0x17) {
        return 0;
    }
    reversedMotion = -motion;
store_reversed_motion:
    *(int *)(slot + 0x1C) = reversedMotion;
    return 0;
}
