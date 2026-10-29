"""Directed m0086i inputs for the root's serialized E21 differential runner.

This module creates synthetic fixtures only. It does not execute code, compile,
read retail assets, or mutate the checkout. Import it and pass each (seed, ram,
args) from cases(name, ov, data) to the existing differential harness.

All callees remain the harness's deterministic mocks. These inputs verify the
three bodies' RAM effects, completion flags, ordered calls and initialized
callee-visible stack arguments. They are not whole-room execution evidence.
"""

RAM_SIZE = 0x200000
SCRATCH_SIZE = 0x400
Q, EXTRA = 0x80140000, 0x80141000
CAM, SLOT, ACTOR, BODY = 0x80180000, 0x80180100, 0x80180200, 0x80180300
FLAG, EFFECT, POOL_OWNER = 0x80180400, 0x80180500, 0x80180600
AYA_PTR, AYA = 0x80180700, 0x80180800
CAMERA_MATRIX, POOL, ARCHIVE = 0x80180900, 0x80181000, 0x80182000

# Filled when cases() is consumed; useful in a root mismatch report.
CASE_LABELS = {}

OUT_ARGS = {
    "func_8018F004": "800CFB7C:2,80079754:1",
    "func_8018F734": "80079754:1",
    "func_801900CC": "",
}

PLANNED_BRANCHES = {
    "func_8018F004": [
        "commands 0/1/2/-1/3; states 0/1/2/-1/3",
        "state-0 floor/hit-height comparisons and owner-hit bits match/mismatch",
        "proximity callee returns zero/nonzero; active effect byte zero/nonzero",
        "state-1 8-tick and state-2 32-tick expiry, signed-short wrap",
        "state-2 floor bounce, positive/negative/zero damping",
        "all three draw variants; large/negative-after-wrap scale; extra aliases particle",
    ],
    "func_8018F734": [
        "commands 0/1/2/-1/3; phases 0/1/2/-1/3",
        "init camera GTE state, null actor/body, effect active flag, node flags 0/1/2/255",
        "phase-0 jitter on timer==1; no jitter otherwise; pool nonzero/null",
        "floor crossing threshold, ring and bounded spread counts 0/1/3/20",
        "floor-cross ring negative cos/sin products: signed /4096 rounding at 8018FBA8/8018FBCC",
        "spread signed step and unsigned multiplication wrap; extra aliases record",
        "phase-1 8-tick and phase-2 64-tick expiry, signed-short wrap",
        "phase-0 mesh/floor sprite and phase-1 impact sprite, unsigned size/20",
    ],
    "func_801900CC": [
        "commands 0/1/2/-1/3; states 0/1/-1/2/3",
        "state-0 24-tick and state-1 32-tick expiry, signed-short wrap",
        "state-1 floor bounce and signed velocity damping",
        "state-0 draw variants selected by flag, signed /24,/6,/2 arithmetic",
        "state-1 draw rotation and signed flag; scale*3 wrap then /2",
        "extra aliases particle, zero/positive/negative/boundary scale values",
    ],
}

RETAIL_UNREACHABLE = {
    "func_8018F004": "8018F3F4..8018F408: CLUT index==4 after this body stores 3",
    "func_8018F734": "8018FE30..8018FE44: CLUT index==4 after this body stores 3",
    "func_801900CC": "80190330..80190344 and 801903E4..801903F8: CLUT index==4 after this body stores 3",
}


def h(address, value):
    return address, 2, value


def w(address, value):
    return address, 4, value


def b(address, value):
    return address, 1, value


def store(ram, address, size, value):
    off = address & 0x1FFFFFFF
    assert 0 <= off <= RAM_SIZE - size
    ram[off:off + size] = (value & ((1 << (size * 8)) - 1)).to_bytes(size, "little")


