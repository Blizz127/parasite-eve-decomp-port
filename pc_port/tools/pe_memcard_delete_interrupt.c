#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
extern int __real_PE_Memcard_Delete(const char *);
int __wrap_PE_Memcard_Delete(const char *name){int result=__real_PE_Memcard_Delete(name);fprintf(stderr,"[SAVE_TRACE] delete result=%d\n",result);if(result==1&&getenv("PE_TRACE_KILL_AFTER_DELETE")){fflush(stderr);kill(getpid(),SIGKILL);_exit(137);}return result;}
