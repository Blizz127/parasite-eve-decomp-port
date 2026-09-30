/*
 * func_800C8C80 — countdown timer step with wrap-to-state-2 retrigger.
 *
 * VRAM 0x800C8C80 / file 0xB9480 / size 0x3C (15 words).
 * ROM: asm/disc1/B9480.s. Twin of func_800C8CBC (0x78) and func_800C8CF8
 * (0xA) — the 0x3C here is the per-frame step added to the +6 counter.
 *
 * The +4 field is stepped down by 8 as an unsigned halfword (retail `lhu`),
 * then re-read as `short` for the `slti 0x14` test; on underflow the field is
 * zeroed and the object's state byte at a1[1] is set to 2. The `short` read
 * must be a second, independent access — folding the compare into the value
 * already in hand changes the register home.
 */
void func_800C8C80(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned short *)(a2 + 4) -= 8;
    *(unsigned short *)(a2 + 6) += 0x3C;
    if (*(short *)(a2 + 4) < 0x14) {
        *(unsigned short *)(a2 + 4) = 0;
        a1[1] = 2;
    }
}
