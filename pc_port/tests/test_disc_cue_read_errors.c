/* Synthetic cue stream errors; no real files or disc images. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_disc.c"
static int mode,read_count,stream_token,image_token,image_closes;
FILE *__wrap_fopen(const char *path,const char *flags) {
    (void)flags;
    if(strcmp(path,"fake.bin")==0) return (FILE *)&image_token;
    if(mode==0 || mode==1 || mode==2) { errno=mode==0?ENOENT:mode==1?EACCES:EIO;return NULL; }
    read_count=0;return (FILE *)&stream_token;
}
char *__wrap_fgets(char *buf,int size,FILE *stream) {
    if(stream!=(FILE *)&stream_token) abort();
    int part=read_count++;
    if(mode==13 || mode==14) {
        if(part==0) { memset(buf,'R',(size_t)size-1);buf[size-1]=0;return buf; }
        if(part==1) { snprintf(buf,(size_t)size,"%s",mode==13?"TRACK 01 MODE2/2352\n":"tail\n");return buf; }
        if(part==2 && mode==14) { snprintf(buf,(size_t)size,"TRACK 01 MODE2/2352\n");return buf; }
        return NULL;
    }
    if(mode>=15 && mode<=21) {
        if(mode==17 || mode==21) {
            if(part) return NULL;
            snprintf(buf,(size_t)size,"%s",mode==17?"TRACK 02 MODE2/2352\n":"TRACK 1 MODE2/2352\n");return buf;
        }
        if(part==0) { snprintf(buf,(size_t)size,"%s",mode==16?"TRACK 01 AUDIO\n":"TRACK 01 MODE2/2352\n");return buf; }
        if(part==1) {
            if(mode==20) { memset(buf,' ',(size_t)size-1);memcpy(buf,"TRACK ",6);buf[size-1]=0;return buf; }
            const char *line=mode==15?"TRACK 02 AUDIO\n":mode==19?"REM unrelated\n":"TRACK 02 MODE2/2352\n";
            snprintf(buf,(size_t)size,"%s",line);return buf;
        }
        if(part==2 && mode==20) { snprintf(buf,(size_t)size,"01 AUDIO\n");return buf; }
        return NULL;
    }
    if(mode>=22 && mode<=30) {
        if(mode==27) {
            if(part==0) { snprintf(buf,(size_t)size,"INDEX 01 00:00:00\n");return buf; }
            if(part==1) { snprintf(buf,(size_t)size,"TRACK 01 MODE2/2352\n");return buf; }
            return NULL;
        }
        if(part==0) { snprintf(buf,(size_t)size,"TRACK 01 MODE2/2352\n");return buf; }
        if(part==1) {
            if(mode==29 || mode==30) { memset(buf,' ',(size_t)size-1);if(mode==30) memcpy(buf+size-4,"IND",3);buf[size-1]=0;return buf; }
            const char *line=mode==23?"INDEX 01 00:02:00\n":mode==24?"INDEX 01 invalid\n":mode==25?"INDEX 02 00:02:00\n":mode==28?"INDEX 00000000000000001 00:02:00\n":"INDEX 01 00:00:00\n";
            snprintf(buf,(size_t)size,"%s",line);return buf;
        }
        if(part==2 && (mode==26 || mode==29 || mode==30)) { snprintf(buf,(size_t)size,"%s",mode==26?"INDEX 01 00:00:00\n":mode==30?"EX 01 00:02:00\n":"INDEX 01 00:02:00\n");return buf; }
        return NULL;
    }
    if(part) return NULL;
    static const char *const lines[]={
        "TRACK 01 MODE2/2352", "TRACK 01 AUDIO", "TRACK 01 MODE2/2352",
        "TRACK 01 MODE2/2352", "REM TRACK 01 MODE2/2352",
        "TRACK 01 MODE2/2352X", "  TRACK 01 MODE2/2352\r\n",
        "FILE MODE2/2352 BINARY", "TRACK MODE2/2352", "track 01 mode2/2352"
    };
    unsigned index=mode>=3?(unsigned)mode-3:0;
    if(index>=sizeof lines/sizeof lines[0]) abort();
    snprintf(buf,(size_t)size,"%s",lines[index]);return buf;
}
int __wrap_ferror(FILE *stream) { if(stream!=(FILE *)&stream_token) abort();return mode==5; }
int __wrap_fclose(FILE *stream) {
    if(stream==(FILE *)&image_token) { image_closes++;return 0; }
    if(stream!=(FILE *)&stream_token) abort();
    return mode==6?EOF:0;
}
int __wrap_fseek(FILE *stream,long offset,int whence) {
    (void)offset;(void)whence;if(stream!=(FILE *)&image_token) abort();return 0;
}
long __wrap_ftell(FILE *stream) {
    if(stream!=(FILE *)&image_token) abort();
    return 17L*PE_DISC_RAW_SECTOR;
}
size_t __wrap_fread(void *ptr,size_t size,size_t count,FILE *stream) {
    if(stream!=(FILE *)&image_token || size!=1 || count!=PE_DISC_RAW_SECTOR) abort();
    memset(ptr,0,count);memset((unsigned char *)ptr+1,0xff,10);
    ((unsigned char *)ptr)[15]=2;return count;
}
int main(void) {
    static const int expected[]={0,-1,-1,1,-1,-1,-1,-1,-1,1,-1,-1,1,-1,1,-1,-1,-1,-1,1,-1,1,1,-1,-1,1,-1,-1,-1,-1,-1};
    for(mode=0;mode<31;mode++) {
        int actual=PE_Disc_CheckCue("fake.bin");
        if(actual!=expected[mode]) { fprintf(stderr,"case %d returned %d, expected %d\n",mode,actual,expected[mode]);return 1; }
        char err[128];image_closes=0;
        PE_Disc *disc=PE_Disc_Open("fake.bin",err,sizeof err);
        if((disc!=NULL)!=(expected[mode]>=0)) return 2;
        if(disc) PE_Disc_Close(disc);
        if(image_closes!=1) return 3;
    }
    puts("31 synthetic cue and disc-open controls pass");return 0;
}
