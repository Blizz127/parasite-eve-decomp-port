void func_80050DC0(void);
void func_800638D8(int slot, void (*callback)(void));

void func_80050038(int slot) {
    func_800638D8(slot, func_80050DC0);
}
