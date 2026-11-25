/* Synthetic size-only host-format checks; no guest game content. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_bios_string.h"
static const char *guest_format;
static unsigned stores;
static uint32_t stored;
uint8_t PE_LoadU8(pe_addr_t address) {
    size_t n=strlen(guest_format);
    if(address<1 || (size_t)(address-1)>n) abort();
    return (uint8_t)guest_format[address-1];
}
void PE_StoreU16(pe_addr_t a,uint16_t v) { (void)a;(void)v;abort(); }
void PE_StoreU32(pe_addr_t a,uint32_t v) { if(a!=0x100) abort(); stores++;stored=v; }
int main(void) {
    static const char *formats[]={"%u","%#x","%#.0o","%040u","%.40u","%-12u"};
    static const uint32_t values[]={0,1,42};
    for(unsigned i=0;i<sizeof formats/sizeof formats[0];i++)
        for(unsigned j=0;j<sizeof values/sizeof values[0];j++) {
            /* Retail differs from host C for alternate-form zero; test_bios_sprintf_retail_vectors.c owns those rows. */
            if(values[j]==0 && strchr(formats[i],'#')) continue;
            guest_format=formats[i];
            int actual=PE_Bios_SprintfHost(NULL,0,1,&values[j],1);
            int expected=snprintf(NULL,0,formats[i],(unsigned)values[j]);
            if(actual!=expected) {
                fprintf(stderr,"format %u value %u: got %d expected %d\n",i,j,actual,expected);return 1;
            }
        }
    uint32_t value=1;
    guest_format="%2147483647u";
    if(PE_Bios_SprintfHost(NULL,0,1,&value,1)!=INT32_MAX) return 2;
    guest_format="prefix%2147483647u";
    if(PE_Bios_SprintfHost(NULL,0,1,&value,1)!=-1) return 3;
    guest_format="%2147483648u";
    if(PE_Bios_SprintfHost(NULL,0,1,&value,1)!=-1) return 4;
    guest_format="abc%n"; value=0x100;
    if(PE_Bios_SprintfHost(NULL,0,1,&value,1)!=3 || stores!=1 || stored!=3) return 5;
    puts("16 sizing comparisons (alternate-form zero left to the retail vectors), 3 extreme controls and %n side effect pass");return 0;
}
