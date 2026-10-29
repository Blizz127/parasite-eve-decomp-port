"""Directed caller-level fixtures for M0089I func_80193618.

cases(name, ov, data) yields (seed, fresh RAM plus scratchpad, four arguments).
descriptions(name) returns labels in exactly that order. The caller supplies
the retail room bytes; this file contains only synthetic fixture state.

These cases target the e21 differential harness, whose callees are mocked.
They check all caller command paths, pointer guards, signed arithmetic,
thresholds and ordered call arguments. They do not execute pool allocation,
proximity, mesh rendering, color interpolation or the fan callee. In
particular a fixed mock return can represent all 16 successful allocations
or all 16 null allocations, but cannot represent partial pool exhaustion.
The preserved pe_m0089i_f9hand_oracle.py supplies the full-callee cases.

No imports from the game, execution, compilation, or filesystem writes occur.
"""

from itertools import product
import struct

RAM_SIZE = 0x200000
SCRATCH_SIZE = 0x400
MASK = 0xFFFFFFFF
NAME = "func_80193618"
RECORD, VECTOR = 0x80110000, 0x80110100
SLOT, NODE, FLAGS, RENDER = 0x80111000, 0x80111100, 0x80111200, 0x80111300
ERECORD, ACTOR, ACTOR_FLAGS = 0x80111400, 0x80111500, 0x80111600
POOL_OWNER, POOL = 0x80112000, 0x80112100
ARCHIVE, MESH1, MESH2, CAMERA = 0x80113000, 0x80114000, 0x80115000, 0x80116000


def _b(ram, address, value):
    ram[address & 0x1FFFFFFF] = value & 0xFF


def _h(ram, address, value):
    struct.pack_into("<H", ram, address & 0x1FFFFFFF, value & 0xFFFF)


def _w(ram, address, value):
    struct.pack_into("<I", ram, address & 0x1FFFFFFF, value & MASK)


def _mock_result(target, seed):
    """Select safe mock input seeds; never compute the leaf's expected result."""
    value = (target * 2654435761 + seed * 40503) & MASK
    if (((seed * 2654435761) & MASK) >> 16) & 1:
        return (value & 0x1FFF) - 0x1000
    return 0x80100000 + (value & 0xDFFFC)


def _pick_seed(index, kind):
    targets = {"pool": 0x800CE610, "near": 0x800C6B90,
               "cos": 0x80077DC4, "sin": 0x80077CF4}
    family, mode = kind.split("_", 1)
    target = targets[family]
    if mode == "zero":
        seed = ((4096 - target * 2654435761) * pow(40503, -1, 8192)) % 8192
        for _ in range(1 << 19):
            if _mock_result(target, seed) == 0:
                return seed
            seed += 8192
        raise AssertionError((kind, "no exact zero mock seed"))
    start = 100000 + index * 4096
    for seed in range(start, start + 16384):
        result = _mock_result(target, seed)
        if mode == "safe":
            # A slot returned by the mock is written by the leaf. Reserve a
            # separate area, disjoint from room code, globals, stacks and
            # every fixture object; preserve 4-byte alignment.
            if 0x80140000 <= result <= 0x8014FF00:
                return seed
        elif mode == "negative" and -4096 <= result < 0:
            return seed
        elif mode == "positive" and 0 < result < 4096:
            return seed
        elif mode == "wide" and result >= 0x80000000:
            return seed
    raise AssertionError((index, kind, "no suitable mock seed"))


def _row(label, cmd, timer=0, mock="cos_wide", **kw):
    return dict(label=label, cmd=cmd, timer=timer, mock=mock, **kw)


