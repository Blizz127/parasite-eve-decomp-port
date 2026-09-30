extern unsigned int *D_8009D2F0;

int func_80019A1C(int **a0) {
    ((short *)D_8009D2F0)[0x127] = (short)**a0;
    ((char *)D_8009D2F0)[0x24B] = (char)*a0[1];
    ((char *)D_8009D2F0)[0x24C] = (char)*a0[2];
    ((char *)D_8009D2F0)[0x24D] = (char)*a0[3];
    ((unsigned short *)D_8009D2F0)[0x128] |= 8;
    return 1;
}
