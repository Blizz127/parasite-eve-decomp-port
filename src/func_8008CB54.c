extern void func_8008D7C0(int *a0);
extern void func_80085A64(int a0);
extern void func_8008CF70(int a0);

void func_8008CB54(int a0) {
    int buf;

    func_8008D7C0(&buf);
    if (buf != a0) {
        func_80085A64(0);
        func_8008CF70(a0 | 0x100);
        func_80085A64(1);
    }
}
