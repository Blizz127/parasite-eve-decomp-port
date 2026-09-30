/* Stream byte consumer/state mutator.
 * VRAM 0x80090B30 / file 0x81330 / size 0x2C.
 */
void func_80090B30(void *arg0) {
    unsigned char *stream;
    unsigned int value;

    stream = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = stream + 1;
    value = *stream;
    if (value != 0) {
        value++;
    } else {
        value = 0x101;
    }
    *(unsigned short *)((unsigned char *)arg0 + 0xBA) = value;
}
