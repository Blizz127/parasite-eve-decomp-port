#ifndef HOST_FILTER_PAUSE_H
#define HOST_FILTER_PAUSE_H
/* Focus policy for the PAUSE ON FOCUS LOSS option (a quality-of-life row,
 * default off).  Backend independent: plain state and pure functions, no
 * X11/Win32/SDL calls, so every presenter feeds it the focus events of its own
 * windowing system and asks the same question.  Header-only (static inline)
 * like host_window_scale.h.
 *
 * The presenter owns the waiting.  The game paces itself on the emulated
 * VBlank (HostWindow's pacer), so holding the pacer holds everything that
 * runs on emulated time: game logic, the sound driver, movies, the CD model.
 * While it waits the presenter keeps polling its window, so focus returning,
 * Escape and a window close still arrive, and it restarts its pacer on the way
 * out so there is no catch-up burst.  Nothing here touches guest state. */
#include <stdint.h>

typedef struct { int focused; } HostFocus;

/* A new window has the focus until its windowing system says otherwise. */
static inline void HostFocus_Reset(HostFocus *f) { f->focused = 1; }

/* The question the presenter asks before each emulated VBlank. */
static inline int HostFocus_ShouldPause(const HostFocus *f, int option_on)
{
    return option_on && !f->focused;
}

/* X11 FocusIn / FocusOut -> 1 (focus gained), 0 (focus lost) or -1 (ignore).
 * mode and detail are the Xlib XFocusChangeEvent fields: mode NotifyNormal 0,
 * NotifyGrab 1, NotifyUngrab 2, NotifyWhileGrabbed 3; detail NotifyAncestor 0,
 * NotifyVirtual 1, NotifyInferior 2, NotifyNonlinear 3, NotifyNonlinearVirtual
 * 4, NotifyPointer 5, NotifyPointerRoot 6, NotifyDetailNone 7.  Ignored, as
 * SDL does: a grab or its release (a hotkey daemon or the window manager
 * flickers focus around its own grabs), and focus moving to or from a child
 * window or following the pointer (the window still has the keyboard, or no
 * other window has taken it). */
static inline int HostFocus_X11Event(int focus_in, int mode, int detail)
{
    if (mode == 1 || mode == 2) return -1;
    if (detail == 2 || detail == 5 || detail == 6 || detail == 7) return -1;
    return focus_in ? 1 : 0;
}

#endif
