"""Directed synthetic states for the three M0089I native candidates.

Use ``cases(name, ov, data)`` or replace e21_diff.inputs with ``inputs``.
The caller supplies the retail room blob; no original bytes live in this file.
Yields (seed, bytearray(2 MiB RAM + 1 KiB scratchpad), four argument words).
All fixture allocations are fixed, aligned and below 0x8018E000.  Each case
starts afresh: these are function fixtures, not a gameplay/reachability proof.

The labels returned by descriptions() explain each branch or edge being tested.
The planned sets are each below 300 cases; they are deliberately enumerated
instead of a truncated Cartesian product that may never reach later factors.
No compilation, test invocation, source import, or filesystem write happens here.
"""

from itertools import product
import struct

RAM_SIZE = 0x200000
SCRATCH_SIZE = 0x400
RECORD = 0x80110000
VECTOR = 0x80120000
CAMERA = 0x80130000
ARCHIVE = 0x80140000
MESH = 0x80150000
MASK = 0xFFFFFFFF

TEXTURES = ((0, 0), (4, 0), (4, 1), (4, -1), (7, 1))
OTHER_COMMANDS = (-1, 0, 3, -2147483648, 2147483647)


def _h(ram, address, value):
    struct.pack_into("<H", ram, address & 0x1FFFFFFF, value & 0xFFFF)


def _w(ram, address, value):
    struct.pack_into("<I", ram, address & 0x1FFFFFFF, value & MASK)


def _mock_result(target, seed):
    """Choose mock-return *inputs*, never calculate an expected leaf result.

    This matches e21's seed mapping so dedicated cases can hit a zero trig
    result and both signs of its division biases. It is not a trig oracle.
    """
    value = (target * 2654435761 + seed * 40503) & MASK
    if (((seed * 2654435761) & MASK) >> 16) & 1:
        return (value & 0x1FFF) - 0x1000
    return 0x80100000 + (value & 0xDFFFC)


def _pick_seed(index, kind):
    target = 0x80077CF4 if kind.startswith("sin_") else 0x80077DC4
    if kind.endswith("zero"):
        # Solve the mock's low-13-bit linear congruence, then select its
        # signed-value branch. This visits at most one 32-bit hash period.
        seed = ((4096 - target * 2654435761) * pow(40503, -1, 8192)) % 8192
        for _ in range(1 << 19):
            if _mock_result(target, seed) == 0:
                return seed
            seed += 8192
        raise AssertionError("no zero-return seed in the mock's hash period")
    if kind == "mixed":
        return 1000 + index
    for seed in range(1000 + index * 64, 1000 + (index + 1) * 64):
        value = _mock_result(target, seed)
        if kind.endswith("negative") and -4096 <= value < 0:
            return seed
        if kind.endswith("positive") and 0 < value < 4096:
            return seed
        if kind.endswith("wide") and value >= 0x80000000:
            return seed
    raise AssertionError((index, kind, "no requested mock sign"))


def _case(label, cmd, state=0, counter=0, timer=0, **kw):
    return dict(label=label, cmd=cmd, state=state, counter=counter,
                timer=timer, **kw)


def _trail_specs():
    rows = []
    for cmd, state in product(OTHER_COMMANDS, (0, 1)):
        rows.append(_case(f"other_cmd_{cmd}_state_{state}", cmd, state))
    # Increment occurs before signed thresholds 16/8; 32767 must wrap.
    for state, age, sign in product((0, 1), (-32768, -17, -1, 0, 6, 7, 8, 14, 15, 16, 32767),
                                    ("cos_negative", "cos_positive")):
        rows.append(_case(f"update_s{state}_age{age}_{sign}", 1, state, age, mock=sign))
    for cmd, state in product((1, 2), (-32768, -1, 2, 32767)):
        rows.append(_case(f"invalid_state_{state}_cmd{cmd}", cmd, state))
    # Exactly the five guards p[5]..p[1]: all 32 presence combinations.
    for mask in range(32):
        rows.append(_case(f"draw_segments_mask_{mask:02x}", 2, history_mask=mask))
    # Each component alone must make the OR condition true; signed xyz
    # cancellation must still count as nonzero (not a sum-of-coordinates test).
    for point, vector in product(range(1, 6), ((1, 0, 0), (0, -1, 0), (0, 0, 1), (-1, 1, 0))):
        rows.append(_case(f"draw_single_point_{point}_{vector}", 2,
                          history_mask=1 << (point - 1), single_xyz=vector))
    for age, texture in product((-32768, -9, -1, 0, 7, 8, 15, 16, 32767), TEXTURES):
        rows.append(_case(f"glow_age{age}_texture{texture}", 2, 1, age, texture=texture))
    # v may overlap history or the counters. Seed record first, never overwrite
    # aliased v afterward: observes increment/shift/store order exactly.
    for state, off, age in product((0, 1), (0, 2, 8, 0x28, 0x30), (7, 15)):
        rows.append(_case(f"update_alias_s{state}_off{off}_age{age}", 1, state, age, alias=off))
    for speed, radius, step in ((-32768, 32767, -32768), (32767, -32768, 32767),
                               (-1, -1, -1), (0, 0, 0), (1, 1, 1)):
        for age in (-1, 15):
            rows.append(_case(f"update_numeric_{speed}_{radius}_{step}_age{age}", 1, 0, age,
                              speed=speed, radius=radius, step=step, mock="cos_wide"))
    return rows


