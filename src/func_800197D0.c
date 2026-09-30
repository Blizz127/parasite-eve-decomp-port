/* Phase 5EI-first-nonleaf: first non-leaf probe func_800197D0 (era path).
 * VRAM 0x800197D0 / file 0x9FD0 / size 0x20.
 * Frame + jal only: callee func_800375B4 is void(void), zero args — no arg setup.
 * era -O2 -G8 (standing int-return rule; mirrors the 5EH entry).
 * ROM: addiu $sp,-0x18; sw $ra,0x10($sp); jal func_800375B4; nop;
 *      lw $ra,0x10($sp); addiu $v0,1; jr $ra; addiu $sp,+0x18 (jr delay slot). */
void func_800375B4(void);
int func_800197D0(void) {
    func_800375B4();
    return 1;
}
