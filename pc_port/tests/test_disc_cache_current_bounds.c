/* Synthetic cache names and file-operation doubles; no real files or data. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_disc_cache.c"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d (mode %d)\n",__LINE__,mode);return 1; } } while(0)
static char fake_root[770],fake_key[140];
static size_t key_len,key_pos;
static int current_token,content_token,current_open,content_open,current_close,content_close,mode;
static uint8_t fake_exe=42;
char *__wrap_getenv(const char *name) {
    if(!strcmp(name,"PE_DISC_CACHE_DIR")) return fake_root;
    return NULL;
}
FILE *__wrap_fopen(const char *path,const char *flags) {
    (void)flags;size_t n=strlen(path);
    if(n>=8 && !strcmp(path+n-8,"/current")) { current_open++;key_pos=0;return (FILE *)&current_token; }
    if(n>=7 && !strcmp(path+n-7,"/PE.IMG")) { content_open++;return (FILE *)&content_token; }
    if(n>=12 && !strcmp(path+n-12,"/SLUS_006.62")) { content_open++;return (FILE *)&content_token; }
    return NULL;
}
char *__wrap_fgets(char *out,int n,FILE *fp) {
    if(fp!=(FILE *)&current_token) abort();
    int k=0;
    while(k<n-1 && key_pos<key_len) { char c=fake_key[key_pos++];out[k++]=c;if(c=='\n') break; }
    if(!k) return NULL;
    out[k]=0;return out;
}
int __wrap_fgetc(FILE *fp) { if(fp!=(FILE *)&current_token) abort();return key_pos<key_len?(unsigned char)fake_key[key_pos++]:EOF; }
int __wrap_ferror(FILE *fp) { if(fp!=(FILE *)&current_token) abort();return mode==7; }
int __wrap_fclose(FILE *fp) {
    if(fp==(FILE *)&current_token) current_close++;
    else if(fp==(FILE *)&content_token) content_close++;
    else abort();
    return mode==8 && fp==(FILE *)&current_token?EOF:0;
}
int __wrap_fseek(FILE *fp,long offset,int whence) { (void)offset;(void)whence;if(fp!=(FILE *)&content_token) abort();return 0; }
long __wrap_ftell(FILE *fp) { if(fp!=(FILE *)&content_token) abort();return 8; }
void __wrap_rewind(FILE *fp) { if(fp!=(FILE *)&content_token) abort(); }
size_t __wrap_fread(void *ptr,size_t size,size_t n,FILE *fp) { if(fp!=(FILE *)&content_token || size!=1) abort();memset(ptr,0,n);return n; }
const PE_KnownDisc g_pe_known_discs[]={{"fixture",1,"fixture","\\FAKE;1","fixture",NULL}};
const unsigned g_pe_known_disc_count=1;
void PE_DiscCheck_Sha1Hex(const void *data,size_t size,char out[41]) { (void)data;(void)size;strcpy(out,"fixture"); }
PE_Disc *PE_Disc_Open(const char *path,char *err,size_t n) { (void)path;(void)err;(void)n;abort(); }
void PE_Disc_Close(PE_Disc *d) { (void)d;abort(); }
int PE_DiscCheck_Identify(const PE_Disc *d,unsigned m,PE_DiscIdentity *out,char *err,size_t n) { (void)d;(void)m;(void)out;(void)err;(void)n;abort(); }
int PE_DiscCheck_ReadFile(const PE_Disc *d,const char *p,uint8_t **data,uint32_t *n) { (void)d;(void)p;(void)data;(void)n;abort(); }
bool PE_Disc_FindFile(const PE_Disc *d,const char *p,uint32_t *l,uint32_t *n,char *o,size_t z) { (void)d;(void)p;(void)l;(void)n;(void)o;(void)z;abort(); }
static void reset(size_t root_len,size_t key_chars,const char *tail) {
    memset(fake_root,'r',root_len);fake_root[root_len]=0;
    memset(fake_key,'k',key_chars);strcpy(fake_key+key_chars,tail);key_len=key_chars+strlen(tail);key_pos=0;
    current_open=content_open=current_close=content_close=0;
    s_disc=NULL;s_peimg_fp=NULL;s_peimg_size=0;s_peimg_tried=0;s_peimg_err[0]=0;
    s_tried=1;s_exe=&fake_exe;s_exe_size=1;
}
int main(int argc,char **argv) {
    int start=argc>1?atoi(argv[1]):0,end=argc>1?start+1:9;
    for(mode=start;mode<end;mode++) {
        size_t root_len=mode==0?768u:767u;
        size_t key_chars=mode==1?128u:127u;
        const char *tail=mode==3?"\r\n":mode==4?"":mode==5?"  \n":"\n";
        reset(root_len,key_chars,tail);
        if(mode==6) fake_key[3]=0;
        uint32_t size=0;uint8_t *data=FromCache(&size);
        if(mode==0) { CHECK(!data && current_open==0 && content_open==0); }
        else if(mode==1 || mode>=6) { CHECK(!data && current_open==1 && content_open==0 && current_close==1); }
        else { CHECK(data && size==8 && current_open==1 && content_open==1 && current_close==1 && content_close==1);free(data); }
        reset(root_len,key_chars,tail);if(mode==6) fake_key[3]=0;OpenPeImg();
        if(mode==0) { CHECK(!s_peimg_fp && current_open==0 && content_open==0); }
        else if(mode==1 || mode>=6) { CHECK(!s_peimg_fp && current_open==1 && content_open==0 && current_close==1); }
        else { CHECK(s_peimg_fp==(FILE *)&content_token && s_peimg_size==8 && current_open==1 && content_open==1 && current_close==1);__wrap_fclose(s_peimg_fp);s_peimg_fp=NULL; }
    }
    puts("synthetic cache root/current-key controls pass");return 0;
}
