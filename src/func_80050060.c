void func_80050E70(void);
void func_800638D8(int slot, void (*callback)(void));

void func_80050060(int slot) {
    func_800638D8(slot, func_80050E70);
}
