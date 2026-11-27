/* Host stub for the state60a1 checker run: pe_plat_sio_poll_tick has no observable
 * output in the vectors (pure port-clock glue), so the checker file does not define it. */
unsigned state60a1_ticks;
void pe_plat_sio_poll_tick(void) { state60a1_ticks++; }
