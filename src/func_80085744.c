extern int D_8009D24C;
extern unsigned char D_8009B81C[];
extern int D_8009CDE0;
extern void func_800850F4(unsigned char *a0, int a1);
extern void func_80085174();
extern int func_80085918(unsigned int a0);
extern void func_80085804(unsigned int a0, int a1);
extern int func_800857F4(int a0);
extern int func_800857E4(int a0);
extern void func_80087728(unsigned int a0);
extern void func_80085984();

void func_80085744(void)
{
    if (D_8009D24C == 1) {
        func_800850F4(D_8009B81C, 0x40);
        func_80085174();
    }
    while (func_80085918(0xF2000002) == 0) {
    }
    func_80085804(0xF2000002, 2);
    while (func_800857F4(D_8009CDE0) == 0) {
    }
    while (func_800857E4(D_8009CDE0) == 0) {
    }
    func_80087728(0xFFFFFF);
    func_80085984();
}
