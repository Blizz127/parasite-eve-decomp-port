void func_8003E0D0(void *arg0) {
    unsigned char *data;

    data = *(unsigned char **)arg0;
    *(int *)((unsigned char *)arg0 + 0x24) = 0;
    if (data[2] == 2) {
        *(short *)((unsigned char *)arg0 + 0x28) = 2;
    } else {
        *(short *)((unsigned char *)arg0 + 0x28) = 0;
    }
}
