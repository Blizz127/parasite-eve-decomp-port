/*
 * Phase 6A — Stub registry implementation.
 */
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int  g_bootstrap_disc = 0;
int  g_strict_stubs   = 0;
int  g_stub_bootstrap_invocations = 0;

#define MAX_STUBS 128

StubEntry g_stub_registry[MAX_STUBS];
int       g_stub_count = 0;

const char *g_stub_order_log[MAX_ORDER_LOG];
int g_stub_order_count = 0;

void Stub_ResetOrderLog(void) { g_stub_order_count = 0; }

void Stub_Record(const char *symbol, const char *classification)
{
    /* Record in ordered log */
    if (g_stub_order_count < MAX_ORDER_LOG) {
        g_stub_order_log[g_stub_order_count++] = symbol;
    }

    /* Check if already recorded */
    for (int i = 0; i < g_stub_count; i++) {
        if (strcmp(g_stub_registry[i].symbol, symbol) == 0) {
            g_stub_registry[i].invoked++;
            return;
        }
    }

    if (g_stub_count >= MAX_STUBS) {
        fprintf(stderr, "FATAL: stub registry overflow\n");
        exit(1);
    }

    g_stub_registry[g_stub_count].symbol         = symbol;
    g_stub_registry[g_stub_count].classification = classification;
    g_stub_registry[g_stub_count].invoked        = 1;
    g_stub_count++;

    if (strcmp(classification, "BOOTSTRAP_RET") == 0) {
        g_stub_bootstrap_invocations++;
    }

    fprintf(stderr, "[STUB:%s] %s (first invocation)\n", classification, symbol);

    if (g_strict_stubs && strcmp(classification, "UNSUPPORTED") == 0) {
        fprintf(stderr, "FATAL: strict-stubs mode, UNSUPPORTED stub '%s' reached\n", symbol);
        exit(1);
    }
}

void Stub_PrintSummary(void)
{
    int implemented = 0, adapted = 0, bootstrap = 0, unsupported = 0;
    int invoked_bootstrap = 0;

    for (int i = 0; i < g_stub_count; i++) {
        const char *c = g_stub_registry[i].classification;
        if (strcmp(c, "IMPLEMENTED") == 0)   implemented++;
        else if (strcmp(c, "HOST_ADAPTED") == 0) adapted++;
        else if (strcmp(c, "BOOTSTRAP_RET") == 0) {
            bootstrap++;
            if (g_stub_registry[i].invoked > 0) invoked_bootstrap++;
        }
        else if (strcmp(c, "UNSUPPORTED") == 0)   unsupported++;
    }

    printf("\n=== Stub Summary ===\n");
    printf("implemented:       %d\n", implemented);
    printf("host-adapted:      %d\n", adapted);
    printf("bootstrap-return:  %d\n", bootstrap);
    printf("unsupported:       %d\n", unsupported);
    printf("invoked bootstrap stubs: %d\n", invoked_bootstrap);
    printf("uninvoked stubs:   %d\n", bootstrap - invoked_bootstrap);
}
