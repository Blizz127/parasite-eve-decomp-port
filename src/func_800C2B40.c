/* Phase 5G: first tail-region matching C leaf.
 * VRAM 0x800C2B40 / file 0xB3340 / size 0x10.
 * Original: lui/lw D_800E2248; jr; sw $a0, 0x70($v0)
 * Temporary types only — no semantic names for the table/field yet.
 * Same GCC flags as Phase 4J/5B–5F.
 */
extern unsigned char *D_800E2248;

void func_800C2B40(unsigned int arg0) {
    *(unsigned int *)(D_800E2248 + 0x70) = arg0;
}
