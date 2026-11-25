/* No MMIO is expected in this synthetic queue-reader test. */
#include "pe_guest_ram.h"
#include <stdlib.h>

int PE_MMIO_InShadowBlock(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void *PE_MMIO_Translate(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void PE_MMIO_Store(pe_addr_t a, size_t n, uint32_t v)
{ (void)a; (void)n; (void)v; abort(); }
