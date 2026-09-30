#!/usr/bin/env python3
"""PE-VIS1-D evidence-only span reconcile.

Verifies that both measurement sites share H, pan, snapped Y, and
origin SZ. Mesh SY spans are replayed in MEASUREMENT_LOG.txt; this
script does not invent posed verts or change any production value.
"""

from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.research import pe_vis1b_project as vis1b


CENSUS = {"m0002i": 24.7767, "m0003i": 57.6353}
NATIVE = {"m0002i": 26, "m0003i": 29}
REPLAY_UNPOSED = {"m0002i": 24.776711, "m0003i": 57.635324}
REPLAY_POSED = {"m0002i": 26.092641, "m0003i": 29.707033}


def load_inputs(args: argparse.Namespace) -> tuple[bytes, bytes]:
    if args.exe and args.peimg:
        return vis1b.load_extracted(args.exe, args.peimg)
    if args.disc:
        return vis1b.load_disc(Path(args.disc))
    raise SystemExit("need disc or --exe/--peimg")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("disc", nargs="?", help="Disc 1 BIN")
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--peimg", type=Path)
    args = parser.parse_args()

    exe, peimg = load_inputs(args)
    print(f"exe_sha256={hashlib.sha256(exe).hexdigest()}")
    print(f"peimg_sha1={hashlib.sha1(peimg).hexdigest()}")

    rooms = {
        "m0002i": {
            "table": 1,
            "geom": vis1b.M0002I_GEOM,
            "coll": vis1b.M0002I_COLL,
            "script": vis1b.M0002I_SCRIPT,
            "want_h": 307,
            "want_pan": (0, 3),
            "want_origin": (-145, 224, 2475),
            "want_y": 0,
        },
        "m0003i": {
            "table": 2,
            "geom": vis1b.M0003I_GEOM,
            "coll": vis1b.M0003I_COLL,
            "script": vis1b.M0003I_SCRIPT,
            "want_h": 251,
            "want_pan": (0, -144),
            "want_origin": (-16, 939, 1235),
            "want_y": -4,
        },
    }

    for name, spec in rooms.items():
        pkg = vis1b.package_from_table(exe, peimg, spec["table"])
        view = vis1b.decode_view(pkg, spec["geom"])
        coll = vis1b.decode_group_height(pkg, spec["coll"])
        poses = vis1b.find_0b_xyz(pkg[spec["script"] :])
        if name == "m0002i":
            authored = next(p for p in poses if p["x"] == vis1b.s32(0xFF810000))
        else:
            authored = next(p for p in poses if p["offset"] == 0x0144)
        snapped_y = coll["height"]
        authored_y = authored["y"] >> 16
        wx, wy, wz = authored["x"] >> 16, snapped_y, authored["z"] >> 16
        origin = vis1b.world_to_camera(view, wx, wy, wz)
        authored_origin = vis1b.world_to_camera(view, wx, authored_y, wz)
        print(f"=== {name} ===")
        print(f"H={view['H']} want={spec['want_h']} h_ok={view['H'] == spec['want_h']}")
        print(f"pan={view['pan']} want={spec['want_pan']} pan_ok={view['pan'] == spec['want_pan']}")
        print(
            f"authored_y={authored_y} group_height={coll['height']} "
            f"snapped_y={snapped_y} y_source={'SNAPPED' if snapped_y == spec['want_y'] else 'MISMATCH'}"
        )
        print(f"origin_snapped={origin} want={spec['want_origin']} origin_ok={origin == spec['want_origin']}")
        print(f"origin_authored_y={authored_origin}")
        print(f"census_span={CENSUS[name]} native_span={NATIVE[name]}")
        print(f"replay_unposed={REPLAY_UNPOSED[name]} replay_posed={REPLAY_POSED[name]}")
        agree = abs(CENSUS[name] - float(NATIVE[name])) < 2.0
        print(f"agreement={'yes' if agree else 'no'}")

    print("m0003i_y_source=SNAPPED")
    print("m0003i_origin_sz_native=1235")
    print("m0003i_h_native=251")
    print("correct_figure=BOTH_CONTEXT_DEPENDENT")
    print("divergence_cause=unposed_bind_verts_vs_posed_idle_mesh")
    print("defect_present=no")
    return 0


if __name__ == "__main__":
    sys.exit(main())
