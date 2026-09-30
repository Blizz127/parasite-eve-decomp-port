/* Phase 5S: sixteenth matching C leaf (mid-80CC4.s).
 * VRAM 0x800906B4 / file 0x80EB4 / size 0x30.
 * Stream advance + OR 0x200 into +0x38, OR 0x4400 into +0xF4,
 * store stream byte as halfword at +0x116 (jr delay slot).
 * Scratch probe: GCC 14.2 Phase 4J flags exact 0x30-byte match.
 */
void func_800906B4(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v0 = *(unsigned int *)((unsigned char *)arg0 + 0x38);
    v1 = *(unsigned char **)arg0;
    v0 |= 0x200u;
    *(unsigned int *)((unsigned char *)arg0 + 0x38) = v0;
    *(unsigned char **)arg0 = v1 + 1;
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x4400u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned short *)((unsigned char *)arg0 + 0x116) = byte;
}
