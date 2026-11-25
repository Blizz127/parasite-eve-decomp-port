/* Real host I/O on invented temporary BIN/CUE files only. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pe_disc.h"
#define SECTORS 17u
static unsigned char image[SECTORS*PE_DISC_RAW_SECTOR];
static int cue_write(const char *path,const char *text) {
    FILE *f=fopen(path,"wb");if(!f) return 0;
    size_t n=strlen(text);int ok=fwrite(text,1,n,f)==n;
    return fclose(f)==0 && ok;
}
int main(void) {
    char dir[]="build/codex-disc-realio-20261108/case-XXXXXX";
    char bin[256],cue[256],err[256];int result=1,have_bin=0,have_cue=0;
    PE_Disc *d=NULL;FILE *f=NULL;
    if(!mkdtemp(dir)) return 1;
    if(snprintf(bin,sizeof bin,"%s/fake.bin",dir)>=(int)sizeof bin ||
       snprintf(cue,sizeof cue,"%s/fake.cue",dir)>=(int)sizeof cue) goto done;
    memset(image+1,0xff,10);image[15]=2;
    f=fopen(bin,"wb");if(!f) goto done;have_bin=1;
    if(fwrite(image,1,sizeof image,f)!=sizeof image) goto done;
    if(fclose(f)!=0) { f=NULL;goto done; } f=NULL;
    d=PE_Disc_Open(bin,err,sizeof err);
    if(!d || PE_Disc_UserSectorCount(d)!=SECTORS) goto done;
    PE_Disc_Close(d);d=NULL;
    have_cue=1;
    if(!cue_write(cue,"FILE \"fake.bin\" BINARY\n  TRACK 01 MODE2/2352\n")) goto done;
    d=PE_Disc_Open(bin,err,sizeof err);if(!d) goto done;
    PE_Disc_Close(d);d=NULL;
    if(!cue_write(cue,"FILE \"fake.bin\" BINARY\n  TRACK 01 MODE2/2352\n  TRACK 02 AUDIO\n")) goto done;
    d=PE_Disc_Open(bin,err,sizeof err);if(d) goto done;
    if(!cue_write(cue,"REM TRACK 01 MODE2/2352\n")) goto done;
    d=PE_Disc_Open(bin,err,sizeof err);if(d) goto done;
    if(!cue_write(cue,"FILE \"fake.bin\" BINARY\n TRACK 01 MODE2/2352\n INDEX 01 00:00:00\n")) goto done;
    d=PE_Disc_Open(bin,err,sizeof err);if(!d) goto done;
    PE_Disc_Close(d);d=NULL;
    if(!cue_write(cue,"FILE \"fake.bin\" BINARY\n TRACK 01 MODE2/2352\n INDEX 01 00:02:00\n")) goto done;
    d=PE_Disc_Open(bin,err,sizeof err);if(d) goto done;
    result=0;
done:
    if(d) PE_Disc_Close(d);
    if(f) fclose(f);
    if(have_cue && unlink(cue)!=0) result=1;
    if(have_bin && unlink(bin)!=0) result=1;
    if(rmdir(dir)!=0) result=1;
    if(result) { fputs("synthetic disc file-I/O control failed\n",stderr);return result; }
    puts("6 synthetic disc host file-I/O controls pass");return 0;
}
