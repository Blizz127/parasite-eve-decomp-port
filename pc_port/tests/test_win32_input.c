/* Exercise the real Win32 message queue and active-low controller output. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "host_window.h"

static int key_flags(HWND window,UINT message,UINT code,LPARAM flags,uint16_t held)
{
    if(!PostMessageA(window,message,code,flags) || HostWindow_Poll())return 0;
    if(HostWindow_PadRaw()!=(uint16_t)~held){
        fprintf(stderr,"key %u message %u flags %08lX: pad %04X expected %04X\n",code,message,(unsigned long)flags,HostWindow_PadRaw(),(uint16_t)~held);
        return 0;
    }
    return 1;
}
static int key(HWND window,UINT message,UINT code,uint16_t held)
{
    return key_flags(window,message,code,0,held);
}

/* Main Enter and numpad Enter share VK_RETURN and scan code 0x1C. Only
 * lParam's extended-key flag (bit 24) distinguishes them. Repeat and
 * key-up flags must not change the held-key identity. */
static int enter_key(HWND window,int numpad,int down,int repeat,uint16_t held)
{
    uint32_t flags=1u|(0x1Cu<<16)|(numpad?1u<<24:0u);
    if(repeat || !down)flags|=1u<<30;
    if(!down)flags|=1u<<31;
    return key_flags(window,down?WM_KEYDOWN:WM_KEYUP,VK_RETURN,(LPARAM)flags,held);
}
int main(void)
{
    static const struct { UINT key; uint16_t button; } buttons[]={
        {VK_RETURN,0x4000},{'C',0x2000},{'V',0x1000},{'S',0x8000},
        {'Q',0x0400},{'E',0x0800},{'1',0x0100},{'3',0x0200},
        {VK_TAB,0x0001},{'P',0x0008},{VK_UP,0x0010},{VK_RIGHT,0x0020},
        {VK_DOWN,0x0040},{VK_LEFT,0x0080}};
    if(HostWindow_Open(NULL,320,240,"PE Win32 input verification",1,0))return 1;
    HWND window=FindWindowA(NULL,"PE Win32 input verification");
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
    /* Two keys on one button (Return and Z are both Cross by default): releasing
     * one keeps the button held until the other is released (host_key_merge.h). */
    if(!key(window,WM_KEYDOWN,VK_RETURN,0x4000) || !key(window,WM_KEYDOWN,'Z',0x4000) ||
       !key(window,WM_KEYUP,VK_RETURN,0x4000) || !key(window,WM_KEYUP,'Z',0))return 1;
    /* Both physical Enter keys contribute independently, in either release
     * order, including repeat on the key that remains held. */
    for(int first=0;first<2;first++){
        if(!enter_key(window,first,1,0,0x4000) ||
           !enter_key(window,!first,1,0,0x4000) ||
           !enter_key(window,!first,1,1,0x4000) ||
           !enter_key(window,first,0,0,0x4000) ||
           !enter_key(window,!first,0,0,0))return 1;
    }
    /* A consumed Alt+numpad-Enter hotkey must not release held main Enter. */
    if(!enter_key(window,0,1,0,0x4000) ||
       !key_flags(window,WM_SYSKEYDOWN,VK_RETURN,(LPARAM)(1u|(0x1Cu<<16)|(1u<<24)|(1u<<29)),0x4000) ||
       !key_flags(window,WM_SYSKEYUP,VK_RETURN,(LPARAM)(1u|(0x1Cu<<16)|(1u<<24)|(1u<<29)|(1u<<30)|(1u<<31)),0x4000) ||
       !enter_key(window,0,0,0,0))return 1;
    /* Focus loss clears both physical Enter contributions. */
    if(!enter_key(window,0,1,0,0x4000) || !enter_key(window,1,1,0,0x4000))return 1;
    SendMessageA(window,WM_KILLFOCUS,0,0);
    if(HostWindow_PadRaw()!=0xFFFFu ||
       !enter_key(window,0,1,0,0x4000) || !enter_key(window,0,0,0,0) ||
       !enter_key(window,1,0,0,0))return 1;
    if(!key(window,WM_KEYDOWN,'P',8))return 1;
    SendMessageA(window,WM_KILLFOCUS,0,0);
    if(HostWindow_PadRaw()!=0xFFFFu){fputs("focus loss retained a button\n",stderr);return 1;}
    HostWindow_Close();
    puts("PASS: all 14 digital buttons, repeat/release, combinations, shared-button keys, distinct Enter keys, consumed hotkey release and focus reset");
    return 0;
}
