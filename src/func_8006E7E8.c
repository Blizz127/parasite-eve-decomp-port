extern int func_800811E4(void *);
extern unsigned int D_800B0CD8;

int func_8006E7E8(void)
{
    int ioArgs;
    int pollStatus;

    pollStatus = func_800811E4(&ioArgs);
    if ((unsigned int)(pollStatus + 1) < 2) {
        unsigned int pollMask = 0xFEFFBFFF;
        unsigned int *flagsPtr = &D_800B0CD8;
        *flagsPtr &= pollMask;
    }
    return pollStatus;
}
