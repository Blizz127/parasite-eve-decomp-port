void func_80042B50(void (*callback)(void));
void func_8004D978(int value);
void func_800504F4(void);
void func_80062F3C(int value);

void func_80050544(int unused, int enabled) {
    if (enabled) {
        func_80062F3C(0x1F);
        func_8004D978(0x45);
        func_80042B50(func_800504F4);
    }
}
