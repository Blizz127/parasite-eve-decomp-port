void func_80051060(void);
void func_800638D8(int slot, void (*callback)(void));

void func_80050204(int slot) {
    func_800638D8(slot, func_80051060);
}
