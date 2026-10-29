/* Isolated native animation + actual native rotation backend. No CPU model. */
#include "pe_port_compat.h"
#include "game_port.h"

static uint8_t ram[0x200000], scratch[0x400];
static size_t address(pe_addr_t a, unsigned size, uint8_t **memory)
{
    size_t offset;
    if (a & (size - 1u)) abort();
    if ((a & 0x1FFFFC00u) == 0x1F800000u) {
        offset = a & 0x3FFu;
        if (offset + size > sizeof scratch) abort();
        *memory = scratch; return offset;
    }
    offset = a & 0x1FFFFFFFu;
    if (offset + size > sizeof ram) abort();
    *memory = ram; return offset;
}
uint8_t PE_LoadU8(pe_addr_t a)
{
    uint8_t *memory; size_t off = address(a, 1, &memory); return memory[off];
}
uint16_t PE_LoadU16(pe_addr_t a)
{
    uint8_t *memory; size_t off = address(a, 2, &memory);
    return (uint16_t)(memory[off] | (uint16_t)memory[off + 1u] << 8);
}
uint32_t PE_LoadU32(pe_addr_t a)
{
    uint8_t *memory; size_t off = address(a, 4, &memory);
    return memory[off] | (uint32_t)memory[off + 1u] << 8 |
           (uint32_t)memory[off + 2u] << 16 | (uint32_t)memory[off + 3u] << 24;
}
void PE_StoreU16(pe_addr_t a, uint16_t value)
{
    uint8_t *memory; size_t off = address(a, 2, &memory);
    memory[off] = (uint8_t)value; memory[off + 1u] = (uint8_t)(value >> 8);
}
void PE_StoreU32(pe_addr_t a, uint32_t value)
{
    uint8_t *memory; size_t off = address(a, 4, &memory);
    for (unsigned i = 0; i < 4u; ++i) memory[off + i] = (uint8_t)(value >> (8u * i));
}
unsigned PE_Port_StopEpoch(void) { return 0; }

static int get_word(uint32_t *value)
{
    uint8_t b[4];
    if (fread(b, 1, 4, stdin) != 4) return -1;
    *value = b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    return 0;
}
int main(void)
{
    unsigned total = 0;
    for (;;) {
        char magic[4]; uint32_t entry, dest, clip, frame;
        size_t got = fread(magic, 1, 4, stdin);
        if (!got && feof(stdin) && !ferror(stdin)) return 0;
        if (got != 4 || memcmp(magic, "ANM1", 4) || ++total > 256u) return 2;
        if (get_word(&entry) || get_word(&dest) || get_word(&clip) || get_word(&frame) ||
            fread(ram, 1, sizeof ram, stdin) != sizeof ram ||
            fread(scratch, 1, sizeof scratch, stdin) != sizeof scratch) return 2;
        if (entry == 0x80039B74u) func_80039B74(dest, clip, (int32_t)frame, 0);
        else if (entry == 0x80039D24u) func_80039D24(dest, clip, (short)frame);
        else if (entry == 0x80039ED4u) func_80039ED4(dest, clip, (short)frame);
        else return 2;
        if (fwrite("ARES", 1, 4, stdout) != 4 ||
            fwrite(ram, 1, sizeof ram, stdout) != sizeof ram ||
            fwrite(scratch, 1, sizeof scratch, stdout) != sizeof scratch || fflush(stdout)) return 2;
    }
}
