/*
 * func_80017410 — field-VM handler: call the sprite/primitive setter with a
 * stack result word.
 *
 * VRAM 0x80017410 / file 0x7C10 / size 0x34 (13 words). Field-VM dispatch
 * target 0x802410 in the low byte (D_800910A0 opcode table).
 *
 * Retail (asm/disc1/7C10.s):
 *   short buf[8];  buf[0] = -1;
 *   func_800375E0((short)*a0[0], 0, buf);
 *   return 1;
 *
 * `buf` must be a multi-word array: a scalar only reserves one stack slot
 * (frame 0x20) while retail's 0x28 frame stores at 0x10 and passes sp+0x10.
 * era -O2 -G8; LINK_EXACT.
 * ROM: asm/disc1/7C10.s @ file 0x7C10, 13 words (0x34 bytes).
 */

extern void func_800375E0(int a0, int a1, short *a2);

int func_80017410(unsigned short **a0) {
    short buf[8];

    buf[0] = -1;
    func_800375E0((short)*a0[0], 0, buf);
    return 1;
}
