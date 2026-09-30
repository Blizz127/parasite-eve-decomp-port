/* Phase 5EQ: boot memory-layout wrapper (era path).
 * Both callees are void(void); func_8006A674 remains an asm symbol. */
void func_8006A8D4(void);
void func_8006A674(void);

void func_8006A64C(void) {
    func_8006A8D4();
    func_8006A674();
}
