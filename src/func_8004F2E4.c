void func_80050728(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004F2E4(int slot) {
    func_800638D8(slot, func_80050728);
}
