/* Boot memory-region layout initializer.
 * VRAM 0x8006A8D4 / file 0x5B0D4 / size 0x110.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 */
extern unsigned char D_800F34F8;
extern unsigned char D_8010BD00;
extern unsigned char D_80120D08;
extern unsigned char D_801ED800;

extern unsigned char *D_80011614;

extern unsigned char *D_800B0E24;
extern unsigned char *D_800B0E28;
extern unsigned char *D_800B0E2C;
extern unsigned char *D_800B0E30;
extern unsigned char *D_800B0E34;
extern unsigned char *D_800B0E38;
extern unsigned char *D_800B0E3C;
extern unsigned char *D_800B0E40;
extern unsigned char *D_800B0E44;
extern unsigned char *D_800B0E48;
extern unsigned char *D_800B0E4C;
extern unsigned char *D_800B0E50;
extern unsigned char *D_800B0E54;
extern unsigned char *D_800B0E58;
extern unsigned char *D_800B0E5C;
extern unsigned char *D_800B0E60;
extern unsigned char *D_800B0E64;
extern unsigned char *D_800B0E68;
extern unsigned char *D_800B0E6C;

void func_8006A8D4(void) {
    /*
     * Compiler-constrained exact match: with plain locals, era assigns the
     * two cursors to $a0/$a1, the retained constants to $v0/$v1, and sinks
     * D_800B0E28 past D_800B0E2C/D_800B0E30.  One retail-order source retry
     * produced the same schedule.  These bindings preserve retail's
     * $v0/$v1 cursor and $a0/$a1 constant allocation; the statements below
     * still describe the byte-oriented memory-region layout.
     */
    register unsigned char *cursor asm("$2");
    register unsigned char *next asm("$3");
    register unsigned int step_8000 asm("$4");
    register unsigned int step_48000 asm("$5");

    step_48000 = 0x48000;
    cursor = &D_800F34F8;
    next = cursor + 0x1800;
    D_800B0E24 = cursor;
    cursor += 0x6000;
    D_800B0E28 = next;
    next = (unsigned char *)0xE000;
    D_800B0E2C = cursor;
    cursor += (unsigned int)next;
    D_800B0E30 = cursor;

    cursor = &D_8010BD00;
    next = &D_80120D08;
    D_800B0E40 = cursor;
    cursor = next + 0x1C98;
    D_800B0E34 = next;
    next += 0x5C98;
    step_8000 = 0x8000;
    D_800B0E38 = cursor;
    cursor += step_8000;
    D_800B0E3C = next;
    next = cursor + 0x2400;
    D_800B0E44 = cursor;
    cursor += 0x4800;
    D_800B0E4C = cursor;
    cursor += step_48000;
    D_800B0E48 = next;
    next = cursor + 0x4000;
    D_800B0E50 = cursor;
    cursor += step_8000;
    D_800B0E54 = next;
    next = cursor + 0x3800;
    D_800B0E5C = next;
    next = D_80011614;
    D_800B0E58 = cursor;
    cursor += 0x7000;
    D_800B0E60 = cursor;

    cursor = &D_801ED800;
    D_800B0E6C = cursor;
    cursor = next - 8;
    D_800B0E64 = cursor;
    D_800B0E68 = next;
}
