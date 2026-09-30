/* Stream byte consumer/state mutator.
 * VRAM 0x8009068C / file 0x80E8C / size 0x28.
 */
void func_8009068C(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v1 = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = v1 + 1;
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x400u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned int *)((unsigned char *)arg0 + 0x108) = byte;
}