def mock_ret(target, seed):
    value = (target * 2654435761 + seed * 40503) & 0xFFFFFFFF
    if (((seed * 2654435761) & 0xFFFFFFFF) >> 16) & 1:
        return ((value & 0x1FFF) - 0x1000) & 0xFFFFFFFF
    return 0x80100000 + (value & 0xDFFFC)


def mock_out(target, arg, seed):
    value = (target * 2654435761 + seed * 40503 + arg * 0x9E37) & 0xFFFFFFFF
    result = bytearray()
    for _ in range(8):
        value = (value * 1103515245 + 12345) & 0xFFFFFFFF
        result.append((value >> 16) & 0xFF)
    return result


def choose_seed(index, kind):
    """Keep pool writes outside code/input/globals; deliberately test nulls."""
    seed = index * 65537 + 1
    if kind in ("pool_null", "near_zero"):
        target = 0x800CE610 if kind == "pool_null" else 0x800C6B90
        residue = ((4096 - target * 2654435761) * pow(40503, -1, 8192)) & 8191
        seed = (seed & ~8191) + residue
        while mock_ret(target, seed) != 0:
            seed += 8192
        return seed
    if kind == "pool_pointer":
        while True:
            address = mock_ret(0x800CE610, seed)
            if 0x80100000 <= address < 0x80130000 or 0x80160000 <= address < 0x80178000:
                return seed
            seed += 1
    if kind == "ring_negative_products":
        # All mocks select their wide/signed return family from the same seed.
        # Wide pool pointers force random&31 to a multiple of four; multiplying
        # their wide trig returns by that even speed never hits the negative
        # rounding arms. A small positive, aligned physical RAM pointer is also
        # valid guest memory and allows negative signed trig returns instead.
        # Reserve 0x400..0x81f for this fixture, disjoint from code and globals.
        for _ in range(1 << 20):
            pool = mock_ret(0x800CE610, seed)
            if 0x400 <= pool <= 0x800 and pool % 4 == 0:
                speed = (mock_ret(0x80071A54, seed) & 0x1F) + 16
                products = [mock_ret(target, seed) * speed
                            for target in (0x80077DC4, 0x80077CF4)]
                if all(product & 0x80000000 and product & 0xFFF for product in products):
                    return seed
            seed += 1
        raise AssertionError("no safe ring seed with both signed products negative")
    return seed


