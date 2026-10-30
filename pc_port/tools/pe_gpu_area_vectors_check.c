/* Check pe_plat_gpu_area.c against pe_gpu_area_vectors.py output.
 * cc -std=c99 -Ipc_port/include -Ipc_port/platform pc_port/tools/pe_gpu_area_vectors_check.c \
 *    pc_port/platform/pe_plat_gpu_area.c -o check && ./check vectors.tsv */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pe_plat/gpu_area.h"
static uint8_t ram[0x200000];
uint16_t PE_LoadU16(uint32_t a){a&=0x1FFFFF;return (uint16_t)(ram[a]|ram[a+1]<<8);}
uint32_t PE_LoadU32(uint32_t a){uint32_t v;memcpy(&v,ram+(a&0x1FFFFF),4);return v;}
void PE_StoreU8(uint32_t a,uint8_t v){ram[a&0x1FFFFF]=v;}
void PE_StoreU32(uint32_t a,uint32_t v){a&=0x1FFFFF;memcpy(ram+a,&v,4);}
int main(int c,char**v){(void)c;
  FILE*f=fopen(v[1],"r");char line[256];unsigned n=0,bad=0;
  while(fgets(line,sizeof line,f)){
    unsigned e,lw,lh,t,w1,w2;char rect[96];int x,y,w,h;
    sscanf(line,"%u\t%u\t%u\t%95[^\t]\t%x %x %x",&e,&lw,&lh,rect,&t,&w1,&w2);
    memset(ram,0,sizeof ram);
    PE_StoreU32(0x80160000u,0xA5A5A5A5u);PE_StoreU32(0x80160004u,0x5A5A5A5Au);PE_StoreU32(0x80160008u,0xC3C3C3C3u);
    ram[0x95750]=lw&255;ram[0x95751]=lw>>8;ram[0x95752]=lh&255;ram[0x95753]=lh>>8;
    uint32_t ra=0;
    if(!strcmp(rect,"NULL")&&e==6){int16_t z[4]={10,20,30,40};memcpy(ram,z,8);}  /* RECT read from address 0 */
    if(strcmp(rect,"NULL")){sscanf(rect,"%d,%d,%d,%d",&x,&y,&w,&h);int16_t r[4]={x,y,w,h};memcpy(ram+0x149C00,r,8);ra=0x80149C00u;}
    if(e==5)pe_plat_gpu_set_tex_window(0x80160000u,ra);else pe_plat_gpu_set_draw_area(0x80160000u,ra);
    uint32_t o[3];memcpy(o,ram+0x160000,12);n++;
    if(o[0]!=t||o[1]!=w1||o[2]!=w2){if(bad++<5)fprintf(stderr,"MISMATCH %s",line);}
  }
  printf("checked %u mismatches %u\n",n,bad);return bad!=0;
}
