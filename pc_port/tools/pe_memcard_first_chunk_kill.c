#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
extern int __real_PE_Memcard_Open(const char *,uint32_t);
extern int __real_PE_Memcard_Write(int,const void *,int32_t);
extern int __real_PE_Memcard_Close(int);
int __wrap_PE_Memcard_Open(const char *n,uint32_t m){int r=__real_PE_Memcard_Open(n,m);fprintf(stderr,"[SAVE_TRACE] open mode=%08x fd=%d\n",m,r);return r;}
int __wrap_PE_Memcard_Write(int fd,const void *p,int32_t len){int r=__real_PE_Memcard_Write(fd,p,len);fprintf(stderr,"[SAVE_TRACE] write fd=%d length=%d result=%d\n",fd,len,r);if(r>0&&getenv("PE_TRACE_KILL_AFTER_FIRST")){fflush(stderr);kill(getpid(),SIGKILL);_exit(137);}return r;}
int __wrap_PE_Memcard_Close(int fd){int r=__real_PE_Memcard_Close(fd);fprintf(stderr,"[SAVE_TRACE] close fd=%d result=%d\n",fd,r);return r;}
