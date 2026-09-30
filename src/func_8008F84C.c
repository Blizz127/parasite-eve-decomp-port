/* Stream byte consumer/state mutator.
 * VRAM 0x8008F84C / file 0x8004C / size 0x1C.
 */
void func_8008F84C(void *arg0) {
    unsigned char *stream;
    unsigned char byte;

    stream = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = stream + 1;
    byte = *stream;
    *(unsigned short *)((unsigned char *)arg0 + 0x7C) = byte;
}
