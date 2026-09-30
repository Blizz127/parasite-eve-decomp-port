void func_80047040(void);
void func_800638D8(int slot, void (*callback)(void));

void func_800471BC(int slot) {
    func_800638D8(slot, func_80047040);
}
