/* Synthetic executable loader; all disc and guest RAM operations are doubles. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_guest_image.h"
#include "pe_guest_ram.h"

static uint8_t header[2048], ram[32];
static uint32_t file_size;
static int range_calls, body_reads;
static const char cnf[] = "BOOT = cdrom:\\FAKE.EXE;1\n";
bool PE_Disc_FindFile(const PE_Disc *d, const char *p, uint32_t *lba,
                      uint32_t *size, char *name, size_t n)
{
    (void)d; (void)name; (void)n;
    if (strcmp(p, "\\SYSTEM.CNF;1") == 0) { *lba=1; *size=sizeof(cnf)-1; return true; }
    if (strcmp(p, "\\FAKE.EXE;1") == 0) { *lba=2; *size=file_size; return true; }
    return false;
}
bool PE_Disc_ReadUserSector(const PE_Disc *d, uint32_t lba, uint8_t out[2048])
{ (void)d; if (lba != 2) return false; memcpy(out,header,2048); return true; }
bool PE_Disc_ReadUserData(const PE_Disc *d, uint32_t lba, uint32_t off, void *out, uint32_t len)
{
    (void)d;
    if (lba == 1 && off == 0 && len == sizeof(cnf)-1) { memcpy(out,cnf,len); return true; }
    if (lba == 3 && off == 0 && len == 4) { memcpy(out,"TEST",4); body_reads++; return true; }
    return false;
}
bool PE_RangeIsRam(pe_addr_t a, size_t n) { range_calls++; return a==PE_RAM_BASE+8 && n==4; }
void *PE_Translate(pe_addr_t a, size_t n) { if (a!=PE_RAM_BASE+8 || n!=4) abort(); return ram+8; }
static void put32(unsigned offset, uint32_t n)
{ for (unsigned i=0;i<4;i++) header[offset+i]=(uint8_t)(n>>(i*8)); }
int main(int argc, char **argv)
{
    (void)argv;
    char err[128];
    const PE_Disc *disc=(const PE_Disc *)&ram;
    memcpy(header,"PS-X EXE",8);
    put32(0x18,PE_RAM_BASE+8);
    if (argc>1) {
        put32(0x1c,UINT32_MAX-2047u); file_size=0;
        if (PE_GuestImage_LoadExe(disc,err,sizeof(err)) != -1) abort();
        printf("overflow geometry: range_calls=%d\n",range_calls);
        return range_calls ? 1 : 0;
    }
    put32(0x1c,4); file_size=2052;
    if (PE_GuestImage_LoadExe(disc,err,sizeof(err)) || range_calls!=1 || body_reads!=1 || memcmp(ram+8,"TEST",4)) abort();
    puts("synthetic executable body and little-endian header pass");
    return 0;
}
