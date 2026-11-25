"""Fail-closed LLVM 21 PE/PDB defined-function inventory for psyq_port_guard.

Only explicit paired EXE/PDB inputs are inspected. No nm fallback, classification,
allowlist policy, logging, runtime execution, or source inspection occurs here.
The supported format is AMD64 PE32+ with RSDS and an unstripped LLVM-readable PDB.
Output spelling is intentionally version-bound: unsupported output fails closed.
"""
from __future__ import annotations

from collections import Counter
import hashlib
from pathlib import Path
import re
import shutil
import subprocess
import uuid

READOBJ = "llvm-readobj-21"
PDBUTIL = "llvm-pdbutil-21"
_FUNCTION = re.compile(r"_?func_([0-9A-Fa-f]{8})\b")
_RECORD = re.compile(r"^[ \t]*(\d+) \| (S_\w+) \[size = (\d+)\](.*)$", re.M)
_MODULE = re.compile(r"^[ \t]*Mod (\d+) \| `([^`\n]+)`:[ \t]*$", re.M)
_PROCEDURES = {"S_GPROC32", "S_LPROC32", "S_GPROC32_ID", "S_LPROC32_ID"}
_NONDEFS = {"S_GDATA32", "S_LDATA32", "S_GTHREAD32", "S_LTHREAD32",
            "S_PROCREF", "S_LPROCREF", "S_DATAREF", "S_CONSTANT", "S_UDT",
            "S_LOCAL"}


def _fail(message: str) -> None:
    raise ValueError("PDB inventory: " + message)


def _one(pattern: str, text: str, label: str, flags: int = re.M) -> re.Match:
    matches = list(re.finditer(pattern, text, flags))
    if len(matches) != 1:
        _fail(f"missing, duplicate, or malformed {label}")
    return matches[0]


def _sha(path: Path) -> str:
    try:
        if not path.is_file():
            _fail(f"input is not a regular file: {path}")
        digest = hashlib.sha256()
        with path.open("rb") as source:
            for chunk in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(chunk)
        return digest.hexdigest()
    except OSError as exc:
        _fail(f"cannot hash {path}: {exc}")


def _run(argv: list[str]) -> str:
    try:
        result = subprocess.run(argv, capture_output=True, text=True,
                                encoding="utf-8", errors="strict", timeout=120)
    except (OSError, subprocess.SubprocessError, UnicodeError) as exc:
        _fail(f"{Path(argv[0]).name} could not produce an inventory: {exc}")
    if result.returncode != 0 or result.stderr.strip():
        _fail(f"{Path(argv[0]).name} failed (exit {result.returncode}): "
              f"{result.stderr.strip()[:400]}")
    if not result.stdout.strip():
        _fail(f"{Path(argv[0]).name} produced an empty inventory")
    return result.stdout


def _guid(value: str) -> str:
    try:
        return str(uuid.UUID(value.strip("{}"))).upper()
    except ValueError:
        _fail("malformed identity GUID")


