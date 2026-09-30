void func_80192D8C(void);

void func_80192D50(unsigned char *a0)
{
    if (*(unsigned short *)(*(int *)(a0 + 8) + 0x16) >= 0x3B) {
        func_80192D8C();
    }
}
