#!/usr/bin/env python3
"""Render the linux-x64 README-PREVIEW.txt from pc_port/package_readme_preview.txt.

One template serves the private Day-1 preview packager (pc_port/package_preview.sh) and the public release packager
(pc_port/tools/package_public_linux.sh), so the run steps, the archive/folder name and the start-up description cannot drift.

  --tag TAG   release flavour: header "PARASITE EVE — NATIVE PC PORT — TAG (linux-x64)" plus the "Release tag / source commit"
              block (and --note); without it the private "DAY 1 PREVIEW" header is used.
  --default-skip 1  the launcher skips the logo/opening movie/title and starts a NEW GAME directly (the text says so);
              0 (default): the build boots like the original (Square logo, opening movie, title; movies play).
"""
import argparse, pathlib

ap = argparse.ArgumentParser()
ap.add_argument("--template", required=True); ap.add_argument("--out", required=True)
ap.add_argument("--tarball", required=True); ap.add_argument("--dir", required=True)
ap.add_argument("--limits", default="* See build-info.json for the source commit.")
ap.add_argument("--default-skip", default="0", choices=["0", "1"])
ap.add_argument("--dev-menu", required=True, choices=["0", "1"])
ap.add_argument("--tag", default=""); ap.add_argument("--commit", default=""); ap.add_argument("--note", default="")
a = ap.parse_args()

if a.tag:
    title = f"PARASITE EVE — NATIVE PC PORT — {a.tag} (linux-x64)"
    header = f"{title}\n{'=' * len(title)}\n\nRelease tag: {a.tag}   source commit: {a.commit}\n{a.note}\n"
else:
    header = "PARASITE EVE — NATIVE PC PORT — DAY 1 PREVIEW (linux-x64)\n" + "=" * 58 + "\n"
if a.default_skip == "1":
    start = ("The preview starts a NEW GAME directly: the Square logo, opening movie and\n"
             "title screen are skipped (the movie path is not finished yet).")
else:
    start = ("The game starts the way the original does: the Square logo, the opening\n"
             "movie and the title screen play (movies play).")
word = "ENABLED" if a.dev_menu == "1" else "DISABLED"
t = pathlib.Path(a.template).read_text()
for k, v in (("__HEADER__", header.rstrip("\n")), ("__TARBALL__", a.tarball), ("__DIR__", a.dir), ("__START_NOTE__", start),
             ("__LIMITS__", a.limits), ("__DEV_MENU_WORD__", word), ("__DEV_MENU__", a.dev_menu)):
    t = t.replace(k, v)
assert "__" not in "".join(l for l in t.splitlines() if "__" in l and l.strip().startswith("__")), "unreplaced placeholder"
pathlib.Path(a.out).write_text(t)
