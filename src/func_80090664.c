/* Stream byte consumer/state mutator.
 * VRAM 0x80090664 / file 0x80E64 / size 0x28.
 */
void func_80090664(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v1 = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = v1 + 1;
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x200u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned int *)((unsigned char *)arg0 + 0x104) = byte;
}