def _image(text: str, file_size: int) -> tuple[str, int, dict[int, dict]]:
    for line in ("Format: COFF-x86-64", "Arch: x86_64", "AddressSize: 64bit"):
        _one(r"^" + re.escape(line) + r"$", text, line)
    _one(r"^  Machine: IMAGE_FILE_MACHINE_AMD64 \(0x8664\)$", text, "AMD64 machine")
    _one(r"^  Magic: 0x20B$", text, "PE32+ optional header")
    _one(r"^  Magic: MZ$", text, "DOS header")
    _one(r"^    IMAGE_FILE_EXECUTABLE_IMAGE \(0x2\)$", text, "executable image flag")
    nsections = int(_one(r"^  SectionCount: (\d+)$", text, "section count")[1])
    image_size = int(_one(r"^  SizeOfImage: (\d+)$", text, "image size")[1])
    section_area = _one(r"^Sections \[\n(.*?)^\]$", text, "sections", re.M | re.S)[1]
    blocks = list(re.finditer(r"^  Section \{\n(.*?)^  \}$", section_area, re.M | re.S))
    if len(blocks) != nsections or not nsections:
        _fail("incomplete section inventory")
    sections = {}
    spans = []
    for match in blocks:
        block = match[1]
        number = int(_one(r"^    Number: (\d+)$", block, "section number")[1])
        virtual_size = int(_one(r"^    VirtualSize: (0x[0-9A-Fa-f]+)$", block, "virtual size")[1], 16)
        rva = int(_one(r"^    VirtualAddress: (0x[0-9A-Fa-f]+)$", block, "section RVA")[1], 16)
        raw_size = int(_one(r"^    RawDataSize: (\d+)$", block, "raw size")[1])
        raw_start = int(_one(r"^    PointerToRawData: (0x[0-9A-Fa-f]+)$", block, "raw offset")[1], 16)
        if number in sections or virtual_size <= 0 or rva <= 0 or rva + virtual_size > image_size:
            _fail("invalid or duplicate section bounds")
        if raw_size and (raw_start <= 0 or raw_start + raw_size > file_size):
            _fail("section raw bytes exceed executable")
        executable = "IMAGE_SCN_MEM_EXECUTE (0x20000000)" in block
        # Memory permission is authoritative even when the section is labelled
        # data. A missing-debug module must not hide executable contributions
        # behind an absent CNT_CODE bit or a disagreeing PDB section map.
        sections[number] = {"rva": rva, "size": virtual_size, "raw_size": raw_size,
                            "executable": executable}
        spans.append((rva, rva + virtual_size))
    if set(sections) != set(range(1, nsections + 1)):
        _fail("noncontiguous section numbers")
    spans.sort()
    if any(a[1] > b[0] for a, b in zip(spans, spans[1:])):
        _fail("overlapping image sections")
    debug = _one(r"^DebugDirectory \[\n(.*?)^\]$", text, "debug directory", re.M | re.S)[1]
    if len(re.findall(r"^    Type: CodeView \(0x2\)$", debug, re.M)) != 1:
        _fail("require exactly one CodeView identity")
    _one(r"^      PDBSignature: 0x53445352$", debug, "RSDS signature")
    guid = _guid(_one(r"^      PDBGUID: (\{[0-9A-Fa-f-]+\})$", debug, "CodeView GUID")[1])
    age = int(_one(r"^      PDBAge: (\d+)$", debug, "CodeView age")[1])
    if age <= 0:
        _fail("invalid CodeView age")
    return guid, age, sections


def _summary(text: str) -> tuple[str, int]:
    _one(r"^[ \t]*Summary[ \t]*$", text, "PDB summary")
    for key, expected in (("Has Debug Info", "true"), ("Has Globals", "true"),
                          ("Has Publics", "true"), ("Is stripped", "false"),
                          ("Is incrementally linked", "false")):
        value = _one(r"^  " + re.escape(key) + r": (true|false)$", text, key)[1]
        if value != expected:
            _fail(f"unsupported PDB: {key}={value}")
    return (_guid(_one(r"^  GUID: (\{[0-9A-Fa-f-]+\})$", text, "PDB GUID")[1]),
            int(_one(r"^  Age: (\d+)$", text, "PDB age")[1]))


def _location(section: int, offset: int, size: int, sections: dict[int, dict]) -> None:
    info = sections.get(section)
    if not info or not info["executable"] or size <= 0 or offset < 0:
        _fail("function has an undefined or non-executable location")
    if offset + size > min(info["size"], info["raw_size"]):
        _fail("function span exceeds executable section bytes")


