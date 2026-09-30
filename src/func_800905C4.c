/* Stream byte consumer/state mutator.
 * VRAM 0x800905C4 / file 0x80DC4 / size 0x28.
 */
void func_800905C4(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v1 = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = v1 + 1;
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x8000u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned short *)((unsigned char *)arg0 + 0x112) = byte;
}
