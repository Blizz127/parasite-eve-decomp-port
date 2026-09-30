#pragma once
/* Authenticated retail libpress table command blocks. */
static uint32_t MDECPIX_quant[]={0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u};
static uint32_t MDECPIX_scale[]={0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u};

/* Retail bytes stripped by tools/analysis/strip_retail_seeds.py: these
 * regions are zero here and restored at test time from the user's disc
 * (test_retail_fixups.h / pe_disc_cache.h).  Never commit them. */
static const PE_TestRetailFixup RETAILFIX_mdec_pixel_tables[]={
    {PE_RF_BYTES,MDECPIX_quant,0u,132u,PE_RF_PEIMG,0x1D1514u},
    {PE_RF_BYTES,MDECPIX_scale,0u,132u,PE_RF_PEIMG,0x1D1598u},
};
