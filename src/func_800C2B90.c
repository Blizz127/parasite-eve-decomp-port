/*
 * func_800C2B90 — weapon-slot record lookup + callback dispatch.
 *
 * VRAM 0x800C2B90 / file 0xB3390 / size 0x17C (95 words), inside B3390.s.
 *
 * Scans the 0x40 six-byte records at D_800F34F4 for the first whose byte +1
 * is zero, recording its index (found = -1 when none). No record sets the
 * "full" flag at D_800E2248+0x40 and returns 0. Otherwise, if the slot kind
 * (func_800C6CE0) is one of 3/4/5, the record is published through
 * func_800C2D0C(found, code & 0xFF, sizes[code & 0xFF]), the callback
 * callbacks[code & 0xFF] is invoked (if not -1) with (slot, rec, data), and
 * the data pointer D_800F3330 + rec->unk4 is returned. Otherwise 0.
 *
 * era -O2 -G0. Levers:
 *  - D_800F34F4 and D_800F3330 are INTEGER bases (they hold pointer values),
 *    so address arithmetic stays `addu`/`addiu`.
 *  - The three func_800C6CE0 calls are genuinely separate short-circuit
 *    calls; storing the kind in a local makes cc1 call once and compare
 *    three times (wrong shape).
 *  - func_800C2D0C's first parameter is `unsigned short`, so the caller emits
 *    `andi $a0,$s1,0xFFFF` (retail) which cc1 fills into the branch delay
 *    slots; an unprototyped call sign-extends instead.
 *  - The four incoming arguments are copied into locals in the order
 *    (slot, sizes, callbacks, code) — cc1 emits the callee-saved save/move
 *    pairs in that assignment order, reproducing retail's
 *    $s2,$s3,$s4,$s0 prologue.
 */
extern unsigned int D_800F34F4;
extern unsigned int D_800F3330;
extern unsigned char *D_800E2248;

extern int func_800C6CE0();
extern void func_800C2D0C(unsigned short idx, unsigned int code, unsigned int value);

int func_800C2B90(int slot, unsigned int code, unsigned short *sizes, int *callbacks) {
    int s_slot = slot;
    unsigned short *s_sizes = sizes;
    int *s_callbacks = callbacks;
    unsigned int s_code = code;
    short found = -1;
    short i;

    for (i = 0; i < 0x40; i++) {
        if (*(signed char *)(D_800F34F4 + i * 6 + 1) == 0) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        *(int *)(D_800E2248 + 0x40) = 1;
        return 0;
    }
    if (func_800C6CE0(s_slot) == 3 || func_800C6CE0(s_slot) == 4 || func_800C6CE0(s_slot) == 5) {
        int c = s_code & 0xFF;
        int rec;
        int data;
        int fn;

        func_800C2D0C(found, c, s_sizes[c]);
        rec = D_800F34F4 + found * 6;
        data = D_800F3330 + *(short *)(rec + 4);
        fn = s_callbacks[c];
        if (fn != -1) {
            ((void (*)(int, int, int))fn)(s_slot, rec, data);
        }
        return data;
    }
    return 0;
}
