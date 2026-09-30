/* Phase-transition init: clears the three-word state block at D_8009D02C,
 * runs the sub-system reset call chain, then sets the 0x40000000 transition
 * flag and kicks func_800371A4.  VRAM 0x800527C8 / file 0x42FC8 / size 0xC4.
 * era_o2_g8: retail clears D_8009D02C[0..2] with gp-relative stores; the
 * D_8009D030 interior word is D_8009D02C + 4 (no symbol of its own).
 * D_800B0CD8 must stay absolute in this -G8 unit -> incomplete array. */
extern void func_8005B890(int a0);
extern void func_8005BC98(int a0);
extern int D_8009D02C;
extern int D_8009D034;
extern void func_8004F808(void);
extern void func_800528F0(void);
extern void func_8005E588(void);
extern void func_80062568(void);
extern void func_80064964(void);
extern void func_8005DE88(void);
extern void func_80042B38(void);
extern void func_80051084(void);
extern void func_80052C6C(void);
extern void func_8005BCBC(int a0);
extern void func_8005D6F4(void);
extern void func_80051CC4(void);
extern void func_80042C78(void);
extern unsigned int D_800B0CD8[];
extern int func_800371A4(int a0);

int func_800527C8(void) {
    func_8005B890(0);
    func_8005BC98(0);
    D_8009D02C = 0;
    *(int *)((char *)&D_8009D02C + 4) = 0;
    D_8009D034 = 0;
    func_8004F808();
    func_800528F0();
    func_8005E588();
    func_80062568();
    func_80064964();
    func_8005DE88();
    func_80042B38();
    func_80051084();
    func_80052C6C();
    func_8005BCBC(0);
    func_8005D6F4();
    func_80051CC4();
    func_80042C78();
    func_8005BC98(1);
    D_800B0CD8[0] |= 0x40000000;
    return func_800371A4(1);
}
