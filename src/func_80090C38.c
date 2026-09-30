/* Phase 5B: first matching C leaf.
 * VRAM 0x80090C38 / file 0x81438 / size 0x14.
 * Evidence: Phase 4J GCC 14.2 probe — exact original 5 instruction words with
 * -EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestanding
 * -fno-builtin -O1 (also -O2/-O3/-Os).
 * No semantic struct/field names yet.
 */
void func_80090C38(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x10u;
}
