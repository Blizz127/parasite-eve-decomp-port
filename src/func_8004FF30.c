void func_80050C50(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004FF30(int slot) {
    func_800638D8(slot, func_80050C50);
}
