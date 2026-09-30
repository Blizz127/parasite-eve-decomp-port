-- PE-BTL126 PCSX-Redux watch: first writer of D_800B6A80 bit 2.
-- Usage (interpreter recommended):
--   PCSX-Redux ... -interpreter -fastboot -lua pc_port/tools/pe_btl126_b6a80_watch.lua
-- Do not poke RAM. Log PC/RA/dest when scratch[0]&4 becomes set.

local ADDR = 0x800B6A80
local DEST = 0x8009D280
local last = 0
local armed = false

local function u32(addr)
  local b0 = PCSX.getMem32 and PCSX:getMem32(addr)
  if b0 ~= nil then
    return b0
  end
  return 0
end

function DrawImgui()
end

PCSX.Events.createEventListener('Frame', function()
  local cur = u32(ADDR)
  if (cur & 4) ~= 0 and (last & 4) == 0 then
    local pc = 0
    local ra = 0
    if PCSX.getRegisters then
      local r = PCSX:getRegisters()
      pc = r.pc or 0
      ra = r.gpr and r.gpr[31] or 0
    end
    print(string.format(
      "BTL126 scratch[0]|=4 pc=%08X ra=%08X dest=%08X old=%08X new=%08X",
      pc, ra, u32(DEST), last, cur))
    io.flush()
  end
  last = cur
  armed = true
end)
