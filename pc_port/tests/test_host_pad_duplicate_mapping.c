/* Synthetic joystick events; no real device, game, or save. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/joystick.h>
#include "host_pad.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"failed line %d\n",__LINE__);return 1; } } while(0)
int main(void)
{
    HostPadDevice d;HostPad_DeviceDefaults(&d);
    /* Mapping preference is captured on the first event, even a hotkey. */
    CHECK(setenv("PE_PAD_SWAP_XY","1",1)==0);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,10,1);
    CHECK(setenv("PE_PAD_SWAP_XY","0",1)==0);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,2,1);
    CHECK(HostPad_DeviceHeld(&d)==HOST_PAD_TRIANGLE);
    HostPad_DeviceDefaults(&d);
    d.btn_code[20]=d.btn_code[0]; /* two physical indices map to Cross */
    d.btn_code[13]=d.btn_code[10]; /* two physical indices map to R3 */
    d.btn_code[14]=d.btn_code[9]; /* two physical indices map to L3 */
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON|JS_EVENT_INIT,0,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,20,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,0,0);
    CHECK(HostPad_DeviceHeld(&d)==HOST_PAD_CROSS);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,20,0);
    CHECK(HostPad_DeviceHeld(&d)==0);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,10,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON|JS_EVENT_INIT,13,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,10,0);
    CHECK(HostPad_DeviceHotkeys(&d)==HOST_PAD_HOTKEY_FF_HOLD);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,13,0);
    CHECK(HostPad_DeviceHotkeys(&d)==0);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,9,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,14,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,9,0);
    CHECK(HostPad_DeviceHotkeys(&d)==HOST_PAD_HOTKEY_FF_TOGGLE);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,14,0);
    CHECK(HostPad_DeviceHotkeys(&d)==0);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,0,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,1,1);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON,1,0);
    CHECK(HostPad_DeviceHeld(&d)==HOST_PAD_CROSS);
    HostPad_DeviceDefaults(&d);
    CHECK(HostPad_DeviceHeld(&d)==0 && HostPad_DeviceHotkeys(&d)==0);
    HostPad_DeviceEvent(&d,JS_EVENT_BUTTON|JS_EVENT_INIT,0,0);
    CHECK(HostPad_DeviceHeld(&d)==0);
    puts("synthetic duplicate joystick mapping controls pass");return 0;
}
