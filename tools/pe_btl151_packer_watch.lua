-- PE-BTL151 PCSX-Redux read-only packer/name capture.
--
-- API deliberately follows the existing PE-BTL126/128 watches:
--   * PCSX.ExecSlots[address] for execution probes
--   * PCSX.Events.createEventListener('Frame', ...) for state polling
--   * PCSX:getRegisters() with r.pc and r.gpr[n]
--   * PCSX:getMem32() for guest reads, with byte extraction for unaligned data
--
-- This script never calls a guest-memory write API and never changes input,
-- pad, breakpoint memory, or emulator state.  D_8009D280 is observed by
-- frame-to-frame transition polling because these templates do not expose a
-- memory-write callback.
--
-- Usage:
--   PE_BTL151_OUT=/tmp/btl151 pcsx-redux ... -lua tools/pe_btl151_packer_watch.lua

local OUT = os.getenv("PE_BTL151_OUT") or "/tmp/btl151"

local PACKER = 0x8006E3D4
local DEST = 0x8009D280
local PERSIST0 = 0x800A77F0
local PERSIST4A = 0x800A7918

local function shell_quote(s)
  return "'" .. tostring(s):gsub("'", "'\\''") .. "'"
end

os.execute("mkdir -p " .. shell_quote(OUT))

local function ru32(addr)
  if PCSX.getMem32 then
    local v = PCSX:getMem32(addr)
    if v ~= nil then return v end
  end
  return 0
end

local function ru8(addr)
  local base = addr - (addr % 4)
  local word = ru32(base)
  local shift = (addr % 4) * 8
  return (word >> shift) & 0xFF
end

local function regs()
  local pc, ra, a0, a1, a2, a3 = 0, 0, 0, 0, 0, 0
  if PCSX.getRegisters then
    local r = PCSX:getRegisters()
    pc = r.pc or 0
    if r.gpr then
      ra = r.gpr[31] or 0
      a0 = r.gpr[4] or 0
      a1 = r.gpr[5] or 0
      a2 = r.gpr[6] or 0
      a3 = r.gpr[7] or 0
    end
  end
  return pc, ra, a0, a1, a2, a3
end

local function cycles()
  -- The existing watches do not require cycle reads.  Use the accessor only
  -- when this PCSX build exposes it; otherwise preserve a deterministic 0.
  if PCSX.getCPUCycles then
    local v = PCSX:getCPUCycles()
    if v ~= nil then return v end
  end
  return 0
end

local function hex8(v) return string.format("%02X", (v or 0) & 0xFF) end
local function hex32(v) return string.format("%08X", (v or 0) & 0xFFFFFFFF) end

local function escaped(bytes)
  local out = {}
  for i = 1, #bytes do
    local c = bytes[i]
    if c >= 0x20 and c <= 0x7E and c ~= 0x5C then
      out[#out + 1] = string.char(c)
    elseif c == 0x5C then
      out[#out + 1] = "\\\\"
    else
      out[#out + 1] = string.format("\\x%02X", c)
    end
  end
  return table.concat(out)
end

local function read_name(a0)
  local bytes = {}
  for i = 0, 5 do bytes[#bytes + 1] = ru8(a0 + i) end
  local raw = {}
  for i = 1, #bytes do raw[#raw + 1] = hex8(bytes[i]) end
  return table.concat(raw, ""), escaped(bytes)
end

local function csv_escape(s)
  s = tostring(s or "")
  if s:find('[,\"]') then return '"' .. s:gsub('"', '""') .. '"' end
  return s
end

local function line(values)
  local out = {}
  for i = 1, #values do out[#out + 1] = csv_escape(values[i]) end
  return table.concat(out, ",")
end

local function open_csv(name, header)
  local f = assert(io.open(OUT .. "/" .. name, "w"))
  f:write(header .. "\n")
  f:flush()
  return f
end

local F_CALLS = open_csv("PACKER_CALLS.csv",
  "tick,cycles,pc,ra,caller,a0,name_bytes,name_ascii,v0_at_entry,dest_before,persist0,persist4a")
local F_DEST = open_csv("DEST_WRITES.csv",
  "tick,cycles,pc,ra,old_value,new_value,persist0,persist4a,watch_kind")
local F_HEART = open_csv("HEARTBEAT.csv",
  "tick,cycles,pc,ra,dest,persist0,persist4a,execslots,packer_seen,dest_change_seen")
local F_LOG = assert(io.open(OUT .. "/capture.log", "w"))

local tick = 0
local packer_seen = 0
local dest_change_seen = 0
local last_dest = ru32(DEST)

local function emit(f, values)
  f:write(line(values) .. "\n")
  f:flush()
  io.flush()
end

local function log(fmt, ...)
  local msg = string.format(fmt, ...)
  print(msg)
  F_LOG:write(msg .. "\n")
  F_LOG:flush()
  io.flush()
end

local function snapshot()
  local pc, ra = regs()
  return pc, ra, ru32(DEST), ru32(PERSIST0), ru32(PERSIST4A)
end

function DrawImgui()
end

if PCSX.ExecSlots then
  PCSX.ExecSlots[PACKER] = function()
    local pc, ra, a0 = regs()
    local raw, text = read_name(a0)
    packer_seen = packer_seen + 1
    emit(F_CALLS, {
      tick, cycles(), hex32(pc), hex32(ra), hex32((ra - 8) & 0xFFFFFFFF),
      hex32(a0), raw, text, "", hex32(ru32(DEST)),
      hex32(ru32(PERSIST0)), hex32(ru32(PERSIST4A))
    })
    log("BTL151 PACKER #%u pc=%s ra=%s caller=%s a0=%s name=%s raw=%s dest=%s",
        packer_seen, hex32(pc), hex32(ra), hex32((ra - 8) & 0xFFFFFFFF),
        hex32(a0), text, raw, hex32(ru32(DEST)))
  end
end

PCSX.Events.createEventListener('Frame', function()
  tick = tick + 1
  local pc, ra, dest, persist0, persist4a = snapshot()
  if dest ~= last_dest then
    dest_change_seen = dest_change_seen + 1
    emit(F_DEST, {
      tick, cycles(), hex32(pc), hex32(ra), hex32(last_dest), hex32(dest),
      hex32(persist0), hex32(persist4a), "frame_transition"
    })
    log("BTL151 DEST #%u pc=%s ra=%s old=%s new=%s persist0=%s persist4a=%s",
        dest_change_seen, hex32(pc), hex32(ra), hex32(last_dest), hex32(dest),
        hex32(persist0), hex32(persist4a))
    last_dest = dest
  end
  if tick == 1 or tick % 60 == 0 then
    emit(F_HEART, {
      tick, cycles(), hex32(pc), hex32(ra), hex32(dest), hex32(persist0),
      hex32(persist4a), tostring(PCSX.ExecSlots ~= nil), tostring(packer_seen),
      tostring(dest_change_seen)
    })
  end
end)

log("BTL151 watch armed output=%s execslots=%s packer=%08X dest=%08X",
    OUT, tostring(PCSX.ExecSlots ~= nil), PACKER, DEST)
log("BTL151 SELF-TEST: within 30 seconds expect PACKER_CALLS.csv rows and at least one DEST_WRITES.csv row during boot.")
log("BTL151 NEGATIVE CONTROL: this script performs no guest-memory writes.")
