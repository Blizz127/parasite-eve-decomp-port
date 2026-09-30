void func_80085098(void);
void func_80085F44(void (*)(void));

extern unsigned int D_8009D24C;

void func_800850C0(void) {
    D_8009D24C = 1;
    func_80085F44(func_80085098);
}