def _specs(name):
    if name != NAME:
        raise ValueError(f"unsupported M0089I effect leaf: {name}")
    rows = [_row(f"other_cmd_{cmd}", cmd) for cmd in
            (-1, -2147483648, 3, 7, 2147483647)]
    for flag in (0, 1, 2, 255):
        rows.append(_row(f"init_render_byte_{flag}", 0, render_byte=flag))
    for guard in ("erecord_zero", "node_null", "flags_null"):
        rows.append(_row(f"init_guard_{guard}", 0, guard=guard))
    rows.append(_row("init_negative_vector", 0, vector=(-32767, -1, 32767)))
    for offset in (0, 2, 4, 8, 10):
        rows.append(_row(f"init_vector_alias_{offset}", 0, alias=offset))
    for mock in ("cos_negative", "cos_positive", "cos_zero"):
        rows.append(_row(f"init_archive_pool_returns_{mock}", 0, mock=mock))

    # Safe nonzero pool-return seeds allow timer 1 to execute all sixteen
    # iterations plus its additional odd-frame spawn without address faults.
    timers = (-2147483648, -1000, -2, -1, 0, 1, 2, 3, 21, 38, 39, 40,
              41, 55, 56, 57, 63, 64, 65, 1000, 2147483647)
    for timer in timers:
        rows.append(_row(f"update_timer_{timer}_pool_nonnull", 1, timer, "pool_safe"))
    for timer in (-1, 1, 3, 39, 55):
        rows.append(_row(f"update_timer_{timer}_pool_null", 1, timer, "pool_zero"))
    # A zero-near seed may have an invalid pool pointer as its independent
    # mock return, so use even timers that never call the pool getter.
    for timer in (0, 38):
        rows.append(_row(f"update_timer_{timer}_near_zero", 1, timer, "near_zero"))
    for timer in (0, 1, 39, 40):
        rows.append(_row(f"update_timer_{timer}_erecord_zero", 1, timer, "pool_safe",
                         guard="erecord_zero"))
    for flags in (0x01000000, 0x01234567, 0xC1FFFFFF, 0x02000000, 0x3F000000, 0):
        rows.append(_row(f"update_near_flags_{flags:08x}", 1, 38, "pool_safe",
                         flags=flags, actor_flags=0xFFFFFFFF))

    draw_timers = (0, 1, 10, 24, 25, 32, 33, 40, 48, 49, 50, 64, 70, 100,
                   127, 200, -1, -10, -24, -40, -100, -1000, 2147483647, -2147483648)
    for timer, mock in product(draw_timers, ("cos_negative", "cos_positive", "cos_zero", "cos_wide")):
        rows.append(_row(f"draw_timer_{timer}_{mock}", 2, timer, mock))
    for timer, mock in product((24, 25, 32, 33, 48, 49),
                               ("sin_negative", "sin_positive", "sin_zero")):
        rows.append(_row(f"draw_threshold_{timer}_{mock}", 2, timer, mock))
    for mesh_index, tail_index, bank, alt in ((0, 0, 3, 0), (1, 6, 4, 1),
                                            (7, 15, 4, 0), (15, 7, 4, -1)):
        rows.append(_row(f"draw_indices_{mesh_index}_{tail_index}_bank{bank}_alt{alt}", 2, 5,
                         mesh_index=mesh_index, tail_index=tail_index, bank=bank, alt=alt))
    for position in ((-32768, -1, 32767), (2048, -1024, 12288), (100, -50, -4096)):
        rows.append(_row(f"draw_position_{position}", 2, 20, position=position))
    for timer, offset in product((24, 25, 32, 33, 48, 49), (0, 2)):
        rows.append(_row(f"draw_unused_vector_alias_{offset}_timer{timer}", 2, timer, alias=offset))
    assert len(rows) <= 250, len(rows)
    return rows


def descriptions(name):
    return [row["label"] for row in _specs(name)]


def _base(ov, data):
    ram = bytearray(RAM_SIZE + SCRATCH_SIZE)
    start = ov["vram"] & 0x1FFFFFFF
    assert start + len(data) <= RAM_SIZE
    ram[start:start + len(data)] = data
    noise = bytes((i * 37 + 11) & 255 for i in range(256))
    for address in (RECORD, VECTOR, SLOT, NODE, FLAGS, RENDER, ERECORD,
                    ACTOR, ACTOR_FLAGS, POOL_OWNER, POOL, ARCHIVE, MESH1, MESH2, CAMERA):
        off = address & 0x1FFFFFFF
        ram[off:off + 256] = noise
    ram[RAM_SIZE:] = noise * 4
    ram[0x1FE000:RAM_SIZE] = noise * 32
    for address, value in ((0x800E2368, ERECORD), (0x800F32D0, SLOT), (SLOT + 8, NODE),
                           (NODE, FLAGS), (FLAGS + 24, RENDER), (0x8009D254, ACTOR),
                           (ACTOR, ACTOR_FLAGS), (ACTOR_FLAGS + 76, 0x00010010),
                           (0x800F33E0, POOL_OWNER), (POOL_OWNER + 8, POOL),
                           (0x800B0E64, ARCHIVE), (0x8019416C, MESH1), (0x80194168, MESH2),
                           (0x800BCFA4, CAMERA), (FLAGS, 0x01234567)):
        _w(ram, address, value)
    _b(ram, ERECORD + 13, 1)
    _b(ram, RENDER, 1)
    for i, value in enumerate((100, -50, 1200)):
        _h(ram, RECORD + 2 * i, value)
    _h(ram, RECORD + 8, 0x1111)
    _h(ram, RECORD + 10, 0x2222)
    for i, value in enumerate((0x123, -256, 0x456)):
        _h(ram, VECTOR + 2 * i, value)
    for i in range(16):
        _h(ram, 0x800E1204 + i * 2, 100 + i * 7)
        _h(ram, 0x800E2850 + i * 2, 0x120 + i * 3)
    return ram


def cases(name, ov, data):
    base = _base(ov, data)
    for index, row in enumerate(_specs(name)):
        ram = bytearray(base)
        seed = _pick_seed(index, row["mock"])
        _w(ram, 0x800E27EC, row["timer"])
        _h(ram, 0x800E11FA, row.get("mesh_index", 1))
        _h(ram, 0x800E11EA, row.get("tail_index", 6))
        _h(ram, 0x800F336C, row.get("bank", 4))
        _w(ram, 0x800F3428, row.get("alt", 1))
        _b(ram, RENDER, row.get("render_byte", 1))
        _w(ram, FLAGS, row.get("flags", 0x01234567))
        _w(ram, ACTOR_FLAGS + 76, row.get("actor_flags", 0x00010010))
        guard = row.get("guard")
        if guard == "erecord_zero":
            _b(ram, ERECORD + 13, 0)
        elif guard == "node_null":
            _w(ram, SLOT + 8, 0)
        elif guard == "flags_null":
            _w(ram, NODE, 0)
        for i, value in enumerate(row.get("position", (100, -50, 1200))):
            _h(ram, RECORD + i * 2, value)
        for i, value in enumerate(row.get("vector", (0x123, -256, 0x456))):
            _h(ram, VECTOR + i * 2, value)
        vector = RECORD + row["alias"] if "alias" in row else VECTOR
        yield seed, ram, [row["cmd"] & MASK, RECORD, vector, 0]