def _publics(text: str, sections: dict[int, dict]) -> tuple[set[int], dict]:
    _one(r"^[ \t]*Public Symbols[ \t]*$", text, "public symbol header")
    header = _one(r"^    sig = 0xFFFFFFFF, hdr = 0xF12F091A, hr size = (\d+), num buckets = (\d+)$",
                  text, "public GSI header")
    byte_count = int(header[1])
    if byte_count == 0 or byte_count % 8:
        _fail("missing or invalid public hash inventory")
    expected = byte_count // 8
    names = ("Records", "Hash Entries", "Hash Buckets", "Address Map")
    markers = [_one(r"^  " + name + r"$", text, name) for name in names]
    if [m.start() for m in markers] != sorted(m.start() for m in markers):
        _fail("misordered public inventory")
    body = text[markers[0].end():markers[1].start()]
    record = re.compile(r"[ \t]*(\d+) \| S_PUB32 \[size = (\d+)\] `([^`\n]+)`\n"
                        r"[ \t]*flags = ([a-z, ]+), addr = (\d+):(\d+)[ \t]*\n?")
    records = list(record.finditer(body))
    if record.sub("", body).strip() or len(records) != expected:
        _fail("malformed or partial public records")
    offsets = []
    functions = set()
    function_count = 0
    for item in records:
        offset, size, name, flags, section, address = item.groups()
        if int(size) < 4:
            _fail("invalid public record size")
        offsets.append(int(offset))
        flags_set = set(flags.split(", "))
        if not flags_set <= {"none", "code", "function", "managed", "msil"} or ("none" in flags_set and len(flags_set) != 1):
            _fail("unsupported public flags")
        match = _FUNCTION.match(name)
        if "function" in flags_set:
            function_count += 1
            if match:
                if flags_set & {"managed", "msil"}:
                    _fail("matching public is not a native function")
                _location(int(section), int(address), 1, sections)
                functions.add(int(match[1], 16))
        elif match and "code" in flags_set:
            _fail("matching code public lacks function classification")
    if len(set(offsets)) != expected:
        _fail("duplicate public record offsets")
    hashes = text[markers[1].end():markers[2].start()]
    hash_rx = re.compile(r"[ \t]*off = (\d+), refcnt = (\d+)[ \t]*\n?")
    hash_rows = list(hash_rx.finditer(hashes))
    if hash_rx.sub("", hashes).strip() or len(hash_rows) != expected or any(int(x[2]) <= 0 for x in hash_rows):
        _fail("malformed or partial public hashes")
    if Counter(int(x[1]) - 1 for x in hash_rows) != Counter(offsets):
        _fail("public hash offsets disagree with records")
    buckets = text[markers[2].end():markers[3].start()]
    if re.sub(r"[ \t]*0x[0-9A-Fa-f]+[ \t]*\n?", "", buckets).strip():
        _fail("malformed public hash buckets")
    address_map = text[markers[3].end():]
    address_rx = re.compile(r"[ \t]*off = (\d+)[ \t]*\n?")
    address_rows = list(address_rx.finditer(address_map))
    if address_rx.sub("", address_map).strip() or Counter(int(x[1]) for x in address_rows) != Counter(offsets):
        _fail("malformed or partial public address map")
    if not function_count:
        _fail("public inventory has no function support")
    return functions, {"public_records": expected, "public_function_records": function_count,
                       "public_named_functions": len(functions)}


def _modules(text: str, title: str) -> dict[int, tuple[str, str]]:
    _one(r"^[ \t]*" + re.escape(title) + r"[ \t]*$", text, title)
    matches = list(_MODULE.finditer(text))
    if not matches:
        _fail(f"missing {title} modules")
    result = {}
    for i, match in enumerate(matches):
        number = int(match[1])
        if number in result:
            _fail("duplicate module number")
        result[number] = (match[2], text[match.end():matches[i + 1].start() if i + 1 < len(matches) else len(text)])
    if set(result) != set(range(len(result))):
        _fail("missing module number")
    return result


def _statistics(text: str) -> dict[int, tuple[str, Counter, int, bool]]:
    summary = _one(r"^[ \t]*Summary \|[ \t]*$", text, "module stats summary")
    modules = _modules(text[:summary.start()], "Module Stats")
    result = {}
    for number, (name, block) in modules.items():
        absent = re.fullmatch(r"\s*Mod " + str(number) + r" \(debug info not present\): \[" + re.escape(name) + r"\]\s*", block)
        if absent:
            # Import DLL descriptor modules contain no object-code debug stream.
            if not re.fullmatch(r"[A-Za-z0-9_.+-]+\.dll", name, re.I):
                _fail("ordinary object module is missing debug information")
            result[number] = (name, Counter(), 0, True)
            continue
        _one(r"^  Stream \d+, \d+ bytes$", block, "module stats stream")
        area = _one(r"^[ \t]*Symbols[ \t]*\n(.*?)^[ \t]*Chunks[ \t]*$", block,
                    "module symbol statistics", re.M | re.S)[1]
        total = _one(r"^[ \t]*Total:[ \t]*([\d,]+) entries \([ \t]*([\d,]+) bytes\)$", area, "module symbol total")
        rows = list(re.finditer(r"^[ \t]*(S_\w+):[ \t]*([\d,]+) entries \([ \t]*([\d,]+) bytes\)$", area, re.M))
        counts = Counter()
        sizes = 0
        for row in rows:
            if row[1] in counts:
                _fail("duplicate symbol statistics kind")
            counts[row[1]] = int(row[2].replace(",", ""))
            sizes += int(row[3].replace(",", ""))
        if sum(counts.values()) != int(total[1].replace(",", "")) or sizes != int(total[2].replace(",", "")):
            _fail("partial module symbol statistics")
        result[number] = (name, counts, sizes, False)
    return result


