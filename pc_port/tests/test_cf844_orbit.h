/* func_800CF844 (orbit point, native port) on analytic inputs with the
 * seeded trig entries (0 and 90 degrees) and D_800C2270 = identity:
 *   yaw 90, ang 0      : T = Z(90)*(50,0,0) + (0,0,100) = (0,50,100)
 *   yaw 0,  ang X(90)  : T = X(90)*(50,0,0) + X(90)*(0,0,100) = (50,-100,0)
 * out = centre + T; the camera RT (identity, TR z=1024) is current again
 * afterwards. */
static void cf844_setup(pe_addr_t cam, pe_addr_t arena, pe_addr_t ot)
{
    unsigned k;
    d0e88_setup(cam, arena, ot, 0x01000000, 0u);
    for (k = 0; k < 8u; k++) PE_StoreU32(0x800C2270u + k * 4u, 0u);
    PE_StoreU16(0x800C2270u, 0x1000u);
    PE_StoreU16(0x800C2270u + 8u, 0x1000u);
    PE_StoreU16(0x800C2270u + 16u, 0x1000u);
}

static void test_CF844_orbit(void)
{
    const pe_addr_t cam = 0x80140000u, centre = 0x80140040u, out = 0x80140048u,
                    ang = 0x80140050u, arena = 0x80150000u, ot = 0x80160000u;
    TEST("CF844_orbit");
    cf844_setup(cam, arena, ot);
    PE_StoreU16(centre, 10u); PE_StoreU16(centre + 2u, 20u); PE_StoreU16(centre + 4u, 30u);
    PE_StoreU16(ang, 0u); PE_StoreU16(ang + 2u, 0u); PE_StoreU16(ang + 4u, 0u);
    func_800CF844(centre, out, 100, ang, 50, 1024);
    ASSERT((int16_t)PE_LoadU16(out) == 10 && (int16_t)PE_LoadU16(out + 2u) == 70 &&
           (int16_t)PE_LoadU16(out + 4u) == 130, "yaw 90: centre + (0,50,100)");
    ASSERT(PE_GTE_ReadCtrl(0u) == 0x1000u && PE_GTE_ReadCtrl(5u) == 0u &&
           PE_GTE_ReadCtrl(7u) == 1024u, "camera RT reloaded");
    cf844_setup(cam, arena, ot);
    PE_StoreU16(centre, 10u); PE_StoreU16(centre + 2u, 20u); PE_StoreU16(centre + 4u, 30u);
    PE_StoreU16(ang, 1024u); PE_StoreU16(ang + 2u, 0u); PE_StoreU16(ang + 4u, 0u);
    func_800CF844(centre, out, 100, ang, 50, 0);
    ASSERT((int16_t)PE_LoadU16(out) == 60 && (int16_t)PE_LoadU16(out + 2u) == -80 &&
           (int16_t)PE_LoadU16(out + 4u) == 30, "ang X(90): centre + (50,-100,0)");
    PASS();
}