def _mesh_specs():
    rows = [_case(f"other_cmd_{cmd}", cmd) for cmd in (-1, 3, -2147483648, 2147483647)]
    for age, state in ((-32768, 32767), (32767, -32768), (15, 1), (0, 0)):
        rows.append(_case(f"init_counter{age}_state{state}", 0, state, age))
    for state, age in product((-32768, -1, 0, 1, 2, 3, 32767), (-32768, -1, 0, 15, 16, 32767)):
        rows.append(_case(f"update_s{state}_age{age}", 1, state, age))
    for state, age, timer in product((0, 1), (-32768, -1, 0, 15, 16, 17, 32767), (-1, 0, 15, 31, 32, 63)):
        rows.append(_case(f"draw_s{state}_age{age}_timer{timer}", 2, state, age, timer))
    # Optional glow uses rsin(counter<<6) in state0. Force its zero path;
    # force both signs of the /32, *3/2 and *2/3 intermediates as well.
    for state, age, mock in product((0, 1), (15, 16),
                                    ("sin_zero", "cos_zero", "cos_negative", "cos_positive")):
        rows.append(_case(f"draw_mock_s{state}_age{age}_{mock}", 2, state, age, mock=mock))
    for state, timer in product((-1, 2, 32767), (-1, 0, 32)):
        rows.append(_case(f"draw_other_state_{state}_timer{timer}", 2, state, timer=timer))
    for state, age, off in product((0, 1), (15, 16), (0, 2, 4, 8)):
        rows.append(_case(f"draw_alias_s{state}_age{age}_off{off}", 2, state, age, alias=off))
    # Initial texture4 cannot take the alternative CLUT arm: this body
    # stores texture3 immediately before reading it. Keep that invariant.
    for row, alt in product((0, 1, 7), (0, 1)):
        rows.append(_case(f"draw_tpage_row{row}_alt{alt}", 2, 1,
                          texture=(4, alt), tpage_row=row))
    return rows


def _particle_specs():
    rows = [_case(f"other_cmd_{cmd}", cmd) for cmd in OTHER_COMMANDS]
    for state, timer, mock in product((0, 1), (-2147483648, -65, -1, 0, 5, 6, 7, 63, 64, 65, 2147483647),
                                      ("cos_negative", "cos_positive")):
        rows.append(_case(f"update_s{state}_timer{timer}_{mock}", 1, state, timer=timer, mock=mock))
    for cmd, state in product((1, 2), (-32768, -1, 2, 32767)):
        rows.append(_case(f"invalid_s{state}_cmd{cmd}", cmd, state))
    for state, timer, texture in product((0, 1), (-65, -1, 0, 1, 5, 6, 15, 16, 63, 64, 65), TEXTURES):
        rows.append(_case(f"draw_s{state}_timer{timer}_texture{texture}", 2, state,
                          timer=timer, texture=texture))
    for cmd, state, off in product((1, 2), (0, 1), (0, 2, 4, 8)):
        rows.append(_case(f"alias_cmd{cmd}_s{state}_off{off}", cmd, state, timer=5, alias=off))
    for cmd, speed, angle in product((1, 2), (-32768, -1, 0, 32767), (-32768, 32767)):
        rows.append(_case(f"numeric_cmd{cmd}_speed{speed}_angle{angle}", cmd, timer=63,
                          speed=speed, angle=angle, mock="cos_wide"))
    return rows


def _specs(name):
    if name == "func_801924F8":
        return _trail_specs()
    if name == "func_80192CD4":
        return _mesh_specs()
    if name == "func_80193200":
        return _particle_specs()
    raise ValueError(f"unsupported M0089I leaf: {name}")


