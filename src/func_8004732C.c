void func_80050280(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004732C(int slot) {
    func_800638D8(slot, func_80050280);
}