def baseline(name, ov, data):
    # Nonzero patterned backing makes accidental unwritten data observable.
    ram = bytearray(bytes(range(256)) * ((RAM_SIZE + SCRATCH_SIZE) // 256))
    base = ov["vram"] & 0x1FFFFFFF
    ram[base:base + len(data)] = data
    for start, size in ((Q, 0x80), (EXTRA, 0x40), (CAM, 0x2400), (0x80000000, 0x240)):
        off = start & 0x1FFFFFFF
        ram[off:off + size] = bytes(size)

    values = (100, -300, 150, 1, 31, -5, -33, 0, 0, 0, 0, 0, 0, 0)
    for index, value in enumerate(values):
        store(ram, Q + index * 2, 2, value)
    if name == "func_8018F734":
        for offset, value in ((16, 0x100), (18, -77), (20, 0x200)):
            store(ram, Q + offset, 2, value)
    for offset, value in ((0, 4096), (4, 288), (8, 17), (12, 3)):
        store(ram, EXTRA + offset, 4, value)
    for address, value in (
        (0x800BCFA4, CAM), (0x800F32D0, SLOT), (SLOT + 8, ACTOR),
        (ACTOR, BODY), (ACTOR + 568, CAMERA_MATRIX), (BODY, 0x01000000),
        (BODY + 24, FLAG), (FLAG, 1), (0x800E2368, EFFECT),
        (0x800F33E0, POOL_OWNER), (POOL_OWNER + 8, POOL),
        (0x8009D254, AYA_PTR), (AYA_PTR, AYA), (AYA + 0x4C, 0x123),
        (0x800B0E64, ARCHIVE), (0x800E27EC, 1), (0x800F3428, 1),
        (0x80190B88, ARCHIVE + 0x100), (0x80190B84, ARCHIVE + 0x200),
        (0x80000238, CAMERA_MATRIX),
    ):
        store(ram, address, 4, value)
    store(ram, EFFECT + 13, 1, 1)
    for address, value in ((0x800942EC, 0), (0x800E11E8, 2), (0x800E11EA, 3), (0x800F336C, 4)):
        store(ram, address, 2, value)
    for index in range(16):
        store(ram, 0x800E1204 + index * 2, 2, 256 + index * 4)
        store(ram, 0x800E2850 + index * 2, 2, 0x40 + index)
    for index, value in enumerate((4096, 0, 4096, 0, 4096, -100, 200, 300)):
        store(ram, CAM + index * 4, 4, value)
    return ram


def descriptions(name):
    out = []

    def add(label, mode, patches=(), extra=EXTRA, seed_kind="ordinary"):
        out.append((label, mode, tuple(patches), extra, seed_kind))

    for mode in (-1, -0x80000000, 0, 3, 0x7FFFFFFF):
        if mode != 0 or name != "func_8018F734":
            add("command_%d" % mode, mode)

    if name == "func_8018F734":
        add("init_default", 0)
        for flag in (0, 2, 255):
            add("init_node_flag_%d" % flag, 0, [b(FLAG, flag)])
        add("init_inactive", 0, [b(EFFECT + 13, 0)])
        add("init_actor_null", 0, [w(SLOT + 8, 0)])
        add("init_body_null", 0, [w(ACTOR, 0)])
        add("init_velocity_wrap", 0, [h(Q + 10, -32768)])
        add("init_camera_signed", 0, [w(CAM + index * 4, 0xF1238000 + index * 0x10203) for index in range(8)])
        for phase, ages in ((1, (0, 6, 7, 8, 32767, -1)), (2, (0, 62, 63, 64, 32767, -2))):
            for age in ages:
                add("update_phase%d_age%d" % (phase, age), 1, [h(Q + 24, phase), h(Q + 26, age)])
        for phase in (-1, 3, 32767):
            add("update_phase%d" % phase, 1, [h(Q + 24, phase)])
        for timer in (0, 1, 2):
            for height in (-600, -509, -508, 100):
                add("flight_timer%d_height%d" % (timer, height), 1,
                    [w(0x800E27EC, timer), h(Q + 2, height)], seed_kind="pool_pointer")
        for timer in (0, 1):
            add("flight_null_pool_timer%d" % timer, 1, [w(0x800E27EC, timer)], seed_kind="pool_null")
        for count in (0, 1, 3, 20):
            for step in (7, -16, 0x10001):
                add("spread_count%d_step%d" % (count, step), 1,
                    [w(EXTRA + 12, count), w(EXTRA + 8, step)], seed_kind="pool_pointer")
        for velocity in (-32768, -1000, -33, -1, 0, 31, 32767):
            add("flight_velocity%d" % velocity, 1,
                [h(Q + 8, velocity), h(Q + 12, -velocity), h(Q + 10, velocity)], seed_kind="pool_pointer")
        add("flight_extra_alias", 1, [h(Q + 12, 2), h(Q + 14, 0)], extra=Q, seed_kind="pool_pointer")
        for phase in (0, 1, 2, 3, -1):
            for size in (0, 19, 333, 0xFFFFFFFF):
                add("draw_phase%d_size%d" % (phase, size), 2, [h(Q + 24, phase), w(EXTRA, size)])
        for phase in (0, 1):
            for age in (-32767, -16, 7, 32767):
                add("draw_phase%d_age%d" % (phase, age), 2, [h(Q + 24, phase), h(Q + 26, age)])
        add("draw_extra_alias", 2, extra=Q)
        # Append to preserve every existing fixture's index-derived seed.
        add("floor_ring_negative_cos_sin_products", 1,
            [w(0x800E27EC, 0), h(Q + 24, 0), h(Q + 2, 0), h(Q + 10, 0),
             w(EXTRA + 12, 0)], seed_kind="ring_negative_products")
        return out

    for state in (0, 1, 2, 3, -1):
        for mode in (1, 2):
            add("mode%d_state%d" % (mode, state), mode, [h(Q + 18, state)])
    for state in ((1, 2) if name == "func_8018F004" else (0, 1)):
        for age in (0, 6, 7, 8, 22, 23, 24, 30, 31, 32, 32767, -1):
            add("update_state%d_age%d" % (state, age), 1, [h(Q + 18, state), h(Q + 16, age)])
    bounce_state = 2 if name == "func_8018F004" else 1
    for height in (-300, -5, 0, 100, 32767, -32768):
        for velocity in (-33, 0, 31):
            add("bounce_height%d_velocity%d" % (height, velocity), 1,
                [h(Q + 18, bounce_state), h(Q + 2, height), h(Q + 8, velocity),
                 h(Q + 12, -velocity), h(Q + 10, 5)])
    for size in (0, 7, 0x123, 0x19000000, 0x7FFFFFFF, 0x80000000, 0xFFFFFF00, 0xFFFFFFFF):
        for state in ((0, 1, 2) if name == "func_8018F004" else (0, 1)):
            add("draw_state%d_size%d" % (state, size), 2, [h(Q + 18, state), w(EXTRA, size)])
    for age in (-32768, -16, -1, 0, 5, 23, 32767):
        for flag in (0, 1, -1):
            add("draw_age%d_flag%d" % (age, flag), 2,
                [h(Q + 16, age), h(Q + 6, flag)])
    for mode in (1, 2):
        add("mode%d_extra_alias" % mode, mode, extra=Q)

    if name == "func_8018F004":
        add("proximity_zero", 1, seed_kind="near_zero")
        for height in (-600, -514, -513, -1, 0, 100):
            for active in (0, 1):
                add("proximity_height%d_active%d" % (height, active), 1,
                    [h(Q + 2, height), b(EFFECT + 13, active)])
        for bits in (0x01000000, 0xC1FFFFFF, 0x02000000, 0x11000000):
            add("proximity_owner_bits_%X" % bits, 1, [w(BODY, bits), w(AYA + 0x4C, 0xFFFFFFFF)])
    else:
        for flag in (-32768, -1, 0, 5, 32767):
            add("draw_state1_flag%d" % flag, 2, [h(Q + 18, 1), h(Q + 6, flag), w(0x800E27EC, -3)])
    return out


def cases(name, ov, data):
    if name not in PLANNED_BRANCHES:
        raise ValueError("unexpected m0086i function: " + name)
    specs = descriptions(name)
    assert len(specs) <= 250
    for index, (label, mode, patches, extra, seed_kind) in enumerate(specs):
        seed = choose_seed(index, seed_kind)
        ram = baseline(name, ov, data)
        if seed_kind == "ring_negative_products":
            pool = mock_ret(0x800CE610, seed)
            ram[pool:pool + 0x20] = bytes(0x20)
        for address, size, value in patches:
            store(ram, address, size, value)
        if name == "func_8018F004" and mode == 1:
            qo = Q & 0x1FFFFFFF
            state = int.from_bytes(ram[qo + 18:qo + 20], "little", signed=True)
            if state == 0:
                # The directed height is the post-step height, so the known
                # mocked CFB7C output cannot hide either floor comparison.
                target = int.from_bytes(ram[qo + 2:qo + 4], "little", signed=True)
                offset = int.from_bytes(mock_out(0x800CFB7C, 2, seed)[2:4], "little", signed=True)
                store(ram, Q + 2, 2, target - offset)
        CASE_LABELS[(name, seed)] = label
        yield seed, ram, [mode & 0xFFFFFFFF, Q, extra, 0x55555555]
