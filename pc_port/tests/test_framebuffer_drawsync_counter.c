/* Host-only DrawSync diagnostic counter; no GPU, device or game. */
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "../platform/host_framebuffer.c"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d\n",__LINE__); return 1; } } while(0)
int main(int argc,char **argv) {
    HostFB_Init();
    if(argc==2 && strcmp(argv[1],"overflow")==0) {
        fb_drawsync_count=INT_MAX;HostFB_DrawSync(0);return 0;
    }
    int count=-1;
    HostFB_DrawSync(-1);HostFB_DrawSync(0);HostFB_DrawSync(1);
    HostFB_GetState(NULL,&count,NULL,NULL);CHECK(count==3);
    fb_drawsync_count=INT_MAX-1;
    HostFB_DrawSync(0);HostFB_GetState(NULL,&count,NULL,NULL);CHECK(count==INT_MAX);
    HostFB_DrawSync(1);HostFB_GetState(NULL,&count,NULL,NULL);CHECK(count==INT_MAX);
    HostFB_DrawSync(-1);HostFB_GetState(NULL,&count,NULL,NULL);CHECK(count==INT_MAX);
    HostFB_Init();HostFB_GetState(NULL,&count,NULL,NULL);CHECK(count==0);
    puts("synthetic DrawSync diagnostic counter controls pass");return 0;
}