def _contributions(text: str, sections: dict[int, dict], module_numbers: set[int]) -> tuple[set[int], int]:
    _one(r"^[ \t]*Section Contributions[ \t]*$", text, "section contributions")
    sentinel = _one(r"^[ \t]*Section Map[ \t]*$", text, "section map after contributions")
    body = text[:sentinel.start()]
    header = _one(r"^=+$", body, "section contribution divider")
    body = body[header.end():]
    record = re.compile(r"^[ \t]*SC\[[^\]\n]+\][ \t]*\| mod = (\d+), (\d+):(\d+), size = (\d+), data crc = \d+, reloc crc = \d+\n"
                        r"((?:[ \t]+IMAGE_SCN_[A-Z0-9_]+(?:[ \t]*\|[ \t]*(?:IMAGE_SCN_[A-Z0-9_]+)?)*[ \t]*\n)+)", re.M)
    rows = list(record.finditer(body))
    if not rows or record.sub("", body).strip():
        _fail("malformed or partial section contribution inventory")
    executable_modules = set()
    for row in rows:
        module, section, offset, size = map(int, row.groups()[:4])
        info = sections.get(section)
        if module not in module_numbers or not info or offset + size > info["size"]:
            _fail("invalid section contribution bounds or module")
        if size and (info["executable"] or "IMAGE_SCN_MEM_EXECUTE" in row[5] or "IMAGE_SCN_CNT_CODE" in row[5]):
            executable_modules.add(module)
    # A complete following section map proves the contributions dump reached its
    # next section; validate it against the independently read executable bounds.
    map_text = text[sentinel.end():]
    divider = _one(r"^=+$", map_text, "section map divider")
    map_text = map_text[divider.end():]
    map_record = re.compile(r"[ \t]*Section (\d+) \| ovl = 0, group = 0, frame = (\d+), name = 65535\n"
                            r"[ \t]*class = 65535, offset = 0, size = (\d+)\n"
                            r"[ \t]*flags = ([^\n]+)\n?")
    maps = list(map_record.finditer(map_text))
    if map_record.sub("", map_text).strip() or len(maps) != len(sections) + 1:
        _fail("malformed or partial section map")
    for i, row in enumerate(maps):
        if int(row[1]) != i or int(row[2]) != i + 1:
            _fail("invalid section map order")
        if i < len(sections):
            info = sections[i + 1]
            if int(row[3]) != info["size"] or ("execute" in row[4]) != info["executable"]:
                _fail("PDB section map disagrees with executable")
        elif int(row[3]) != 4294967295 or row[4].strip() != "32 bit addr | absolute addr":
            _fail("unsupported absolute section map entry")
    return executable_modules, len(rows)


def _procedures(text: str, stats_text: str, contributions: str, sections: dict[int, dict]) -> tuple[set[int], dict]:
    modules = _modules(text, "Symbols")
    stats = _statistics(stats_text)
    if modules.keys() != stats.keys():
        _fail("module inventory disagrees with statistics")
    executable_modules, contribution_count = _contributions(contributions, sections, set(modules))
    functions = set()
    procedure_count = local_count = absent_count = record_count = 0
    for number, (name, block) in modules.items():
        expected_name, counts, size, absent = stats[number]
        if name != expected_name:
            _fail("module name disagrees with statistics")
        if absent:
            if number in executable_modules:
                _fail("missing-debug module contributes executable code")
            if not re.fullmatch(r"\s*Error loading module stream " + str(number) +
                                r"\.  The specified stream could not be loaded\. Module stream not present\s*", block):
                _fail("unexpected missing-import-module output")
            absent_count += 1
            continue
        records = list(_RECORD.finditer(block))
        actual = Counter(record[2] for record in records)
        if actual != counts or sum(int(x[3]) for x in records) != size:
            _fail("partial or malformed module records")
        if records and block[:records[0].start()].strip():
            _fail("unexpected module inventory preamble")
        if re.search(r"^[ \t]*(?:Error|error|warning|Warning)\b", block, re.M):
            _fail("module inventory contains a diagnostic")
        offsets = {int(record[1]): record[2] for record in records}
        if len(offsets) != len(records):
            _fail("duplicate module record offset")
        if records and (int(records[0][1]) != 4 or any(int(a[1]) + int(a[3]) != int(b[1]) for a, b in zip(records, records[1:]))):
            _fail("noncontiguous module records")
        record_count += len(records)
        for i, record in enumerate(records):
            kind, suffix = record[2], record[4]
            body = block[record.end():records[i + 1].start() if i + 1 < len(records) else len(block)]
            name_match = re.fullmatch(r" `([^`\n]+)`", suffix)
            symbol_name = name_match[1] if name_match else None
            matching = _FUNCTION.match(symbol_name) if symbol_name else None
            # Unknown definitions must not hide a matching func behind a new kind.
            if kind not in _PROCEDURES:
                if kind.startswith(("S_GPROC", "S_LPROC")) and kind not in _NONDEFS:
                    _fail(f"unsupported procedure kind {kind}")
                if kind in {"S_END", "S_PROC_ID_END"} and (suffix.strip() or body.strip()):
                    _fail("malformed procedure end record")
                if matching and kind not in _NONDEFS:
                    _fail(f"unsupported matching symbol kind {kind}")
                continue
            if symbol_name is None:
                _fail("malformed procedure name")
            fields = _one(r"^[ \t]*parent = (\d+), end = (\d+), addr = (\d+):(\d+), code size = (\d+)$",
                          body, "procedure location")
            tail = _one(r"^[ \t]*type = .*, debug start = (\d+), debug end = (\d+), flags = [^\n]*$",
                        body, "procedure metadata")
            if body[:fields.start()].strip() or body[fields.end():tail.start()].strip() or body[tail.end():].strip():
                _fail("unexpected or partial procedure record")
            code_size = int(fields[5])
            if offsets.get(int(fields[2])) not in {"S_END", "S_PROC_ID_END"}:
                _fail("procedure lacks its end record")
            if int(tail[1]) > code_size or int(tail[2]) > code_size:
                _fail("invalid procedure debug span")
            _location(int(fields[3]), int(fields[4]), code_size, sections)
            procedure_count += 1
            if kind.startswith("S_LPROC"):
                local_count += 1
            if matching:
                functions.add(int(matching[1], 16))
    if not procedure_count:
        _fail("module inventory has no procedure/debug support")
    return functions, {"modules": len(modules), "import_modules_without_debug": absent_count,
                       "section_contributions": contribution_count,
                       "module_records": record_count, "module_procedure_records": procedure_count,
                       "local_procedure_records": local_count, "module_named_functions": len(functions)}


