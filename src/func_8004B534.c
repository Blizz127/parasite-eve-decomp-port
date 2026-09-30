void func_80050438(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004B534(int slot) {
    func_800638D8(slot, func_80050438);
}
