# BTL151 harness — running procedure

This is the read-only capture harness for
`docs/evidence/pe-btl151-packer-capture/PROMPT.md`. It has not been run here.

## API and watch model

The script follows the existing local Lua watches:

- `PCSX.ExecSlots[address] = function() ... end` for the
  `0x8006E3D4` execution probe;
- `PCSX.Events.createEventListener('Frame', function() ... end)` for polling;
- `PCSX:getRegisters()` with `r.pc` and `r.gpr[31]`/`r.gpr[4]`;
- `PCSX:getMem32(address)` for guest reads, with byte extraction for the six
  name bytes.

The existing templates do not expose a memory-write callback. Therefore the
`D_8009D280` write watch is a read-only frame-transition watch: it records the
old and new observed values and labels the event `frame_transition`. No guest
memory write API is called anywhere in the script.

## Launch

Use the project's Disc 1 retail image and the BTL83 identity gates. The
documented PCSX-Redux flags are:

```text
-interpreter -debugger -gdb -gdb-port 3334 -softgpu -fastboot
```

From the repository root, with the PCSX-Redux AppImage path substituted for
`$PCSX_REDUX` and the retail image path for `$PE_DISC1`:

```bash
PE_BTL151_OUT=/tmp/btl151 \
  "$PCSX_REDUX" \
  -interpreter -debugger -gdb -gdb-port 3334 -softgpu -fastboot \
  -lua tools/pe_btl151_packer_watch.lua \
  "$PE_DISC1"
```

If this build does not accept `-lua` on the command line, launch with the
same flags, then use the built-in Lua editor/console to load
`tools/pe_btl151_packer_watch.lua` before booting the disc. The script prints
an armed line and opens its output files when evaluated.

## Output

The default output directory is `/tmp/btl151`. Override it before launch:

```bash
PE_BTL151_OUT=/absolute/path/to/btl151 "$PCSX_REDUX" ...
```

The script creates and flushes:

```text
PACKER_CALLS.csv
DEST_WRITES.csv
HEARTBEAT.csv
capture.log
```

## Smoke check

During the first 30 seconds of boot, confirm:

1. `capture.log` contains `BTL151 watch armed` and
   `SELF-TEST: ... no guest-memory writes`;
2. `PACKER_CALLS.csv` gains at least one row, proving the exec slot fires;
3. `DEST_WRITES.csv` gains at least one row during boot, proving the observed
   destination transition path is active;
4. `HEARTBEAT.csv` reports `execslots=true`.

If no packer row appears, stop and record the PCSX build/API behavior. Do not
poke RAM to exercise the watch. If the destination remains unchanged during
the first 30 seconds, that is a capture result, not permission to synthesize
a write.

## Stop cleanly

Stop emulation through PCSX-Redux's normal Stop/Quit control, then close the
Lua console/editor. The script flushes every event row and log line. Preserve
the complete `/tmp/btl151` directory; do not edit or filter the raw CSVs before
copying them into the eventual BTL151 evidence directory.
