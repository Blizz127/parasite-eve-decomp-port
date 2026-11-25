/* Standalone checker for pe_plat_mdec_io.c against the black-box vector
 * headers: retail_mdec_io_cases.h (final-state, 24 + 16 cases) and
 * retail_mdec_io_trace_cases.h (ordered write traces, 1884 + 2274 cases). */
#include <stdint.h>
#include <stdio.h>
#include "pe_plat/mdec_io.h"
#include "retail_mdec_io_cases.h"
#include "retail_mdec_io_trace_cases.h"

#define SEED_DPCR 0x12345678u /* old header: inferred seed (trace header seeds are recorded) */

static int has_last(const pe_mdec_io_result *r, uint32_t reg, uint32_t *v)
{
    int j, f = 0;
    for (j = 0; j < (int)r->count; j++)
        if (r->writes[j].reg == reg) { *v = r->writes[j].value; f = 1; }
    return f;
}

int main(void)
{
    unsigned i, n_in = sizeof MDECIO_input / sizeof MDECIO_input[0];
    unsigned n_out = sizeof MDECIO_output / sizeof MDECIO_output[0];
    unsigned t_in = sizeof MDECIOT_input / sizeof MDECIOT_input[0];
    unsigned t_out = sizeof MDECIOT_output / sizeof MDECIOT_output[0];
    unsigned bad_in = 0, bad_out = 0, n_refused = 0, bad_ref = 0;
    unsigned tbad_in = 0, tbad_out = 0, tn_wait = 0, tbad_wait = 0, reads_total = 0;

    /* ---- old header: final values ---- */
    for (i = 0; i < n_in; i++) {
        pe_mdec_io_result r = pe_mdec_io_input_trace(MDECIO_input[i].command, MDECIO_input[i].mode,
                                                     MDECIO_input[i].address, SEED_DPCR, 0);
        if (r.would_wait || r.edited_command != MDECIO_input[i].changed ||
            r.upload_source != MDECIO_input[i].address + 4u || r.upload_words != MDECIO_input[i].words) {
            bad_in++;
            printf("old row1 case %u mismatch\n", i);
        }
    }
    for (i = 0; i < n_out; i++) {
        pe_mdec_io_result r = pe_mdec_io_output_trace(MDECIO_output[i].destination,
                                                      MDECIO_output[i].words, SEED_DPCR);
        uint32_t v, blocks = MDECIO_output[i].words >> 5;
        int ok = has_last(&r, PE_MDECIO_REG_DPCR, &v) && v == MDECIO_output[i].dpcr;
        ok = ok && has_last(&r, PE_MDECIO_REG_DMA1_CHCR, &v) && v == MDECIO_output[i].chcr;
        ok = ok && has_last(&r, PE_MDECIO_REG_DMA1_MADR, &v) && v == MDECIO_output[i].madr;
        ok = ok && has_last(&r, PE_MDECIO_REG_DMA1_BCR, &v) && v == MDECIO_output[i].bcr;
        if (!ok) { bad_out++; printf("old row2 case %u mismatch\n", i); }
        {   /* guarded entry point: refusal per spec, otherwise identical to raw */
            pe_mdec_io_result g = pe_mdec_io_output_guarded(MDECIO_output[i].destination,
                                                            MDECIO_output[i].words, SEED_DPCR);
            if (blocks == 0 || blocks > 0xFFFFu) {
                n_refused++;
                if (g.count != 0 || g.exit_code != PE_MDECIO_EXIT_BLOCK_COUNT) bad_ref++;
            } else {
                unsigned j;
                if (g.count != r.count || g.exit_code != 0) bad_ref++;
                for (j = 0; j < g.count && j < r.count; j++)
                    if (g.writes[j].reg != r.writes[j].reg || g.writes[j].value != r.writes[j].value) bad_ref++;
            }
        }
    }

    /* ---- trace header: ordered write lists (compared in order); reads informational ---- */
    for (i = 0; i < t_in; i++) {
        const __typeof__(MDECIOT_input[0]) *c = &MDECIOT_input[i];
        pe_mdec_io_result r = pe_mdec_io_input_trace(c->command, c->mode, 0x80140000u, c->dpcr, c->status);
        unsigned j, bad = 0;
        reads_total += c->nr;
        if (r.would_wait) tn_wait++;
        if (r.would_wait != !c->terminated) { tbad_wait++; bad = 1; }
        else if (c->terminated) {
            if (r.count != c->nw || r.edited_command != c->edited) bad = 1;
            for (j = 0; !bad && j < r.count; j++) {
                const MdecIoAccess *a = &MDECIOT_pool[c->w_off + j];
                if (a->size != 4u || a->reg != r.writes[j].reg || a->value != r.writes[j].value) bad = 1;
            }
        } else if (r.count != 0) bad = 1;
        if (bad) { tbad_in++; if (tbad_in < 6) printf("trace row1 case %u mismatch (cmd %08X mode %08X dpcr %08X status %08X)\n", i, (unsigned)c->command, (unsigned)c->mode, (unsigned)c->dpcr, (unsigned)c->status); }
    }
    for (i = 0; i < t_out; i++) {
        const __typeof__(MDECIOT_output[0]) *c = &MDECIOT_output[i];
        pe_mdec_io_result r = pe_mdec_io_output_trace(c->destination, c->words, c->dpcr);
        unsigned j, bad = 0;
        reads_total += c->nr;
        if (!c->terminated || r.would_wait || r.count != c->nw) bad = 1;
        for (j = 0; !bad && j < r.count; j++) {
            const MdecIoAccess *a = &MDECIOT_pool[c->w_off + j];
            if (a->size != 4u || a->reg != r.writes[j].reg || a->value != r.writes[j].value) bad = 1;
        }
        if (bad) { tbad_out++; if (tbad_out < 6) printf("trace row2 case %u mismatch (dest %08X words %08X dpcr %08X)\n", i, (unsigned)c->destination, (unsigned)c->words, (unsigned)c->dpcr); }
    }

    printf("old header  row 0x8010BFA0: %u/%u match\n", n_in - bad_in, n_in);
    printf("old header  row 0x8010C01C: %u/%u match (guarded: %u refused cases, %u guarded mismatches)\n",
           n_out - bad_out, n_out, n_refused, bad_ref);
    printf("trace header row 0x8010BFA0: %u/%u match (%u would-wait cases, %u wait mismatches)\n",
           t_in - tbad_in, t_in, tn_wait, tbad_wait);
    printf("trace header row 0x8010C01C: %u/%u match\n", t_out - tbad_out, t_out);
    printf("reads: informational only, not produced (%u recorded in total)\n", reads_total);
    return (bad_in || bad_out || bad_ref || tbad_in || tbad_out) ? 1 : 0;
}
