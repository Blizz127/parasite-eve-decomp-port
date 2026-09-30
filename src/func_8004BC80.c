extern unsigned int *func_80062D2C(int a, int b, int c, int d);
extern void func_8004BCB4(void);
void func_8004BC80(void) {
    unsigned char *p = (unsigned char *)func_80062D2C(0x16, 0, 0, 0);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004BCB4;
}
