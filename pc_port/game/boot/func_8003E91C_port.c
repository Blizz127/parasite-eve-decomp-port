/* func_8003E91C (VBlank game callback) and func_80036F7C (timer ramps):
 * ported from the matching decomp -- generated TUs
 * pc_port/game/decomp/func_8003E91C_port.c (src/func_8003E91C.c) and
 * pc_port/game/decomp/func_80036F7C_port.c (src/func_80036F7C.c).  The hand
 * ports (with a port-only StopEpoch guard between the two calls) are retired;
 * stop handling lives in the callback dispatcher (pe_callback.c). */
