void func_80192BFC(unsigned char *);

void func_80192BC0(unsigned char *a0)
{
    if (*(unsigned short *)(*(int *)(a0 + 8) + 0x16) >= 0x26) {
        func_80192BFC(a0);
    }
}
