void func_80050308(void);
void func_800638D8(int slot, void (*callback)(void));

void func_800474A8(int slot) {
    func_800638D8(slot, func_80050308);
}
