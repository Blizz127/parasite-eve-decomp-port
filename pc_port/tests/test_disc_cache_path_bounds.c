/* Synthetic config and disc-operation doubles; never access real data. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_disc_cache.c"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"case failed at line %d\n",__LINE__); return 1; } } while (0)
static const char *test_env;
static const unsigned char *file_data;
static size_t file_len, file_pos;
static int file_present, read_failure, close_failure, file_closes, image_closes;
static int disc_open_error, identify_error;
static int file_token, image_token;
char *__wrap_getenv(const char *name) {
    if (!strcmp(name,"PE_DISC1_BIN")) return (char *)test_env;
    return NULL;
}
FILE *__wrap_fopen(const char *path,const char *mode) {
    (void)mode;
    if (file_present && !strcmp(path,"local/pe_disc1.path")) { file_pos=0;return (FILE *)&file_token; }
    return NULL;
}
int __wrap_fgetc(FILE *f) {
    if (f!=(FILE *)&file_token) abort();
    if (read_failure && file_pos==1) return EOF;
    return file_pos<file_len ? file_data[file_pos++] : EOF;
}
int __wrap_ferror(FILE *f) { if(f!=(FILE *)&file_token) abort();return read_failure && file_pos==1; }
int __wrap_fclose(FILE *f) { if(f!=(FILE *)&file_token) abort();file_closes++;return close_failure?EOF:0; }
static const PE_KnownDisc fake_known={"fixture",1,"fixture","\\SYNTHETIC;1",NULL,NULL};
const PE_KnownDisc g_pe_known_discs[]={{"fixture",1,"fixture","\\SYNTHETIC;1",NULL,NULL}};
const unsigned g_pe_known_disc_count=1;
PE_Disc *PE_Disc_Open(const char *path,char *err,size_t size) {
    if(strcmp(path,"abc")) abort();
    if(disc_open_error) { snprintf(err,size,"open failed");return NULL; }
    return (PE_Disc *)&image_token;
}
void PE_Disc_Close(PE_Disc *d) { if(d!=(PE_Disc *)&image_token) abort();image_closes++; }
int PE_DiscCheck_Identify(const PE_Disc *d,unsigned mask,PE_DiscIdentity *out,char *err,size_t size) {
    (void)mask;if(d!=(PE_Disc *)&image_token) abort();
    if(identify_error) { snprintf(err,size,"identify failed");return -1; }
    out->known=&fake_known;return 0;
}
int PE_DiscCheck_ReadFile(const PE_Disc *d,const char *path,uint8_t **data,uint32_t *size) {
    if(d!=(PE_Disc *)&image_token || strcmp(path,"\\SYNTHETIC;1")) abort();
    *data=NULL;*size=0;return -1;
}
void PE_DiscCheck_Sha1Hex(const void *data,size_t n,char out[41]) { (void)data;(void)n;(void)out;abort(); }
bool PE_Disc_FindFile(const PE_Disc *d,const char *path,uint32_t *lba,uint32_t *size,char *name,size_t n) {
    (void)lba;(void)size;(void)name;(void)n;if(d!=(PE_Disc *)&image_token || strcmp(path,"\\PE.IMG;1")) abort();return false;
}
static void set_file(const unsigned char *s,size_t n) { file_data=s;file_len=n;file_present=1;read_failure=close_failure=0;file_closes=0; }
int main(void) {
    char out[16];uint32_t size=77;
    test_env="abc";file_present=1;file_closes=0;
    CHECK(!PE_DiscCache_ImagePath(out,4) && !strcmp(out,"abc") && file_closes==0);
    memset(out,0x55,sizeof out);CHECK(PE_DiscCache_ImagePath(out,3)==-1 && out[0]==0);
    CHECK(PE_DiscCache_ImagePath(NULL,4)==-1);out[0]='X';CHECK(PE_DiscCache_ImagePath(out,0)==-1 && out[0]=='X');
    test_env=NULL;
    set_file((const unsigned char *)"abc\n",4);CHECK(!PE_DiscCache_ImagePath(out,4) && !strcmp(out,"abc") && file_closes==1);
    set_file((const unsigned char *)"abc",3);CHECK(!PE_DiscCache_ImagePath(out,4) && !strcmp(out,"abc") && file_closes==1);
    set_file((const unsigned char *)"abc\r\n",5);CHECK(!PE_DiscCache_ImagePath(out,4) && !strcmp(out,"abc") && file_closes==1);
    set_file((const unsigned char *)"abc  \n",6);CHECK(!PE_DiscCache_ImagePath(out,4) && !strcmp(out,"abc") && file_closes==1);
    set_file((const unsigned char *)"abc d\n",6);CHECK(PE_DiscCache_ImagePath(out,4)==-1 && !out[0] && file_closes==1);
    set_file((const unsigned char *)"abcd\n",5);CHECK(PE_DiscCache_ImagePath(out,4)==-1 && !out[0] && file_closes==1);
    static const unsigned char embedded[]={'a',0,'b','\n'};
    set_file(embedded,sizeof embedded);CHECK(PE_DiscCache_ImagePath(out,sizeof out)==-1 && !out[0] && file_closes==1);
    set_file((const unsigned char *)"abc\n",4);read_failure=1;CHECK(PE_DiscCache_ImagePath(out,sizeof out)==-1 && !out[0] && file_closes==1);
    set_file((const unsigned char *)"abc\n",4);close_failure=1;CHECK(PE_DiscCache_ImagePath(out,sizeof out)==-1 && !out[0] && file_closes==1);
    set_file((const unsigned char *)"  \n",3);CHECK(PE_DiscCache_ImagePath(out,sizeof out)==-1 && !out[0] && file_closes==1);
    test_env="abc";file_present=0;image_closes=0;s_err[0]=0;
    CHECK(!FromImage(&size) && size==0 && image_closes==1 && !strcmp(s_err,"abc: boot file is unreadable"));
    image_closes=0;s_peimg_err[0]=0;s_disc=NULL;s_peimg_fp=NULL;s_peimg_tried=0;
    OpenPeImg();CHECK(image_closes==1 && s_disc==NULL && !strcmp(s_peimg_err,"abc: PE.IMG;1 not found or invalid"));
    disc_open_error=1;image_closes=0;s_err[0]=0;
    CHECK(!FromImage(&size) && image_closes==0 && !strcmp(s_err,"abc: open failed"));
    disc_open_error=0;identify_error=1;image_closes=0;s_err[0]=0;
    CHECK(!FromImage(&size) && image_closes==1 && !strcmp(s_err,"abc: identify failed"));
    image_closes=0;s_peimg_err[0]=0;s_disc=NULL;s_peimg_fp=NULL;
    OpenPeImg();CHECK(image_closes==1 && !strcmp(s_peimg_err,"abc: identify failed"));
    identify_error=0;disc_open_error=1;image_closes=0;s_peimg_err[0]=0;
    OpenPeImg();CHECK(image_closes==0 && !strcmp(s_peimg_err,"abc: open failed"));
    puts("synthetic configured-path and post-identification diagnostics pass");return 0;
}
