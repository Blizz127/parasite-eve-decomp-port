/* Synthetic host-format checks; no original game strings or saves. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_bios_string.h"
static const char *guest_format;
uint8_t PE_LoadU8(pe_addr_t address)
{
    size_t length=strlen(guest_format);
    if(address<1 || (size_t)(address-1)>length) abort();
    return (uint8_t)guest_format[address-1];
}
void PE_StoreU16(pe_addr_t a,uint16_t v) { (void)a;(void)v;abort(); }
void PE_StoreU32(pe_addr_t a,uint32_t v) { (void)a;(void)v;abort(); }
int main(void)
{
    static const char *formats[]={"%#.0o","%#.1o","%#.3o","%#5.0o","%-.0o","%o"};
    static const uint32_t values[]={0,1,8,63};
    for(unsigned i=0;i<sizeof formats/sizeof formats[0];i++) {
        for(unsigned j=0;j<sizeof values/sizeof values[0];j++) {
            static const size_t caps[]={1,2,4,64};
            /* Retail differs from host C for alternate-form zero; test_bios_sprintf_retail_vectors.c owns those rows. */
            if(values[j]==0 && strchr(formats[i],'#')) continue;
            for(unsigned k=0;k<sizeof caps/sizeof caps[0];k++) {
                char actual[64],expected[64]; guest_format=formats[i];
                int a=PE_Bios_SprintfHost(actual,(uint32_t)caps[k],1,&values[j],1);
                int e=snprintf(expected,caps[k],formats[i],(unsigned)values[j]);
                if(a!=e || strcmp(actual,expected)) {
                    fprintf(stderr,"format %u value %u cap %zu: actual '%s' (%d), expected '%s' (%d)\n",i,j,caps[k],actual,a,expected,e);
                    return 1;
                }
            }
        }
    }
    puts("80 synthetic octal formatting and truncation comparisons pass (alternate-form zero left to the retail vectors)"); return 0;
}