def descriptions(name):
    return [row["label"] for row in _specs(name)]


def _base(ov, data):
    ram = bytearray(RAM_SIZE + SCRATCH_SIZE)
    start = ov["vram"] & 0x1FFFFFFF
    assert start + len(data) <= RAM_SIZE
    ram[start:start + len(data)] = data
    # Test-owned noise gives untouched fields and scratchpad nonzero sentinels.
    noise = bytes((i * 37 + 11) & 255 for i in range(256))
    for address in (RECORD, VECTOR, CAMERA, ARCHIVE, MESH):
        off = address & 0x1FFFFFFF
        ram[off:off + 256] = noise
    ram[RAM_SIZE:] = noise * 4
    ram[0x1FE000:RAM_SIZE] = noise * 32
    _w(ram, 0x800BCFA4, CAMERA)
    _w(ram, 0x800B0E64, ARCHIVE)
    _w(ram, 0x80194168, MESH)
    # Asymmetric camera values expose register/word ordering, with a full
    # signed range rather than an identity matrix that could hide omissions.
    for k, value in enumerate((4096, -211, 313, 417, 2048, -619, 723, -827, 3072, 0)):
        _h(ram, CAMERA + k * 2, value)
    for k, value in enumerate((-123456, 456789, -789012)):
        _w(ram, CAMERA + 20 + k * 4, value)
    for k in range(16):
        _h(ram, 0x800E1204 + k * 2, 100 + 7 * k)
        _h(ram, 0x800E2850 + k * 2, 0x120 + 3 * k)
    _h(ram, 0x800F336A, -3)
    _h(ram, 0x800E11FA, 1)
    for k, value in enumerate((-300, 211, -123)):
        _h(ram, VECTOR + k * 2, value)
    return ram


def cases(name, ov, data):
    rows = _specs(name)
    assert len(rows) <= 300, (name, len(rows))
    base = _base(ov, data)
    for index, spec in enumerate(rows):
        ram = bytearray(base)
        seed = _pick_seed(index, spec.get("mock", "mixed"))
        texture, alt = spec.get("texture", (4, 1))
        _h(ram, 0x800F336C, texture)
        _w(ram, 0x800F3428, alt)
        _w(ram, 0x800E27EC, spec["timer"])
        _h(ram, 0x800E11FA, spec.get("tpage_row", 1))
        if name == "func_801924F8":
            present = spec.get("history_mask", 31)
            for point in range(6):
                xyz = (point * 37 - 111, point * -19 + 77, point * 23 - 47)
                if point and not (present & (1 << (point - 1))):
                    xyz = (0, 0, 0)
                elif point and "single_xyz" in spec:
                    xyz = spec["single_xyz"]
                for axis, value in enumerate(xyz):
                    _h(ram, RECORD + point * 8 + axis * 2, value)
                _h(ram, RECORD + point * 8 + 6, 29 + point * 7)
            _h(ram, RECORD + 6, spec.get("radius", -93))
            _h(ram, RECORD + 14, spec.get("step", -37))
            _h(ram, RECORD + 0x30, spec["counter"])
            _h(ram, RECORD + 0x32, spec["state"])
            _h(ram, RECORD + 0x34, spec.get("angle", -701))
            _h(ram, RECORD + 0x36, spec.get("speed", 137))
        elif name == "func_80192CD4":
            _h(ram, RECORD, spec["counter"])
            _h(ram, RECORD + 2, spec["state"])
            _h(ram, RECORD + 4, -173)
            _h(ram, RECORD + 6, 251)
        else:
            for axis, value in enumerate((-32760, 321, 32760)):
                _h(ram, RECORD + axis * 2, value)
            _h(ram, RECORD + 8, spec["state"])
            _h(ram, RECORD + 10, spec.get("angle", -701))
            _h(ram, RECORD + 12, spec.get("speed", 137))
        vector = RECORD + spec["alias"] if "alias" in spec else VECTOR
        yield seed, ram, [spec["cmd"] & MASK, RECORD, vector, 0]


def inputs(name, ov, data, seeds=None, recorded="", spec=None):
    """Drop-in e21.inputs adapter; the enumerated suite defines its own size.

    Recorded captures belong to e21's original provider and are not silently
    replaced. Save/restore e21.inputs when temporarily selecting this suite.
    """
    if recorded:
        raise ValueError("directed synthetic provider does not consume recorded captures")
    yield from cases(name, ov, data)
