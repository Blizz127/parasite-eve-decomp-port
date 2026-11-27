__HEADER__

This is an early, unfinished research build. It contains NO game data:
you must supply your own Parasite Eve (USA) Disc 1 image.

RUN
---
  tar -xzf __TARBALL__
  cd __DIR__
  ./parasite-eve-preview "/path/to/Parasite Eve (USA) (Disc 1).bin"

The disc argument may be the Disc 1 .bin, its .cue, or the folder holding
them. The image must be a raw BIN (2352 bytes/sector), e.g. a Redump
BIN/CUE dump; .iso and .chd are not supported. Extra arguments go to the
game binary, e.g.  ./parasite-eve-preview disc1.bin --scale 4

It opens a window (X11; on Wayland desktops and Steam Game Mode, XWayland
is used automatically). Drag to resize; the 4:3 picture is kept. Esc quits.

__START_NOTE__

CONTROLS                 KEYBOARD                 CONTROLLER (Xbox layout)
--------                 --------                 ------------------------
D-pad / move             Arrow keys               D-pad or left stick
Cross (confirm / act)    Enter, Space, Z or X     A
Circle (cancel)          C                        B
Square                   S                        X
Triangle (menu)          V                        Y
L1 / R1                  Q / E                    LB / RB
L2 / R2                  1 / 3                    LT / RT
Select                   Tab                      Back / View
Start                    P                        Start / Menu
Fast-forward toggle      F6                       L3 (click left stick)
Fast-forward while held  -                        R3 (click right stick)
Quit                     Esc                      -

Keyboard and controller work at the same time. Controllers are read
through SDL2's GameController API (libSDL2-2.0.so.0, loaded at runtime) with
the SDL_GameControllerDB mappings (bin/gamecontrollerdb.txt), so Xbox,
PlayStation, Switch Pro, 8BitDo, Steam Deck / Legion Go / ROG Ally built-in
controls, Steam Input's virtual pad and Handheld Daemon (HHD) pads all use
the layout above. Pads may be plugged in or removed while the game runs.
A device SDL has no mapping for is read as a raw joystick (buttons in Xbox
order, D-pad hat, left stick). Without SDL2 the game falls back to the Linux
joystick interface (/dev/input/js*).

Under Steam Game Mode / gamescope the game starts fullscreen (black bars
keep the 4:3 picture); PE_FULLSCREEN=0 keeps a window, PE_FULLSCREEN=1 or
--fullscreen forces fullscreen anywhere.

Fast-forward runs the game uncapped and mutes the sound (it fades out and
back in, so there is no chipmunk audio or static); the window title shows
[Fast-forward] while it is on.

Dev menu (cheats, Day warps, Options > Enhancements / Assists): hold Back/View
and Start/Menu together on a controller, or press F8 on a keyboard. This build
ships with the dev menu __DEV_MENU_WORD__ (launcher default PE_DEV_MENU=__DEV_MENU__);
PE_DEV_MENU=1 or PE_DEV_MENU=0 in the environment overrides that default.

Controller environment variables:
  PE_PAD_SDL=off               skip SDL2, use /dev/input/js* only
  PE_GAMECONTROLLERDB=file     extra SDL mapping file (also read:
                               ~/.config/parasite-eve-port/gamecontrollerdb.txt)
  PE_PAD_RAW_MAP=c,o,s,t,l1,r1,l2,r2,select,start
                               button indices for an unmapped raw joystick
                               (Cross,Circle,Square,Triangle,...; -1 = none)
  PE_JOYSTICK=/dev/input/js1   js fallback: use only this device
  PE_JOYSTICK=off              js fallback: off
  PE_PAD_SWAP_XY=1             swap Square/Triangle
  PE_PAD_DEBUG=1               print controller devices and the pad word
e.g.  PE_PAD_DEBUG=1 ./parasite-eve-preview disc1.bin
In Steam, a non-Steam game shortcut gets Steam Input's virtual pad; set the
controller layout to "Gamepad" if Steam shows a desktop/mouse layout.

KNOWN LIMITS
------------
__LIMITS__

RUNTIME REQUIREMENTS
--------------------
64-bit Linux with glibc and libX11.so.6 (loaded at runtime; present on
Bazzite / SteamOS / any desktop distro). Controllers use libSDL2-2.0.so.0
when present (Bazzite / SteamOS ship it). Sound uses libpulse-simple.so.0
(PipeWire's pulse compatibility provides it; the game is silent without
it). No other libraries needed. PE_AUDIO_LATENCY_MS=150 raises the audio
buffer (default 100 ms) if sound crackles on a busy system.
