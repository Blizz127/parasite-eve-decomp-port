void func_800504BC(void);
void func_800638D8(int slot, void (*callback)(void));

void func_8004B55C(int slot) {
    func_800638D8(slot, func_800504BC);
}
