#!/usr/bin/env bash
# scripts/setup_era.sh — fetch the era-accurate PS1 compiler toolchain used for
# functions that GCC 14.2 cannot match (Psy-Q ccpsx = GCC 2.7.2 fingerprint).
#
# Installs into tools/era/ (git-ignored, EXCEPT the locally patched maspsx
# files in MASPSX_TRACKED below — see .gitignore negations):
#   tools/era/gcc-2.7.2-psx/{cpp,cc1,gcc,...}   from decompals/old-gcc (0.17)
#   tools/era/gcc-2.8.1-psx/{cpp,cc1,gcc,...}   from decompals/old-gcc (0.17); selected
#                                                per leaf by ERA_CC1_VER=2.8.1
#   tools/era/maspsx/                            from mkst/maspsx (assembler-macro layer)
#
# Idempotent: skips downloads that are already present. Requires network + curl + git.
# These are open-source community rebuilds / tools — NOT proprietary Psy-Q SDK files.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ERA="$ROOT/tools/era"
GCC_RELEASE="https://github.com/decompals/old-gcc/releases/download/0.17"
GCC_URL="$GCC_RELEASE/gcc-2.7.2-psx.tar.gz"
GCC_DIR="$ERA/gcc-2.7.2-psx"
MASPSX_REPO="https://github.com/mkst/maspsx"
# The tracked local patch (maspsx/__init__.py) is based on this upstream commit;
# a newer upstream maspsx.py imports names the patched __init__.py lacks.
MASPSX_COMMIT="42b862c988fe7a13fe4e7ac0ebec90ed6b9fb763"

mkdir -p "$ERA"

# fetch_gcc <version>: install tools/era/gcc-<version>-psx from the same
# old-gcc release (idempotent; skipped when cc1 is already executable).
# Optional second arg: release asset basename when it is not gcc-<version>-psx.
fetch_gcc() {
    local ver="$1" dir="$ERA/gcc-$1-psx" url="$GCC_RELEASE/${2:-gcc-$1-psx}.tar.gz" tmp
    if [[ -x "$dir/cc1" ]]; then
        echo "OK  era gcc present: $dir"
        return 0
    fi
    echo "Fetching gcc-$ver-psx ..."
    mkdir -p "$dir"
    tmp="$(mktemp)"
    curl -fsSL -o "$tmp" "$url"
    tar xzf "$tmp" -C "$dir"
    rm -f "$tmp"
    [[ -x "$dir/cc1" ]] || { echo "ERROR: cc1 missing after extract ($dir)" >&2; exit 1; }
    echo "OK  installed $dir"
}

fetch_gcc 2.7.2   # default era cc1 (Psy-Q ccpsx fingerprint)
fetch_gcc 2.7.2-cdk gcc-2.7.2-cdk  # per-leaf ERA_CC1_VER=2.7.2-cdk (func_80071A84, libc printf)
fetch_gcc 2.8.1   # per-leaf ERA_CC1_VER=2.8.1 (fills the delay slots 2.7.2 leaves to a nop)

