# Changelog

## 2026-09-28 — repository history rewritten

- Removed from all history with git filter-repo: 635 blobs containing retail
  game bytes (embedded tables, instruction words, hex dumps), decoded game
  script text, and 82 game-frame screenshots. The current tree was unchanged
  by the rewrite (tree 22901f1f…).
- All 40 branches were force-pushed. Existing clones and forks keep the old
  objects: re-clone, and do not push old branches back.
- Verification: `build/lanes/hist/verify_rewrite.py` over every blob of every
  rewritten branch reported 0 blobs with retail bytes, script text or game media.
- Going forward, `tools/analysis/retail_data_guard.py` runs in the build, ctest,
  the pre-commit hook (`git config core.hooksPath .githooks`) and CI.
