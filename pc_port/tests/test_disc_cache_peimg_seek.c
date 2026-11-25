/* Synthetic cache stream failures; no user files, discs or data. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_disc_cache.c"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d (mode %d)\n",__LINE__,mode);return 1; } } while(0)
static int current_token, image_token, close_current, close_image, mode, fake_read;
static uint8_t fake_exe=42;
static size_t key_pos;
char *__wrap_getenv(const char *name) { (void)name;return NULL; }
FILE *__wrap_fopen(const char *path,const char *flags) {
    (void)flags;
    if(!strcmp(path,"build/disc-cache/current")) { key_pos=0;return (FILE *)&current_token; }
    if(!strcmp(path,"build/disc-cache/fixture/PE.IMG")) return (FILE *)&image_token;
    return NULL;
}
char *__wrap_fgets(char *out,int n,FILE *fp) {
    if(fp!=(FILE *)&current_token) abort();
    snprintf(out,(size_t)n,"fixture\n");return out;
}
int __wrap_fgetc(FILE *fp) {
    static const char key[]="fixture\n";
    if(fp!=(FILE *)&current_token) abort();
    return key_pos<sizeof(key)-1u?(unsigned char)key[key_pos++]:EOF;
}
int __wrap_ferror(FILE *fp) { if(fp!=(FILE *)&current_token) abort();return 0; }
int __wrap_fclose(FILE *fp) {
    if(fp==(FILE *)&current_token) close_current++;
    else if(fp==(FILE *)&image_token) close_image++;
    else abort();
    return 0;
}
int __wrap_fseek(FILE *fp,long offset,int whence) {
    if(fp!=(FILE *)&image_token) abort();
    if(whence==SEEK_END) return mode==0?-1:0;
    if(whence!=SEEK_SET || offset<0) abort();
    return 0;
}
long __wrap_ftell(FILE *fp) {
    if(fp!=(FILE *)&image_token) abort();
    if(mode==1) return -1;
#if LONG_MAX > 0x7fffffffL
    if(mode==2) return (long)((uint64_t)UINT32_MAX+1u);
    if(mode==4) return (long)UINT32_MAX;
#endif
    if(mode==5) return 0;
    return 1234;
}
size_t __wrap_fread(void *ptr,size_t size,size_t n,FILE *fp) {
    if(fp!=(FILE *)&image_token || size!=1) abort();
    memset(ptr,0xa6,n);fake_read++;return n;
}
const PE_KnownDisc g_pe_known_discs[]={{"fixture",1,"fixture","\\SYNTHETIC;1",NULL,NULL}};
const unsigned g_pe_known_disc_count=1;
void PE_DiscCheck_Sha1Hex(const void *data,size_t size,char out[41]) { (void)data;(void)size;(void)out;abort(); }
PE_Disc *PE_Disc_Open(const char *path,char *err,size_t size) { (void)path;(void)err;(void)size;abort(); }
void PE_Disc_Close(PE_Disc *d) { (void)d;abort(); }
int PE_DiscCheck_Identify(const PE_Disc *d,unsigned mask,PE_DiscIdentity *out,char *err,size_t n) { (void)d;(void)mask;(void)out;(void)err;(void)n;abort(); }
int PE_DiscCheck_ReadFile(const PE_Disc *d,const char *path,uint8_t **data,uint32_t *n) { (void)d;(void)path;(void)data;(void)n;abort(); }
bool PE_Disc_FindFile(const PE_Disc *d,const char *path,uint32_t *lba,uint32_t *n,char *name,size_t cap) { (void)d;(void)path;(void)lba;(void)n;(void)name;(void)cap;abort(); }
bool PE_Disc_ReadUserData(const PE_Disc *d,uint32_t lba,uint32_t off,void *out,uint32_t n) { (void)d;(void)lba;(void)off;(void)out;(void)n;abort(); }
static void reset(void) {
    s_disc=NULL;s_peimg_fp=NULL;s_peimg_size=0;s_peimg_tried=0;s_peimg_err[0]=0;
    s_tried=1;s_exe=&fake_exe;s_exe_size=1;close_current=close_image=fake_read=0;
}
int main(int argc,char **argv) {
    int start=argc>1?atoi(argv[1]):0,end=argc>1?start+1:7;
    for(mode=start;mode<end;mode++) {
#if LONG_MAX <= 0x7fffffffL
        if(mode==2 || mode==4) continue;
#endif
        reset();if(mode==6) s_exe=NULL;
        OpenPeImg();CHECK(close_current==1);
        if(mode==0 || mode==1 || mode==2 || mode==6) {
            CHECK(s_peimg_fp==NULL && s_peimg_size==0 && close_image==1);
        } else {
            uint32_t expected=mode==4?UINT32_MAX:mode==5?0u:1234u;
            CHECK(s_peimg_fp==(FILE *)&image_token && s_peimg_size==expected && close_image==0);
            if(expected) {
                uint8_t b=0;CHECK(PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG,expected-1,&b,1,NULL,0)==0 && b==0xa6 && fake_read==1);
            }
            CHECK(__wrap_fclose(s_peimg_fp)==0);s_peimg_fp=NULL;
        }
    }
    puts("synthetic PE.IMG cache seek/size controls pass");return 0;
}
