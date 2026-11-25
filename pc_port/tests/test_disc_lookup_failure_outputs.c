/* Synthetic ISO lookup; no original disc bytes or files. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_disc.h"
#define SECTORS 24u
static unsigned char image[SECTORS*PE_DISC_RAW_SECTOR];
static uint8_t *user(unsigned lba) { return image+lba*PE_DISC_RAW_SECTOR+PE_DISC_USER_OFFSET; }
static void put32(uint8_t *p,uint32_t v) { for(unsigned i=0;i<4;i++) p[i]=(uint8_t)(v>>(8*i)); }
static unsigned record(uint8_t *p,uint32_t extent,uint32_t bytes,uint8_t flags,const char *name) {
    size_t n=strlen(name);unsigned length=(unsigned)(33+n);if(length<34) length=34;
    memset(p,0,length);p[0]=(uint8_t)length;put32(p+2,extent);put32(p+10,bytes);p[25]=flags;p[32]=(uint8_t)n;memcpy(p+33,name,n);return length;
}
static int unchanged(PE_Disc *d,const char *path) {
    uint32_t extent=0x11223344u,size=0x55667788u;char name[16]="canary";
    return !PE_Disc_FindFile(d,path,&extent,&size,name,sizeof name) && extent==0x11223344u && size==0x55667788u && strcmp(name,"canary")==0;
}
int main(void) {
    memset(image+1,0xff,10);image[15]=2;uint8_t *p=user(16);p[0]=1;memcpy(p+1,"CD001",5);p[6]=1;
    memset(p+40,' ',32);memcpy(p+40,"SYNTHETIC",9);
    record(p+156,17,PE_DISC_USER_SECTOR,2,"R");p[156+33]=0;p=user(17);
    unsigned n=record(p,19,5,0,"GOOD;1");n+=record(p+n,500,5,0,"BAD;1");record(p+n,18,PE_DISC_USER_SECTOR,2,"SUB");
    record(user(18),20,7,0,"OK;1");
    PE_Disc *d=PE_Disc_OpenMemory(image,sizeof image);if(!d) return 1;
    uint32_t extent=0,size=0;char name[16];
    char valid_volume[16];
    if(!PE_Disc_VerifyPVD(d) || !PE_Disc_VolumeId(d,valid_volume,sizeof valid_volume) || strcmp(valid_volume,"SYNTHETIC")) return 10;
    if(!PE_Disc_FindFile(d,"\\GOOD;1",&extent,&size,name,sizeof name) || extent!=19 || size!=5 || strcmp(name,"GOOD;1")) return 2;
    char tiny[3]={0};
    if(!PE_Disc_FindFile(d,"\\SUB\\OK;1",&extent,&size,tiny,sizeof tiny) || extent!=20 || size!=7 || strcmp(tiny,"OK")) return 6;
    if(!PE_Disc_FindFile(d,"\\GOOD;1",&extent,&size,NULL,0) || extent!=19 || size!=5) return 7;
    if(!unchanged(d,"\\BAD;1")) { fputs("bad extent changed output\n",stderr);return 3; }
    if(!unchanged(d,"\\SUB\\MISSING;1")) { fputs("missing child changed output\n",stderr);return 4; }
    if(!unchanged(d,"\\GOOD;1\\CHILD")) { fputs("non-directory changed output\n",stderr);return 5; }
    char volume[16]="unchanged";
    user(16)[6]=0;
    if(PE_Disc_VerifyPVD(d) || PE_Disc_VolumeId(d,volume,sizeof volume) || strcmp(volume,"unchanged") || !unchanged(d,"\\GOOD;1")) {
        fputs("invalid PVD version accepted\n",stderr);return 8;
    }
    user(16)[6]=1;user(16)[156+25]=0;
    if(!PE_Disc_VerifyPVD(d) || !unchanged(d,"\\GOOD;1")) {
        fputs("non-directory root accepted\n",stderr);return 9;
    }
    user(16)[156+25]=2;user(16)[156+32]=0;
    if(!unchanged(d,"\\GOOD;1")) { fputs("invalid root identifier length accepted\n",stderr);return 11; }
    user(16)[156+32]=1;user(16)[156+33]=1;
    if(!unchanged(d,"\\GOOD;1")) { fputs("invalid root identifier accepted\n",stderr);return 12; }
    PE_Disc_Close(d);puts("synthetic disc lookup and PVD controls pass");return 0;
}
