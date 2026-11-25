/* Synthetic extreme format controls; no real game content. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_bios_string.h"
static const char *guest_format;
uint8_t PE_LoadU8(pe_addr_t address) {
    size_t n=strlen(guest_format);
    if(address==0x100) return 0;
    if(address<1 || (size_t)(address-1)>n) abort();
    return (uint8_t)guest_format[address-1];
}
void PE_StoreU16(pe_addr_t a,uint16_t v) { (void)a;(void)v;abort(); }
void PE_StoreU32(pe_addr_t a,uint32_t v) { (void)a;(void)v;abort(); }
int main(void) {
    static const struct { const char *format; const char *prefix; int32_t result; } cases[]={
        {"%2147483647u","       ",INT32_MAX},
        {"%.2147483647u","0000000",INT32_MAX},
        {"%02147483647u","0000000",INT32_MAX},
        {"%-2147483647u","1      ",INT32_MAX},
        {"x%2147483647u","",-1},
        {"%+.2147483647d","",-1},
        {"%2147483647ux","",-1}
    };
    uint32_t arg=1;
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];i++) {
        char output[8]; memset(output,0x55,sizeof output); guest_format=cases[i].format;
        int32_t n=PE_Bios_SprintfHost(output,sizeof output,1,&arg,1);
        if(n!=cases[i].result || strcmp(output,cases[i].prefix)) {
            fprintf(stderr,"huge format case %u failed: length %d\n",i,n); return 1;
        }
    }
    {
        static const char *formats[]={"%2147483647s","%-2147483647s","x%2147483647s"};
        uint32_t empty_string=0x100;
        for(unsigned i=0;i<3;i++) {
            char output[8]; guest_format=formats[i];
            int32_t n=PE_Bios_SprintfHost(output,sizeof output,1,&empty_string,1);
            if(n!=(i==2?-1:INT32_MAX) || strcmp(output,i==2?"":"       ")) return 2;
        }
    }
    puts("10 synthetic huge-format controls pass"); return 0;
}
