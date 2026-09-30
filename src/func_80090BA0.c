/* Stream byte consumer/state mutator.
 * VRAM 0x80090BA0 / file 0x813A0 / size 0x2C.
 */
void func_80090BA0(void *arg0) {
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
    *(unsigned short *)((unsigned char *)arg0 + 0xBC) = value;
}
