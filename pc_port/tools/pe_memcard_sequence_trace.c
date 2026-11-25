#include <stdint.h>
#include <stdio.h>
extern int __real_PE_Memcard_Open(const char *,uint32_t);
extern int __real_PE_Memcard_Write(int,const void *,int32_t);
extern int __real_PE_Memcard_Close(int);
int __wrap_PE_Memcard_Open(const char *name,uint32_t mode){int result=__real_PE_Memcard_Open(name,mode);fprintf(stderr,"[SAVE_TRACE] open mode=%08x fd=%d\n",mode,result);return result;}
int __wrap_PE_Memcard_Write(int fd,const void *p,int32_t length){int result=__real_PE_Memcard_Write(fd,p,length);fprintf(stderr,"[SAVE_TRACE] write fd=%d length=%d result=%d\n",fd,length,result);return result;}
int __wrap_PE_Memcard_Close(int fd){int result=__real_PE_Memcard_Close(fd);fprintf(stderr,"[SAVE_TRACE] close fd=%d result=%d\n",fd,result);return result;}
