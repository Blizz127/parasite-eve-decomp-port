#!/usr/bin/env python3
"""Comprehensive persist[0] writer scanner across ALL scenes.

Scans every scene in the field table for persist[0] (index 0) accesses,
focusing on writers that set bit 4 (0x4).  Imports the proven decoder
from pe_pst0_scan.
"""

from __future__ import annotations

import json
import sys
import time
from pathlib import Path

# Re-use the battle-tested functions from the existing scanner.
from tools.research.pe_pst0_scan import (
    FORM1_USER_SIZE,
    HANDLERS,
    PERSIST_BASE,
    PERSIST_WORDS,
    RawMode2Image,
    classify_access,
    decode_script,
    extract_script_from_package,
    find_entry,
    iso_entries,
    parse_field_table,
    sha256_bytes,
    PE_IMG,
    DISC1_EXE,
)

ROOT = Path(__file__).resolve().parents[2]  # repo root
LOCAL_DISC = ROOT / "local" / "pe_disc1.path"


def resolve_disc() -> Path:
    if LOCAL_DISC.is_file():
        p = Path(LOCAL_DISC.read_text().strip().splitlines()[0].strip())
        if p.is_file():
            return p
    raise SystemExit("disc image not found (local/pe_disc1.path)")


def main() -> int:
    disc = resolve_disc()
    handler_09 = HANDLERS[0x09]
    handler_0a = HANDLERS[0x0A]

    t0 = time.time()

    with RawMode2Image(disc) as image:
        entries = iso_entries(image)
        pe_entry = find_entry(entries, PE_IMG)
        field_table = parse_field_table(
            # parse_field_table needs the exe bytes; read it
            image.read_form1_extent(
                find_entry(entries, DISC1_EXE)[1],
                find_entry(entries, DISC1_EXE)[2],
            )
        )

        total = len(field_table)
        scanned = 0
        script_found = 0
        script_missing = 0
        package_missing = 0
        errors = 0

        # Accumulate ALL persist[0] accesses
        persist0_reads: list[dict] = []
        persist0_writes: list[dict] = []
        persist0_writers: list[dict] = []  # only writers
        all_persist0: list[dict] = []

        # Also track every write to persist[0] across all scenes
        # for a comprehensive registry
        bit4_writers: list[dict] = []

        for rec in field_table:
            scanned += 1
            idx = rec["index"]
            name = rec["name"]
            start = rec["start"]
            end = rec["end"]

            if scanned % 50 == 0:
                elapsed = time.time() - t0
                print(f"  [{scanned}/{total}] {name} ({elapsed:.1f}s) ...", file=sys.stderr)

            if end <= start:
                package_missing += 1
                continue

            try:
                blob = image.read_form1_extent(
                    pe_entry[1] + start, (end - start) * FORM1_USER_SIZE
                )
            except Exception as exc:
                errors += 1
                print(f"  ERROR reading {name}: {exc}", file=sys.stderr)
                continue

            extracted = extract_script_from_package(blob, rec["meta"])
            if extracted is None:
                script_missing += 1
                continue

            script_off, script_blob = extracted
            script = decode_script(script_blob)
            if not script["modules"]:
                script_missing += 1
                continue

            script_found += 1

            # Scan every module/command for persist[0] accesses
            for module in script["modules"]:
                for cmd in module["commands"]:
                    for access in classify_access(cmd):
                        index = access.get("index", "")
                        if not isinstance(index, int) or index != 0:
                            continue

                        entry = {
                            "scene": name,
                            "table_index": idx,
                            "module": module["index"],
                            "script_pc": f"+0x{cmd['offset']:04X}",
                            "opcode": f"0x{cmd['opcode']:02X}",
                            "modes": " ".join(str(m) for m in cmd["modes"]),
                            "args": " ".join(f"0x{a:X}" for a in cmd["args"]),
                            "rw": access["rw"],
                            "operation": access["operation"],
                            "value": access.get("value", ""),
                            "value_hex": (
                                f"0x{access['value']:X}"
                                if isinstance(access.get("value"), int)
                                else ""
                            ),
                            "src_bank": access.get("src_bank", ""),
                            "dst_bank": access.get("dst_bank", ""),
                            "alu_desc": access.get("alu_desc", ""),
                            "signedness": access.get("signedness", ""),
                            "notes": access.get("persist_role", ""),
                        }
                        all_persist0.append(entry)

                        if access["rw"] == "write":
                            persist0_writes.append(entry)
                            persist0_writers.append(entry)

                            # Check if this write could set bit 4
                            val = access.get("value")
                            op = access["operation"]

                            # Direct assign with value 4
                            if op == "assign" and isinstance(val, int) and val == 4:
                                bit4_writers.append(
                                    {**entry, "bit4_reason": "direct_assign_4"}
                                )

                            # ALU OR with value 4
                            if op == "alu_or" and isinstance(val, int) and val == 4:
                                bit4_writers.append(
                                    {**entry, "bit4_reason": "alu_or_4"}
                                )

                            # ALU OR with value that includes bit 4
                            if op == "alu_or" and isinstance(val, int) and (val & 4):
                                bit4_writers.append(
                                    {
                                        **entry,
                                        "bit4_reason": f"alu_or_includes_bit4 (val=0x{val:X})",
                                    }
                                )

                            # Assign from cond — flag it, we'd need to check what was computed
                            if op == "assign" and access.get("src_bank") == "cond":
                                # Check the preceding ALU commands in the same module
                                # that wrote to cond
                                # We already have the full cmd list; look at what cond register was written
                                # This is indirect — need to trace back
                                pass

                            # ALU AND clear (AND with ~4) — flag as potential clearer
                            if op == "alu_and" and isinstance(val, int) and (val & 4) == 0:
                                bit4_writers.append(
                                    {
                                        **entry,
                                        "bit4_reason": f"alu_and_clears_bit4 (val=0x{val:X})",
                                    }
                                )

                        else:
                            persist0_reads.append(entry)

                            # Check if the read tests bit 4
                            val = access.get("value")
                            op = access["operation"]
                            if op == "alu_and" and isinstance(val, int) and val == 4:
                                entry["bit4_reason"] = "read_test_bit4"
                            elif op == "alu_and" and isinstance(val, int) and (val & 4):
                                entry["bit4_reason"] = f"read_test_includes_bit4 (val=0x{val:X})"

    elapsed = time.time() - t0

    # --- Analysis: look for indirect writers via cond ---
    # For every persist[0] write from cond, find the preceding ALU op in the same
    # module that wrote to cond and check if it involves 0x4.
    indirect_bit4_writers = []
    for w in persist0_writes:
        if w.get("operation") != "assign" or w.get("src_bank") != "cond":
            continue
        scene = w["scene"]
        table_index = w["table_index"]
        module_idx = w["module"]
        pc_str = w["script_pc"]
        pc = int(pc_str[3:], 16)

        # Find the corresponding scene's script data — we need to re-scan
        # Actually, we need access to the script to find the preceding cond write.
        # We'll note these for manual analysis.
        indirect_bit4_writers.append(
            {
                **w,
                "bit4_reason": "assign_from_cond (needs tracing)",
            }
        )

    # --- Also do a second pass: for each scene with persist[0] write from cond,
    #     re-read the script and look at the ALU chain that fed the cond register.
    # We need to track which cond register index was written.
    # Let's do a targeted second scan for scenes that had persist[0] writes.
    scenes_with_p0_write = {(w["scene"], w["table_index"]) for w in persist0_writes}

    # Second pass: full simulation for scenes with persist[0] writes
    # to trace what value actually gets written to persist[0]
    trace_results = []
    with RawMode2Image(disc) as image:
        entries2 = iso_entries(image)
        pe_entry2 = find_entry(entries2, PE_IMG)
        exe_entry2 = find_entry(entries2, DISC1_EXE)

        for scene_name, tbl_idx in sorted(scenes_with_p0_write):
            # Re-read the scene
            ft_entries = parse_field_table(
                image.read_form1_extent(exe_entry2[1], exe_entry2[2])
            )
            rec = next((r for r in ft_entries if r["index"] == tbl_idx), None)
            if rec is None:
                continue
            start, end = rec["start"], rec["end"]
            if end <= start:
                continue
            blob = image.read_form1_extent(
                pe_entry2[1] + start, (end - start) * FORM1_USER_SIZE
            )
            extracted = extract_script_from_package(blob, rec["meta"])
            if extracted is None:
                continue
            _, script_blob = extracted
            script = decode_script(script_blob)

            for module in script["modules"]:
                # Walk commands looking for persist[0] writes from cond
                # and trace the cond register value backwards
                cond_state: dict[int, int] = {}
                for cmd in module["commands"]:
                    modes = cmd["modes"]
                    args = cmd["args"]
                    op = cmd["opcode"]

                    # Execute ALU ops to track cond register
                    if op == 0x09 and len(args) >= 2:
                        sub = args[0]
                        # dst is mode[1], args[1]
                        if len(modes) > 1 and modes[1] == 3:
                            # This writes to cond register args[1]
                            # Compute the result
                            a = 0
                            b = 0
                            if len(args) > 2 and len(modes) > 2:
                                if modes[2] == 0:
                                    a = args[2] & 0xFFFFFFFF
                                elif modes[2] == 2:
                                    a = 0  # persist unknown
                                elif modes[2] == 3:
                                    a = cond_state.get(args[2], 0)
                            if len(args) > 3 and len(modes) > 3:
                                if modes[3] == 0:
                                    b = args[3] & 0xFFFFFFFF
                                elif modes[3] == 2:
                                    b = 0  # persist unknown
                                elif modes[3] == 3:
                                    b = cond_state.get(args[3], 0)

                            # Check if this ALU op involves bit 4 with persist[0]
                            involves_persist0 = any(
                                m == 2 and args[i] == 0
                                for i, m in enumerate(modes)
                                if i >= 2
                            )

                            # Check for OR with 4
                            if sub == 0x02 and involves_persist0:
                                imm_val = None
                                if len(args) > 3 and len(modes) > 3 and modes[3] == 0:
                                    imm_val = args[3]
                                elif len(args) > 2 and len(modes) > 2 and modes[2] == 0:
                                    imm_val = args[2]
                                if imm_val is not None and (imm_val & 4):
                                    trace_results.append(
                                        {
                                            "scene": scene_name,
                                            "table_index": tbl_idx,
                                            "module": module["index"],
                                            "script_pc": f"+0x{cmd['offset']:04X}",
                                            "operation": "alu_or",
                                            "imm": f"0x{imm_val:X}",
                                            "reason": f"OR with 0x{imm_val:X} on persist[0] feeds cond -> persist[0] write",
                                        }
                                    )

                            # Check for AND with ~4 (clearing bit)
                            if sub == 0x03 and involves_persist0:
                                imm_val = None
                                if len(args) > 3 and len(modes) > 3 and modes[3] == 0:
                                    imm_val = args[3]
                                elif len(args) > 2 and len(modes) > 2 and modes[2] == 0:
                                    imm_val = args[2]
                                if imm_val is not None and (imm_val & 4) == 0:
                                    trace_results.append(
                                        {
                                            "scene": scene_name,
                                            "table_index": tbl_idx,
                                            "module": module["index"],
                                            "script_pc": f"+0x{cmd['offset']:04X}",
                                            "operation": "alu_and_clear",
                                            "imm": f"0x{imm_val:X}",
                                            "reason": f"AND with 0x{imm_val:X} on persist[0] clears bit 4, feeds cond -> persist[0] write",
                                        }
                                    )

                    # Check for direct persist[0] assign with value 4
                    if op == 0x0A and len(args) >= 2 and modes:
                        if modes[0] == 2 and args[0] == 0:
                            src_mode = modes[1] if len(modes) > 1 else -1
                            if src_mode == 0 and len(args) > 1 and args[1] == 4:
                                trace_results.append(
                                    {
                                        "scene": scene_name,
                                        "table_index": tbl_idx,
                                        "module": module["index"],
                                        "script_pc": f"+0x{cmd['offset']:04X}",
                                        "operation": "assign_imm_4",
                                        "imm": "0x4",
                                        "reason": "direct assign 0x4 to persist[0]",
                                    }
                                )
                            if src_mode == 0 and len(args) > 1 and (args[1] & 4):
                                trace_results.append(
                                    {
                                        "scene": scene_name,
                                        "table_index": tbl_idx,
                                        "module": module["index"],
                                        "script_pc": f"+0x{cmd['offset']:04X}",
                                        "operation": f"assign_imm_0x{args[1]:X}",
                                        "imm": f"0x{args[1]:X}",
                                        "reason": f"direct assign 0x{args[1]:X} to persist[0] (includes bit 4)",
                                    }
                                )

    # --- Output ---
    result = {
        "elapsed_seconds": round(elapsed, 1),
        "field_table_entries": total,
        "scenes_scanned": scanned,
        "scripts_found": script_found,
        "scripts_missing": script_missing,
        "packages_missing": package_missing,
        "errors": errors,
        "persist0_total_accesses": len(all_persist0),
        "persist0_reads": len(persist0_reads),
        "persist0_writes": len(persist0_writes),
        "persist0_write_scenes": sorted(
            {w["scene"] for w in persist0_writes}
        ),
        "bit4_direct_writers": [
            w for w in bit4_writers if "direct_assign" in w.get("bit4_reason", "")
        ],
        "bit4_or_writers": [
            w for w in bit4_writers if "alu_or" in w.get("bit4_reason", "")
        ],
        "bit4_and_clearers": [
            w for w in bit4_writers if "clears_bit4" in w.get("bit4_reason", "")
        ],
        "all_bit4_related": bit4_writers,
        "cond_trace_results": trace_results,
        "all_persist0_accesses": all_persist0,
    }

    out_dir = ROOT / "docs" / "evidence" / "pe-pst0-persist-provenance"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "persist0_all_scenes.json"
    out_path.write_text(json.dumps(result, indent=2) + "\n")

    # Print summary
    print(f"\n{'='*70}")
    print(f"PERSIST[0] COMPREHENSIVE SCAN COMPLETE")
    print(f"{'='*70}")
    print(f"Scenes scanned: {scanned} (scripts found: {script_found})")
    print(f"Persist[0] total accesses: {len(all_persist0)}")
    print(f"Persist[0] reads: {len(persist0_reads)}")
    print(f"Persist[0] writes: {len(persist0_writes)}")
    print(f"Persist[0] write scenes: {sorted({w['scene'] for w in persist0_writes})}")
    print()

    if bit4_writers:
        print(f"BIT 4 (0x4) RELATED WRITERS FOUND: {len(bit4_writers)}")
        for w in bit4_writers:
            print(f"  {w['scene']} mod{w['module']} {w['script_pc']} op={w['operation']} "
                  f"args=[{w['args']}] reason={w['bit4_reason']}")
    else:
        print("NO DIRECT BIT 4 (0x4) WRITERS FOUND")

    if trace_results:
        print(f"\nCOND-TRACE RESULTS (indirect bit 4 via cond register): {len(trace_results)}")
        for t in trace_results:
            print(f"  {t['scene']} mod{t['module']} {t['script_pc']} op={t['operation']} "
                  f"imm={t['imm']} -- {t['reason']}")

    print(f"\nFull results: {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