# Repo-tracked files under tools/era/maspsx (un-ignored via .gitignore
# negations) that carry LOCAL patches. A (re)clone copies the upstream working
# tree AROUND these and restores them from git if absent — upstream must never
# overwrite them, and no .git may be left behind (an embedded repo would make
# them untrackable by the parent repo).
MASPSX_TRACKED=(
    "maspsx/__init__.py"
    "tests/test_fill_store_delay_slot.py"
    "tests/test_fill_epilogue_delay_slot.py"
    "tests/test_spill_before_symbol_load.py"
    "tests/test_sink_return_zero_into_flag_delay.py"
    "tests/test_sink_zero_store_into_beqz.py"
    "tests/test_unaligned_word_copy_return_temp.py"
    "tests/test_drop_unused_var8.py"
    "tests/test_swap_s2_s1_save_pairs.py"
    "tests/test_load_before_stack_half.py"
    "tests/test_bnez_postdec_from_minus_one.py"
    "tests/test_beqz_sym_store_delay.py"
    "tests/test_beqz_sym_store_delay_any.py"
    "tests/test_frame48_s5_after_s2.py"
    "tests/test_sink_reg_sw_into_bnez.py"
    "tests/test_three_word_symbol_store.py"
    "tests/test_dispatch_fold.py"
    "tests/test_dispatch_fold_multi.py"
    "tests/test_fill_jump_stack_store.py"
    "tests/test_fill_jump_symbol_store.py"
    "tests/test_drop_target_dup_li.py"
    "tests/test_return_via_epilogue_jump.py"
    "tests/test_load_delay_call_slot_store.py"
    "tests/test_load_delay_nop_before_label.py"
    "tests/test_unfill_epilogue_delay_slot.py"
    "tests/test_div_no_reuse_nop.py"
    "tests/test_la_absolute_small_data.py"
    "tests/test_gp_rel.py"
    "tests/test_dispatch_fold_la.py"
    "tests/test_fill_branch_split_li.py"
    "tests/test_rodata_fold.py"
    "tests/test_load_delay_nop_before_loop_label.py"
    "tests/test_reorder_fill_calls.py"
    "tests/test_fill_call_volatile_store.py"
    "tests/test_symbol_load_dest_temp.py"
    "tests/test_symbol_at_temp.py"
    "tests/test_lo_store_passthrough.py"
    "tests/test_fill_branch_macro_split.py"
    "tests/test_addu_neg1_beq_delay.py"
    "tests/test_div_delay_reuse_v0.py"
    "tests/test_drop_jump_to_epilogue.py"
    "tests/test_sltu_a0_lt_a2_dest_v1.py"
    "tests/test_dup_half_shift.py"
    "tests/test_mult_mflo_to_multu.py"
    "tests/test_mflo_before_mfhi.py"
    "tests/test_fill_branch_delay_slot.py"
    "tests/test_extern_gp_load_delay.py"
    "tests/test_fill_call_delay_slot.py"
    "tests/test_narrow_shifted_word_load.py"
    "tests/test_hoist_block_li.py"
    "tests/test_narrow_shifted_reg_load.py"
    "tests/test_fill_jump_reg_store.py"
    "tests/test_hoist_block_li_tail1.py"
    "tests/test_ulw_load_delay_nop.py"
    "tests/test_shift_count_const.py"
    "tests/test_swap_s1_s2_save_pairs_rev.py"
    "tests/test_symbol_pointer_load_li_after.py"
    "tests/test_branch_steal_jump_slot.py"
    "tests/test_fill_branch_tested_li_void.py"
    "tests/test_sink_volatile_stack_load.py"
    "tests/test_branch_steal_target_insn.py"
    "tests/test_fill_jump_preceding_move.py"
    "tests/test_load_delay_nop_before_any_label.py"
    "tests/test_numeric_load_aspsx.py"
    "tests/test_cdk_split_dispatch.py"
)

if [[ -f "$ERA/maspsx/maspsx.py" ]]; then
    echo "OK  maspsx present: $ERA/maspsx"
else
    echo "Cloning maspsx ..."
    tmp="$(mktemp -d)"
    git clone -q "$MASPSX_REPO" "$tmp/maspsx"
    git -C "$tmp/maspsx" checkout -q "$MASPSX_COMMIT"
    mkdir -p "$ERA/maspsx"
    excludes=(--exclude='./.git')
    for f in "${MASPSX_TRACKED[@]}"; do excludes+=("--exclude=./$f"); done
    ( cd "$tmp/maspsx" && tar cf - "${excludes[@]}" . ) | ( cd "$ERA/maspsx" && tar xf - )
    rm -rf "$tmp"
    for f in "${MASPSX_TRACKED[@]}"; do
        if [[ ! -f "$ERA/maspsx/$f" ]]; then
            if git -C "$ROOT" ls-files --error-unmatch "tools/era/maspsx/$f" >/dev/null 2>&1; then
                git -C "$ROOT" checkout -- "tools/era/maspsx/$f"
                echo "OK  restored tracked file from git: tools/era/maspsx/$f"
            else
                echo "WARNING: tools/era/maspsx/$f is un-ignored but not in git; local patches NOT restored" >&2
            fi
        fi
    done
    [[ -f "$ERA/maspsx/maspsx.py" ]] || { echo "ERROR: maspsx.py missing" >&2; exit 1; }
    echo "OK  cloned $ERA/maspsx"
fi

# Smoke test: era cc1 must reproduce the lui;ori const fingerprint GCC 14.2 can't.
echo "Smoke test: era const-synthesis fingerprint ..."
t="$(mktemp -d)"; printf 'int f(void){return 0x7F7F7F;}\n' >"$t/x.c"
"$GCC_DIR/cpp" "$t/x.c" >"$t/x.i" 2>/dev/null
"$GCC_DIR/cc1" -quiet -O2 -G0 "$t/x.i" -o "$t/x.s" 2>/dev/null
if grep -q 'ori' "$t/x.s"; then echo "OK  era emits lui;ori (matches retail)"; else
    echo "ERROR: era cc1 did not emit ori — wrong compiler?" >&2; exit 1; fi
# The 2.8.1 pair must accept the same cc1 flag set the build profiles use.
"$ERA/gcc-2.8.1-psx/cpp" "$t/x.c" >"$t/y.i" 2>/dev/null
"$ERA/gcc-2.8.1-psx/cc1" -quiet -O2 -G0 "$t/y.i" -o "$t/y.s" 2>/dev/null \
    && echo "OK  gcc-2.8.1-psx cc1 compiles (ERA_CC1_VER=2.8.1 available)" \
    || { echo "ERROR: gcc-2.8.1-psx cc1 failed the smoke compile" >&2; exit 1; }
rm -rf "$t"
echo "Era toolchain ready. The YAML-derived build plan will use it for era-profile leaves."
