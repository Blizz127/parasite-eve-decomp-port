/* The m0005i (Eve's charge effect) code bodies the effect dispatcher binds
 * by fingerprint (pc_port/game/boot/func_800D413C_port.c): copy them from the
 * owner's PE.IMG (room m0005i chunk 2, sectors 0x2734.., VMA 0x8018EFE8)
 * into guest RAM, as retail has them resident.  Code only: none of these
 * bytes lies in a fixture's hashed RAM ranges.  Returns 0 on success. */
#ifndef TEST_M0005I_CODE_H
#define TEST_M0005I_CODE_H
static int load_m0005i_code(void)
{
    static const uint32_t bodies[][2] = {
        {0x8018F018u, 0x318u}, {0x8018F330u, 0x2E4u}, {0x8018F614u, 0x570u},
        {0x8018FB84u, 0x240u}, {0x8018FDC4u, 0xCA8u}, {0x80190A6Cu, 0x2Cu},
    };
    static uint8_t bytes[0xCA8];
    char err[160];
    unsigned i, j;
    for (i = 0; i < sizeof bodies / sizeof bodies[0]; i++) {
        uint32_t off = 0x2734u * 0x800u + (bodies[i][0] - 0x8018EFE8u);
        if (PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG, off, bytes, bodies[i][1], err, sizeof err) != 0)
            return -1;
        for (j = 0; j < bodies[i][1]; j++) PE_StoreU8(bodies[i][0] + j, bytes[j]);
    }
    return 0;
}
#endif
