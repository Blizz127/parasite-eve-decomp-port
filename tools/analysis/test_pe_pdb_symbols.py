from collections import Counter
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import pe_pdb_symbols as p

GUID = "{1A92F367-8FE4-CBF7-4C4C-44205044422E}"
EXE = f"""File: test.exe
Format: COFF-x86-64
Arch: x86_64
AddressSize: 64bit
ImageFileHeader {{
  Machine: IMAGE_FILE_MACHINE_AMD64 (0x8664)
  SectionCount: 2
    IMAGE_FILE_EXECUTABLE_IMAGE (0x2)
}}
ImageOptionalHeader {{
  Magic: 0x20B
  SizeOfImage: 12288
}}
DOSHeader {{
  Magic: MZ
}}
Sections [
  Section {{
    Number: 1
    VirtualSize: 0x200
    VirtualAddress: 0x1000
    RawDataSize: 512
    PointerToRawData: 0x200
      IMAGE_SCN_CNT_CODE (0x20)
      IMAGE_SCN_MEM_EXECUTE (0x20000000)
  }}
  Section {{
    Number: 2
    VirtualSize: 0x200
    VirtualAddress: 0x2000
    RawDataSize: 512
    PointerToRawData: 0x400
      IMAGE_SCN_CNT_INITIALIZED_DATA (0x40)
  }}
]
DebugDirectory [
  DebugEntry {{
    Type: CodeView (0x2)
    PDBInfo {{
      PDBSignature: 0x53445352
      PDBGUID: {GUID}
      PDBAge: 1
    }}
  }}
]
"""
SUMMARY = f"""Summary
{'=' * 7}
  GUID: {GUID}
  Age: 1
  Has Debug Info: true
  Has Globals: true
  Has Publics: true
  Is stripped: false
  Is incrementally linked: false
"""
MAP = """Section Map
===========
  Section 0000 | ovl = 0, group = 0, frame = 1, name = 65535
    class = 65535, offset = 0, size = 512
    flags = read | execute | 32 bit addr | selector
  Section 0001 | ovl = 0, group = 0, frame = 2, name = 65535
    class = 65535, offset = 0, size = 512
    flags = read | 32 bit addr | selector
  Section 0002 | ovl = 0, group = 0, frame = 3, name = 65535
    class = 65535, offset = 0, size = 4294967295
    flags = 32 bit addr | absolute addr
"""
CONTRIB = """Section Contributions
=====================
  SC[.text] | mod = 0, 0001:0000, size = 32, data crc = 0, reloc crc = 0
    IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ
"""


def publics(items=(('func_80010000', 'function', 1, 0),)):
    result = ["Public Symbols\n==============\n  Publics Header\n    sym hash = 1, thunk table addr = 0000:0000\n  GSI Header",
              f"    sig = 0xFFFFFFFF, hdr = 0xF12F091A, hr size = {8 * len(items)}, num buckets = 1", "  Records"]
    for i, (name, flags, section, offset) in enumerate(items):
        result += [f"  {i * 28} | S_PUB32 [size = 28] `{name}`", f"    flags = {flags}, addr = {section:04}:{offset:04}"]
    result += ["  Hash Entries"] + [f"    off = {i * 28 + 1}, refcnt = 1" for i in range(len(items))]
    result += ["  Hash Buckets", "    0x00000000", "  Address Map"] + [f"    off = {i * 28}" for i in range(len(items))]
    return '\n'.join(result) + '\n'


def modules(items=(('func_80010000', 'S_GPROC32', 1, 0, 32),), absent=None):
    out = ["Symbols\n=======\n  Mod 0000 | `game.obj`:"]
    counts = Counter()
    offset = 4
    for name, kind, section, address, size in items:
        out += [f"  {offset} | {kind} [size = 48] `{name}`",
                f"    parent = 0, end = {offset + 48}, addr = {section:04}:{address:04}, code size = {size}",
                "    type = <no type>, debug start = 0, debug end = 0, flags = none",
                f"  {offset + 48} | S_END [size = 4]"]
        counts[kind] += 1
        counts['S_END'] += 1
        offset += 52
    stats = ["Module Stats\n============\nMod 0000 | `game.obj`:", f"  Stream 11, {offset} bytes", "    Symbols",
             f"    Total: {sum(counts.values())} entries ({offset - 4} bytes)", "    ----------"]
    stats += [f"    {kind}: {count} entries ({count * (4 if kind == 'S_END' else 48)} bytes)" for kind, count in counts.items()]
    stats += ["    Chunks", "    Total: 0 entries (0 bytes)", "    ----------"]
    if absent:
        out += [f"  Mod 0001 | `{absent}`:", "  Error loading module stream 1.  The specified stream could not be loaded. Module stream not present"]
        stats += [f"Mod 0001 | `{absent}`:", f"  Mod 1 (debug info not present): [{absent}]"]
    stats += ["Summary |"]
    return '\n'.join(out) + '\n', '\n'.join(stats) + '\n'


class InventoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.exe = Path(self.temp.name) / 'game.exe'
        self.pdb = Path(self.temp.name) / 'game.pdb'
        self.exe.write_bytes(b'X' * 2048)
        self.pdb.write_bytes(b'PDB fixture')
        syms, stats = modules()
        self.outputs = {'--file-headers': EXE, '--summary': SUMMARY, '--publics': publics(),
                        '--symbols': syms, '--sym-stats': stats, '--section-contribs': CONTRIB + MAP}
        self.commands = []

    def execute(self, modifier=None):
        def run(argv, **kwargs):
            self.commands.append(argv)
            option = argv[1] if argv[1] != 'dump' else argv[2]
            if modifier:
                replacement = modifier(option)
                if replacement is not None:
                    return replacement
            return subprocess.CompletedProcess(argv, 0, self.outputs[option], '')
        with patch.object(p.shutil, 'which', side_effect=lambda name: '/tools/' + name), patch.object(p.subprocess, 'run', side_effect=run):
            return p.inspect(self.exe, self.pdb)

    def fails(self, match=None, modifier=None):
        with self.assertRaisesRegex(ValueError, match or 'PDB inventory'):
            self.execute(modifier)

    def test_complete_identity_audit_and_explicit_commands_no_nm(self):
        result, audit = self.execute()
        self.assertEqual(result, {0x80010000})
        self.assertEqual(audit['guid'], GUID.strip('{}'))
        self.assertEqual(audit['age'], 1)
        self.assertEqual(audit['public_records'], 1)
        self.assertTrue(audit['input_hashes_verified_after'])
        self.assertEqual(len(audit['sha256']['pdb']), 64)
        json.dumps(audit)
        self.assertEqual(len(self.commands), 6)
        self.assertTrue(all(Path(command[0]).name in {p.READOBJ, p.PDBUTIL} for command in self.commands))
        self.assertIn('--public-extras', self.commands[2])
        self.assertIn('--section-map', self.commands[-1])

    def test_wrapper(self):
        with patch.object(p, 'inspect', return_value=({123}, {})):
            self.assertEqual(p.symbols(self.exe, self.pdb), {123})

    def test_executable_permission_cannot_be_hidden_by_data_section_kind(self):
        self.outputs['--file-headers'] = EXE.replace(
            '      IMAGE_SCN_CNT_INITIALIZED_DATA (0x40)',
            '      IMAGE_SCN_CNT_INITIALIZED_DATA (0x40)\n'
            '      IMAGE_SCN_MEM_EXECUTE (0x20000000)')
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(absent='IMPORT.dll')
        # The PDB calls this non-executable data, but the PE memory permission
        # is authoritative. A missing-debug module must not hide code here.
        self.outputs['--section-contribs'] = CONTRIB + (
            '  SC[.data] | mod = 1, 0002:0000, size = 16, data crc = 0, reloc crc = 0\n'
            '    IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ\n') + MAP
        self.fails()

    def test_unsupported_procedure_kind_with_malformed_name_is_not_ignored(self):
        self.outputs['--publics'] = publics((('game_main', 'function', 1, 0),))
        syms, stats = modules((('game_main', 'S_GPROC32', 1, 0, 32),
                               ('func_80010004', 'S_LPROC32_DPC', 1, 40, 16)))
        self.outputs['--symbols'] = syms.replace('`func_80010004`', 'func_80010004')
        self.outputs['--sym-stats'] = stats
        self.fails()

    def test_valid_complete_zero_matching_functions(self):
        self.outputs['--publics'] = publics((('game_main', 'function', 1, 0),))
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules((('game_main', 'S_GPROC32', 1, 0, 32),))
        self.assertEqual(self.execute()[0], set())

    def test_static_and_id_procedures_and_decimal_zero(self):
        items = (('_func_80010004', 'S_LPROC32', 1, 0, 32), ('func_80010008', 'S_GPROC32_ID', 1, 112, 8),
                 ('func_8001000C', 'S_LPROC32_ID', 1, 120, 8))
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(items)
        result, audit = self.execute()
        self.assertEqual(result, {0x80010000, 0x80010004, 0x80010008, 0x8001000c})
        self.assertEqual(audit['local_procedure_records'], 2)

    def test_decimal_offset_is_not_hex(self):
        # 400 decimal fits 512 bytes; 0x400 would not.
        self.outputs['--publics'] = publics((('func_80010000', 'function', 1, 400),))
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules((('func_80010000', 'S_GPROC32', 1, 400, 32),))
        self.assertEqual(self.execute()[0], {0x80010000})

    def test_exact_names_and_data_not_definitions(self):
        names = ['room_a__func_80010004', '.refptr.func_80010008', 'func_8001000C_extra', 'func_800100000', '__func_80010010']
        self.outputs['--publics'] = publics(tuple((n, 'function', 1, 0) for n in names) + (('func_80010014', 'none', 2, 0),))
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(tuple((n, 'S_GPROC32', 1, 0, 8) for n in names))
        self.assertEqual(self.execute()[0], set())

    def test_compiler_suffix_coverage_matches_nm_word_boundary(self):
        self.outputs['--publics'] = publics((('func_80010004.constprop.0', 'function', 1, 0),
                                             ('_func_80010008.isra.0', 'function', 1, 0)))
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules((('_func_8001000C.constprop.0', 'S_LPROC32', 1, 0, 32),))
        self.assertEqual(self.execute()[0], {0x80010004, 0x80010008, 0x8001000c})

    def test_module_data_and_reference_records_do_not_define_functions(self):
        syms, stats = modules((('game_main', 'S_GPROC32', 1, 0, 32),))
        # The ordinary procedure supplies actual debug support; following data
        # and reference records deliberately use the same func naming prefix.
        syms += '  56 | S_LDATA32 [size = 28] `func_80010004`\n    type = <no type>, addr = 0002:0000\n'
        syms += '  84 | S_PROCREF [size = 28] `func_80010008`\n    module = 0, sum name = 0, offset = 4\n'
        stats = stats.replace('2 entries (52 bytes)', '4 entries (108 bytes)').replace('    Chunks', '    S_LDATA32: 1 entries (28 bytes)\n    S_PROCREF: 1 entries (28 bytes)\n    Chunks')
        self.outputs['--symbols'], self.outputs['--sym-stats'] = syms, stats
        self.outputs['--publics'] = publics((('game_main', 'function', 1, 0),))
        self.assertEqual(self.execute()[0], set())

    def test_guid_mismatch(self):
        self.outputs['--summary'] = SUMMARY.replace('1A92F367', '2A92F367')
        self.fails('does not match')

    def test_age_mismatch(self):
        self.outputs['--summary'] = SUMMARY.replace('Age: 1', 'Age: 2')
        self.fails('does not match')

    def test_guid_case_does_not_matter(self):
        self.outputs['--summary'] = SUMMARY.replace(GUID, GUID.lower())
        self.assertEqual(self.execute()[0], {0x80010000})

    def test_stripped_missing_debug_publics_globals_incremental(self):
        for key in ['Has Debug Info', 'Has Publics', 'Has Globals', 'Is stripped', 'Is incrementally linked']:
            with self.subTest(key=key):
                old = 'true' if key.startswith('Has') else 'false'
                new = 'false' if old == 'true' else 'true'
                self.outputs['--summary'] = SUMMARY.replace(f'{key}: {old}', f'{key}: {new}')
                self.fails('unsupported PDB')

    def test_missing_and_unsupported_identity(self):
        for old, new in [('0x53445352', '0x3031424E'), ('COFF-x86-64', 'COFF-i386'), ('Magic: 0x20B', 'Magic: 0x10B'), ('PDBAge: 1', 'PDBAge: 0')]:
            with self.subTest(old=old):
                self.outputs['--file-headers'] = EXE.replace(old, new)
                self.fails()

    def test_duplicate_codeview_identity(self):
        self.outputs['--file-headers'] = EXE.replace('    Type: CodeView (0x2)', '    Type: CodeView (0x2)\n    Type: CodeView (0x2)')
        self.fails('exactly one')

    def test_missing_tool(self):
        with patch.object(p.shutil, 'which', return_value=None), self.assertRaisesRegex(ValueError, 'tool unavailable'):
            p.inspect(self.exe, self.pdb)

    def test_missing_input(self):
        self.pdb.unlink()
        self.fails('not a regular file')

    def test_nonzero_even_with_valid_partial_output(self):
        self.fails('exit 1', lambda option: subprocess.CompletedProcess([], 1, self.outputs[option], 'broken stream'))

    def test_stderr_diagnostic_even_exit_zero(self):
        self.fails('failed', lambda option: subprocess.CompletedProcess([], 0, self.outputs[option], 'warning: incomplete'))

    def test_empty_command_output(self):
        self.fails('empty inventory', lambda option: subprocess.CompletedProcess([], 0, ' \n', ''))

    def test_timeout_and_oserror(self):
        for error in [subprocess.TimeoutExpired('tool', 120), OSError('missing')]:
            def broken(option):
                raise error
            with self.subTest(error=type(error).__name__):
                self.fails('could not produce', broken)

    def test_input_mutation_rejects_successful_inventory(self):
        for target in (self.exe, self.pdb):
            def mutate(option):
                if option == '--section-contribs':
                    target.write_bytes(target.read_bytes() + b'changed')
            with self.subTest(target=target.name):
                self.fails('changed during', mutate)

    def test_public_boundary_truncation_and_malformed_tail(self):
        original = publics()
        for text in [original[:original.index('  Hash Entries')], original[:original.index('  Address Map')], original.rsplit('    off = 0', 1)[0], original + 'error: failed\n']:
            with self.subTest(length=len(text)):
                self.outputs['--publics'] = text
                self.fails()

    def test_public_hash_count_and_offsets_must_agree(self):
        for old, new in [('hr size = 8', 'hr size = 16'), ('off = 1, refcnt', 'off = 2, refcnt')]:
            self.outputs['--publics'] = publics().replace(old, new)
            self.fails()

    def test_matching_public_unknown_flags_or_non_native(self):
        for flags in ('unknown', 'function, managed', 'code'):
            self.outputs['--publics'] = publics((('func_80010000', flags, 1, 0),))
            self.fails()

    def test_matching_public_undefined_data_or_out_of_bounds(self):
        for section, offset in [(0, 0), (2, 0), (1, 512), (3, 0)]:
            self.outputs['--publics'] = publics((('func_80010000', 'function', section, offset),))
            self.fails()

    def test_procedure_zero_span_undefined_nonexec_or_overflow(self):
        for section, offset, size in [(1, 0, 0), (0, 0, 32), (2, 0, 32), (1, 500, 32)]:
            self.outputs['--symbols'], self.outputs['--sym-stats'] = modules((('func_80010000', 'S_GPROC32', section, offset, size),))
            self.fails()

    def test_unknown_matching_proc_kind_rejects(self):
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules((('func_80010004', 'S_GPROC32_FUTURE', 1, 0, 32),))
        self.fails('unsupported procedure kind')

    def test_missing_module_procedure_support(self):
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(())
        self.fails('no procedure/debug support')

    def test_partial_module_records_and_tails(self):
        original = self.outputs['--symbols']
        for text in [original.rsplit('  52 |', 1)[0], original.replace('    type = <no type>, debug start = 0, debug end = 0, flags = none\n', ''), original + 'TRUNCATED\n']:
            self.outputs['--symbols'] = text
            self.fails()

    def test_partial_stats_rejects(self):
        self.outputs['--sym-stats'] = self.outputs['--sym-stats'].replace('    S_GPROC32: 1 entries (48 bytes)\n', '')
        self.fails('partial module symbol statistics')

    def test_missing_module_in_either_inventory(self):
        syms, stats = modules(absent='KERNEL32.dll')
        self.outputs['--symbols'] = syms
        self.fails('disagrees with statistics')

    def test_missing_debug_dll_no_or_data_only_contributions_allowed(self):
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(absent='KERNEL32.dll')
        self.assertEqual(self.execute()[1]['import_modules_without_debug'], 1)
        self.outputs['--section-contribs'] = CONTRIB + '  SC[.data] | mod = 1, 0002:0000, size = 32, data crc = 0, reloc crc = 0\n    IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ\n' + MAP
        self.assertEqual(self.execute()[1]['import_modules_without_debug'], 1)

    def test_missing_debug_dll_executable_contribution_rejects(self):
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(absent='KERNEL32.dll')
        self.outputs['--section-contribs'] = CONTRIB.replace('mod = 0', 'mod = 1') + MAP
        self.fails('contributes executable code')

    def test_missing_debug_ordinary_module_rejects(self):
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(absent='game.obj')
        self.fails('ordinary object module')

    def test_malformed_absent_module_error_rejects(self):
        self.outputs['--symbols'], self.outputs['--sym-stats'] = modules(absent='KERNEL32.dll')
        self.outputs['--symbols'] = self.outputs['--symbols'].replace('Module stream not present', 'stream corruption')
        self.fails('unexpected missing-import')

    def test_partial_contribution_record_or_section_map_rejects(self):
        for text in [CONTRIB, CONTRIB + MAP[:-50], CONTRIB.replace('size = 32', 'size = 999') + MAP, CONTRIB + MAP.replace('size = 512', 'size = 511', 1)]:
            self.outputs['--section-contribs'] = text
            self.fails()

    def test_raw_and_virtual_section_bounds_rejects(self):
        for old, new in [('RawDataSize: 512', 'RawDataSize: 99999'), ('VirtualAddress: 0x2000', 'VirtualAddress: 0x1000'), ('Number: 2', 'Number: 1')]:
            self.outputs['--file-headers'] = EXE.replace(old, new)
            self.fails()


if __name__ == '__main__':
    unittest.main()
