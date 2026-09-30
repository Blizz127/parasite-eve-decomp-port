"""Pure helpers for port.py (importable for unit tests)."""


def derive_field_base(m: dict, a: int, window: int = 0x100):
    """Map an unmapped symbol address ``a`` via struct-field references.

    ``m`` maps source-body addresses to target-body addresses. A body that uses
    ``&D + off`` only yields the field address ``D + off`` in ``m``. Every mapped
    address in ``(a, a + window)`` implies a delta ``m[x] - x``. When all such deltas
    agree, ``a`` maps to ``a + delta``. When there is none, or they disagree,
    return None (the caller refuses the port).
    """
    deltas = {m[x] - x for x in m if 0 < x - a < window}
    if len(deltas) == 1:
        return a + deltas.pop()
    return None
