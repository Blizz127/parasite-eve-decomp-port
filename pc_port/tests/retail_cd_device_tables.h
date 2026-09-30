#pragma once
/* Authenticated original Disc1 SDK command tables. */
static struct { uint32_t address,values[32]; } CDDEV_tables[]={
{0x9B07Cu,{0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u}},
{0x9B17Cu,{0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u}},
{0x9B0FCu,{0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u}},
{0x9B1FCu,{0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u}},
{0x9B5A4u,{0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u}},
{0x9B624u,{0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u}},
};
static const uint32_t CDDEV_command_jumps[]={0x80080318u,0x80080354u,0x80080368u,0x80080368u,0x80080354u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080340u,0x80080340u,0x80080368u,0x80080368u,0x80080368u,0x80080368u,0x80080354u};

/* Retail bytes stripped by tools/analysis/strip_retail_seeds.py: these
 * regions are zero here and restored at test time from the user's disc
 * (test_retail_fixups.h / pe_disc_cache.h).  Never commit them. */
static const PE_TestRetailFixup RETAILFIX_cd_device_tables[]={
    {PE_RF_BYTES,CDDEV_tables[0].values,0u,128u,PE_RF_EXE,0x8B87Cu},
    {PE_RF_BYTES,CDDEV_tables[1].values,0u,128u,PE_RF_EXE,0x8B97Cu},
    {PE_RF_BYTES,CDDEV_tables[2].values,0u,128u,PE_RF_EXE,0x8B8FCu},
    {PE_RF_BYTES,CDDEV_tables[3].values,0u,128u,PE_RF_EXE,0x8B9FCu},
    {PE_RF_BYTES,CDDEV_tables[4].values,0u,128u,PE_RF_EXE,0x8BDA4u},
    {PE_RF_BYTES,CDDEV_tables[5].values,0u,128u,PE_RF_EXE,0x8BE24u},
};
