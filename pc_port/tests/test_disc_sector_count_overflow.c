/* Synthetic disc-size metadata; no disc files or original data are read. */
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_disc.h"
static unsigned char image[17u*PE_DISC_RAW_SECTOR];
static long fake_length;
static int fake_stream;
FILE *__wrap_fopen(const char *path,const char *mode) {
    (void)mode;
    if(strcmp(path,"synthetic.bin")==0) return (FILE *)&fake_stream;
    errno=ENOENT;return NULL;
}
int __wrap_fclose(FILE *stream) { if(stream!=(FILE *)&fake_stream) return -1; return 0; }
int __wrap_fseek(FILE *stream,long offset,int whence) { (void)offset;(void)whence; return stream==(FILE *)&fake_stream?0:-1; }
long __wrap_ftell(FILE *stream) { return stream==(FILE *)&fake_stream?fake_length:-1; }
size_t __wrap_fread(void *ptr,size_t size,size_t count,FILE *stream) {
    if(stream!=(FILE *)&fake_stream || size!=1 || count!=PE_DISC_RAW_SECTOR) return 0;
    memcpy(ptr,image,count);return count;
}
int main(void) {
    image[0]=0;memset(image+1,0xff,10);image[11]=0;image[15]=2;
    char err[128];
    PE_Disc *d=PE_Disc_OpenMemory(image,sizeof image);
    if(!d || PE_Disc_UserSectorCount(d)!=17) return 1;
    PE_Disc_Close(d);
    fake_length=(long)sizeof image;d=PE_Disc_Open("synthetic.bin",err,sizeof err);
    if(!d || PE_Disc_UserSectorCount(d)!=17) return 2;
    PE_Disc_Close(d);
#if SIZE_MAX > UINT32_MAX && LONG_MAX > 0x7fffffffL
    /* Only sector zero is accessible: the oversized length itself is fake. */
    uint64_t max_count=(uint64_t)UINT32_MAX*PE_DISC_RAW_SECTOR;
    d=PE_Disc_OpenMemory(image,(size_t)max_count);
    if(!d || PE_Disc_UserSectorCount(d)!=UINT32_MAX) return 5;
    PE_Disc_Close(d);
    fake_length=(long)max_count;d=PE_Disc_Open("synthetic.bin",err,sizeof err);
    if(!d || PE_Disc_UserSectorCount(d)!=UINT32_MAX) return 6;
    PE_Disc_Close(d);
    uint64_t first_overflow=((uint64_t)UINT32_MAX+1u)*PE_DISC_RAW_SECTOR;
    d=PE_Disc_OpenMemory(image,(size_t)first_overflow);
    if(d) { PE_Disc_Close(d);return 7; }
    fake_length=(long)first_overflow;d=PE_Disc_Open("synthetic.bin",err,sizeof err);
    if(d) { PE_Disc_Close(d);return 8; }
    uint64_t huge=((uint64_t)UINT32_MAX+18u)*PE_DISC_RAW_SECTOR;
    if(!getenv("FILE_ONLY")) {
        d=PE_Disc_OpenMemory(image,(size_t)huge);
        if(d) { PE_Disc_Close(d);fputs("oversized memory count accepted\n",stderr);return 3; }
    }
    fake_length=(long)huge;d=PE_Disc_Open("synthetic.bin",err,sizeof err);
    if(d) { PE_Disc_Close(d);fputs("oversized file count accepted\n",stderr);return 4; }
#else
    puts("oversized count controls skipped: host size/long not wide enough");
#endif
    puts("synthetic disc-sector-count controls pass");return 0;
}
