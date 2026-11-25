/* Synthetic format error checks; does not use game data. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_bios_string.h"
static const char *guest_format;
static unsigned stores;
static uint8_t stored;
uint8_t PE_LoadU8(pe_addr_t address)
{
    size_t n=strlen(guest_format);
    if(address<1 || (size_t)(address-1)>n) abort();
    return (uint8_t)guest_format[address-1];
}
void PE_StoreU8(pe_addr_t a,uint8_t v) { if(a!=0x100) abort(); stores++; stored=v; }
void PE_StoreU16(pe_addr_t a,uint16_t v) { (void)a;(void)v;abort(); }
void PE_StoreU32(pe_addr_t a,uint32_t v) { (void)a;(void)v;abort(); }
int main(int argc,char **argv)
{
    static const char *formats[]={"prefix%2147483648u","prefix%.2147483648u","prefix%*u"};
    uint32_t args[]={0x80000000u,1};
    unsigned first=argc>1?(unsigned)strtoul(argv[1],NULL,10):0;
    unsigned end=argc>1?first+1:3;
    if(first>=3) return 2;
    for(unsigned i=first;i<end;i++) {
        char out[32]; memset(out,0x55,sizeof out); guest_format=formats[i];
        if(PE_Bios_SprintfHost(out,sizeof out,1,args,2)!=-1 || out[0]!=0) return 3;
        stores=0;stored=0x55;
        if(PE_Bios_Sprintf(0x100,1,args,2)!=-1 || stores!=1 || stored!=0) return 4;
        if(PE_Bios_Printf(1,args,2)!=-1) return 5;
    }
    if(argc==1) {
        char out[32]; uint32_t normal[]={0xfffffffbu,7}; guest_format="%*u";
        if(PE_Bios_SprintfHost(out,sizeof out,1,normal,2)!=5 || strcmp(out,"7    ")) return 6;
    }
    puts("3 synthetic formatter range-error controls and normal width pass"); return 0;
}