def _inspect(exe: Path, pdb: Path) -> tuple[set[int], dict]:
    """Return nm-compatible func_XXXXXXXX function addresses and identity audit.

    Any unavailable tool, unsupported format, diagnostic, incomplete inventory,
    identity mismatch, invalid matching-function span, or input mutation raises
    ValueError. An otherwise complete inventory with zero matching names is valid.
    """
    exe, pdb = Path(exe).resolve(), Path(pdb).resolve()
    if exe == pdb:
        _fail("EXE and PDB must be different files")
    tools = {}
    for name in (READOBJ, PDBUTIL):
        tools[name] = shutil.which(name)
        if tools[name] is None:
            _fail(f"required tool unavailable: {name}")
    before = {"exe": _sha(exe), "pdb": _sha(pdb)}
    readobj = _run([tools[READOBJ], "--file-headers", "--coff-debug-directory", "--sections", str(exe)])
    identity_guid, identity_age, sections = _image(readobj, exe.stat().st_size)
    summary = _run([tools[PDBUTIL], "dump", "--summary", str(pdb)])
    if _summary(summary) != (identity_guid, identity_age):
        _fail("EXE CodeView GUID/age does not match supplied PDB")
    publics = _run([tools[PDBUTIL], "dump", "--publics", "--public-extras", str(pdb)])
    public_symbols, public_audit = _publics(publics, sections)
    records = _run([tools[PDBUTIL], "dump", "--symbols", str(pdb)])
    statistics = _run([tools[PDBUTIL], "dump", "--sym-stats", str(pdb)])
    contributions = _run([tools[PDBUTIL], "dump", "--section-contribs", "--section-map", str(pdb)])
    module_symbols, module_audit = _procedures(records, statistics, contributions, sections)
    after = {"exe": _sha(exe), "pdb": _sha(pdb)}
    if before != after:
        _fail("EXE or PDB changed during inventory")
    addresses = public_symbols | module_symbols
    audit = {"schema": "pe-pdb-symbols-v1", "exe": str(exe), "pdb": str(pdb),
             "guid": identity_guid, "age": identity_age, "sha256": before,
             "input_hashes_verified_after": True, "tools": tools,
             "named_functions": len(addresses), **public_audit, **module_audit}
    return addresses, audit


def inspect(exe: Path, pdb: Path) -> tuple[set[int], dict]:
    """Inspect a paired AMD64 image/PDB; failures are always ValueError.

    Names use the guard's anchored optional-underscore func_HEX word boundary,
    including compiler suffixes such as .constprop.0 but excluding _helper.
    """
    try:
        return _inspect(exe, pdb)
    except OSError as exc:
        _fail(f"cannot read inputs: {exc}")


def symbols(exe: Path, pdb: Path) -> set[int]:
    """Convenience wrapper for callers that do not need audit metadata."""
    return inspect(exe, pdb)[0]
