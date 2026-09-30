/* Stream byte consumer/state mutator.
 * VRAM 0x80090574 / file 0x80D74 / size 0x28.
 */
void func_80090574(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v1 = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = v1 + 1;
    asm volatile("" : : : "memory");
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x900u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned short *)((unsigned char *)arg0 + 0x10E) = byte;
}
