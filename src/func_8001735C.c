/*
 * func_8001735C — field-VM handler: allocate a record, fill three fields from
 * operand words, then run the record's initialiser.
 *
 * VRAM 0x8001735C / file 0x7B5C / size 0x98 (38 words). Field-VM dispatch
 * target 0x80235C in the low byte (D_800910A0 opcode table).
 *
 * Retail (asm/disc1/7B1C.s):
 *   char buf[2] = { **a0, *a0[1] };
 *   p = func_80035038(buf, D_8009D2F0, 1);
 *   p->0x28 = *a0[2];  p->0x2C = *a0[3];  p->0x30 = *a0[4];
 *   func_8001AA78(p);
 *   return 1;
 *
 * D_8009D2F0 is loaded absolutely (needs
 * `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` under `-G8`); func_80035038 is
 * given the raw state pointer as its second argument. era -O2 -G8;
 * LINK_EXACT.
 * ROM: asm/disc1/7B1C.s @ file 0x7B5C, 38 words (0x98 bytes).
 */

extern unsigned int *D_8009D2F0;
extern void *func_80035038(char *a0, void *a1, int a2);
extern void func_8001AA78(void *a0);

int func_8001735C(unsigned int **a0) {
    char buf[2];
    unsigned char *p;

    buf[0] = **a0;
    buf[1] = *a0[1];
    p = func_80035038(buf, D_8009D2F0, 1);
    *(int *)(p + 0x28) = *a0[2];
    *(int *)(p + 0x2C) = *a0[3];
    *(int *)(p + 0x30) = *a0[4];
    func_8001AA78(p);
    return 1;
}
