/* Synthetic integration of native formatter and guest RAM; no game data. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_bios_string.h"
#include "pe_guest_ram.h"
#define FMT 0x80100000u
#define OUT 0x80101000u
#define COUNT 0x80102000u
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d\n",__LINE__); return 1; } } while(0)
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
static void set_format(const char *fmt) {
    for(size_t i=0;i<=strlen(fmt);i++) PE_StoreU8(FMT+(uint32_t)i,(uint8_t)fmt[i]);
}
int main(void) {
    const char *formats[]={"%#.0o","%#08x","%040u","%.40u"};
    const uint32_t values[]={0,42,1,1};
    PE_RamInit();
    for(unsigned i=0;i<4;i++) {
        char expected[128]; PE_RamReset();set_format(formats[i]);PE_Fill(OUT,128,0xa5);
        int n=snprintf(expected,sizeof expected,formats[i],(unsigned)values[i]);
        CHECK(PE_Bios_Sprintf(OUT,FMT,&values[i],1)==n);
        for(int j=0;j<=n;j++) CHECK(PE_LoadU8(OUT+(uint32_t)j)==(uint8_t)expected[j]);
        CHECK(PE_LoadU8(OUT+(uint32_t)n+1u)==0xa5);
    }
    PE_RamReset();set_format("abc%n");PE_Fill(OUT,8,0xa5);PE_StoreU32(COUNT,0);
    uint32_t count_ptr=COUNT; CHECK(PE_Bios_Sprintf(OUT,FMT,&count_ptr,1)==3);
    CHECK(PE_LoadU32(COUNT)==3 && PE_LoadU8(OUT+3)==0 && PE_LoadU8(OUT+4)==0xa5);
    PE_RamReset();set_format("%.1100u");PE_Fill(OUT,1025,0xa5);uint32_t one=1;
    CHECK(PE_Bios_Sprintf(OUT,FMT,&one,1)==1100);
    for(unsigned i=0;i<1023;i++) CHECK(PE_LoadU8(OUT+i)=='0');
    CHECK(PE_LoadU8(OUT+1023)==0 && PE_LoadU8(OUT+1024)==0xa5);
    PE_RamReset();set_format("prefix%2147483648u");PE_Fill(OUT,8,0xa5);
    CHECK(PE_Bios_Sprintf(OUT,FMT,&one,1)==-1);
    CHECK(PE_LoadU8(OUT)==0 && PE_LoadU8(OUT+1)==0xa5);
    PE_RamDestroy();puts("7 synthetic native guest-RAM formatter controls pass");return 0;
}
