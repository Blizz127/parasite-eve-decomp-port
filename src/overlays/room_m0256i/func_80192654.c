void func_80192680(void);

void func_80192654(unsigned char *a0)
{
    if (*(unsigned char *)(*(int *)(a0 + 8) + 0xE) == 0xB) {
        *(void **)(a0 + 0xC) = func_80192680;
    }
}
