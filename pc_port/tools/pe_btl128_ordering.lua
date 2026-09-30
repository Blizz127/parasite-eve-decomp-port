-- PE-BTL128/129 PCSX-Redux watch: retail first-visit dest-enter ordering.
-- Usage (interpreter recommended):
--   PCSX-Redux ... -interpreter -fastboot -lua pc_port/tools/pe_btl128_ordering.lua
-- Do not poke RAM. Logs 1266C / 6BE4C / 125E0 / 17018 kind-4 /
-- 0x2A / 65400 / 12700 / 36448 / B6A80 writes / type-6 PCs / mailboxes.
-- BTL129 also logs 0x77, type-0 +0xCAC/+0x113C, type-2 type-6 sends,
-- and type-6 +0xD08/+0xFC8/+0x1100/+0x1850.

local B6A80 = 0x800B6A80
local DEST = 0x8009D280
local D20C = 0x8009D20C
local D254 = 0x8009D254
local D1A0 = 0x8009D1A0
local D28C = 0x8009D28C
local CE2 = 0x800B0CE2
local CE3 = 0x800B0CE3
local CDB4 = 0x8009CDB4
local A3180 = 0x800A3180
local last_scratch = 0
local last_dest = 0
local saw_1266c = false
local type6_wait_escape = false

local function ru32(addr)
  if PCSX.getMem32 then
    local v = PCSX:getMem32(addr)
    if v ~= nil then return v end
  end
  return 0
end

local function ru8(addr)
  local w = ru32(addr - (addr % 4))
  local sh = (addr % 4) * 8
  return (w >> sh) & 0xFF
end

local function regs()
  local pc, ra = 0, 0
  if PCSX.getRegisters then
    local r = PCSX:getRegisters()
    pc = r.pc or 0
    ra = r.gpr and r.gpr[31] or 0
  end
  return pc, ra
end

local function log(fmt, ...)
  print(string.format(fmt, ...))
  io.flush()
end

local function snapshot(tag)
  local pc, ra = regs()
  log("BTL128 %s pc=%08X ra=%08X dest=%08X scratch0=%08X D20C=%08X D254=%08X D1A0=%08X D28C=%08X CE2=%u CE3=%u mbx=%u",
      tag, pc, ra, ru32(DEST), ru32(B6A80), ru32(D20C), ru32(D254),
      ru32(D1A0), ru32(D28C), ru8(CE2), ru8(CE3), ru8(CDB4))
end

local function walk_actors(tag)
  local actor = ru32(D20C)
  local n = 0
  while actor ~= 0 and n < 16 do
    local typ = ru8(actor + 0x0C)
    local idb = ru8(actor + 0x0D)
    local t0 = ru32(actor + 0xA0)
    local t1 = ru32(actor + 0xA4)
    local t2 = ru32(actor + 0xA8)
    local base = ru32(actor + 0x9C)
    local lab = ru32(actor + 0x19C)
    local pc0 = t2 ~= 0 and ru32(t2) or 0
    local fl = t2 ~= 0 and (ru32(t2 + 8) & 0xFFFF) or 0
    local prox = ru32(actor + 0x1A0)
    log("BTL128 actor %s a=%08X type=%u idB=%u A0=%08X A4=%08X A8=%08X base=%08X lab=%08X prox=%08X pc=%08X f=%04X",
        tag, actor, typ, idb, t0, t1, t2, base, lab, prox, pc0, fl)
    if typ == 6 and base ~= 0 and pc0 ~= 0 then
      local rel = pc0 - base
      if rel == 0x1DC then
        log("BTL128 type6_wait a=%08X task=%08X rel=01DC scratch0=%08X",
            actor, t2, ru32(B6A80))
      elseif rel == 0x1E8 or (rel > 0x1E8 and rel < 0xD08) then
        if not type6_wait_escape then
          type6_wait_escape = true
          log("BTL128 type6_escape a=%08X rel=%04X scratch0=%08X",
              actor, rel, ru32(B6A80))
        end
      end
    end
    actor = ru32(actor + 4)
    n = n + 1
  end
end

function DrawImgui()
end

PCSX.Events.createEventListener('Frame', function()
  local dest = ru32(DEST)
  local cur = ru32(B6A80)
  if dest ~= last_dest then
    snapshot(string.format("dest %08X->%08X", last_dest, dest))
    walk_actors("dest")
    last_dest = dest
  end
  if (cur & 4) ~= 0 and (last_scratch & 4) == 0 then
    snapshot("scratch0|=4")
    walk_actors("bit2")
  end
  last_scratch = cur
end)

-- Optional exec probes if this PCSX build exposes them.
local function hook(addr, name)
  if PCSX.ExecSlots then
    PCSX.ExecSlots[addr] = function()
      if name == "1266C" then
        saw_1266c = true
      end
      snapshot(name)
      if name == "125E0" or name == "65400" or name == "36448"
          or name == "0x77" or name == "0x1C" then
        walk_actors(name)
      end
      if name == "0x2A" then
        local pc, ra = regs()
        log("BTL128 0x2A dest=%08X scratch0=%08X", ru32(DEST), ru32(B6A80))
      end
    end
  end
end

hook(0x8001266C, "1266C")
hook(0x8006BE4C, "6BE4C")
hook(0x800125E0, "125E0")
hook(0x80017018, "17018")
hook(0x80017A50, "0x2A")
hook(0x80065400, "65400")
hook(0x80012700, "12700")
hook(0x80017FF0, "0x89")
hook(0x80017764, "0x1C")
hook(0x80036448, "36448")
hook(0x80012774, "12774")
hook(0x80014DA0, "0x77")

log("BTL128 watch armed saw_execslots=%s", tostring(PCSX.ExecSlots ~= nil))
