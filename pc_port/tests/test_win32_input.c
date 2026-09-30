/* Exercise the real Win32 message queue and active-low controller output. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "host_window.h"

static int key(HWND window,UINT message,UINT code,uint16_t held)
{
    if(!PostMessageA(window,message,code,0) || HostWindow_Poll())return 0;
    if(HostWindow_PadRaw()!=(uint16_t)~held){
        fprintf(stderr,"key %u message %u: pad %04X expected %04X\n",code,message,HostWindow_PadRaw(),(uint16_t)~held);
        return 0;
    }
    return 1;
}
int main(void)
{
    static const struct { UINT key; uint16_t button; } buttons[]={
        {VK_RETURN,0x4000},{'C',0x2000},{'V',0x1000},{'S',0x8000},
        {'Q',0x0400},{'E',0x0800},{'1',0x0100},{'3',0x0200},
        {VK_TAB,0x0001},{'P',0x0008},{VK_UP,0x0010},{VK_RIGHT,0x0020},
        {VK_DOWN,0x0040},{VK_LEFT,0x0080}};
    if(HostWindow_Open(NULL,320,240,"PE Win32 input verification",1,0))return 1;
    HWND window=FindWindowA(NULL,"PE Win32 input verification [Normal speed]");
    if(!window){fputs("test window not found\n",stderr);return 1;}
    HostWindow_Poll();
    for(unsigned i=0;i<sizeof(buttons)/sizeof(buttons[0]);i++){
        if(!key(window,WM_KEYDOWN,buttons[i].key,buttons[i].button) ||
           !key(window,WM_KEYDOWN,buttons[i].key,buttons[i].button) ||
           !key(window,WM_KEYUP,buttons[i].key,0))return 1;
    }
    /* Movement must remain held when the battle/menu button is released. */
    if(!key(window,WM_KEYDOWN,VK_UP,0x10) || !key(window,WM_KEYDOWN,'V',0x1010) ||
       !key(window,WM_KEYUP,'V',0x10) || !key(window,WM_KEYUP,VK_UP,0))return 1;
    if(!key(window,WM_SYSKEYDOWN,'C',0x2000) || !key(window,WM_SYSKEYUP,'C',0))return 1;
    if(!key(window,WM_KEYDOWN,'P',8))return 1;
    SendMessageA(window,WM_KILLFOCUS,0,0);
    if(HostWindow_PadRaw()!=0xFFFFu){fputs("focus loss retained a button\n",stderr);return 1;}
    HostWindow_Close();
    puts("PASS: all 14 digital buttons, repeat/release, combinations and focus reset");
    return 0;
}
