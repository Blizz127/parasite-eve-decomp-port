-- PE retail ground-truth replay for PCSX-Redux (headless).
--
-- Drives the retail Disc 1 game from New Game through the first Eve
-- encounter using only pad input and guest-RAM readback (the same signals
-- pe_live_replay.py uses on the native port), then writes a JSON dump of
-- every actor plus ONE screenshot and quits. Never writes guest memory.
--
-- Usage:
--   PE_RETAIL_OUT=/path/prefix pcsx-redux -no-ui -stdout -lua_stdout -run \
--       -iso "<Disc 1>.bin" -dofile tools/pe_retail_replay.lua
--
-- Known emulator caveats (memory: pcsx-redux-headless-capture): skip the
-- opening STR by pressing Start shortly after D_800B0DBA goes nonzero; take
-- only one 24-bit screenshot per run and quit right after it.

local OUT = os.getenv("PE_RETAIL_OUT") or "/tmp/pe-retail"
local START_DELAY = tonumber(os.getenv("PE_RETAIL_START_DELAY") or "30")
local ffi = require("ffi")
local bit = require("bit")
local band, bor, rshift = bit.band, bit.bor, bit.rshift

local log = Support.File.open(OUT .. "-log.txt", "TRUNCATE")
local function say(s)
  log:write(s .. "\n")
  print(s)
end

local mem = PCSX.getMemPtr()
local u32p = ffi.cast("uint32_t*", mem)
local u16p = ffi.cast("uint16_t*", mem)
local u8p = ffi.cast("uint8_t*", mem)
local function U32(a) return u32p[band(a, 0x1FFFFF) / 4] end
local function S32(a) local v = U32(a); if v >= 0x80000000 then v = v - 0x100000000 end; return v end
local function U16(a) return u16p[band(a, 0x1FFFFF) / 2] end
local function U8(a) return u8p[band(a, 0x1FFFFF)] end

local PAD = PCSX.CONSTS.PAD.BUTTON
local pad = PCSX.SIO0.slots[1].pads[1]
local held = {}
local function press(b) if not held[b] then pad.setOverride(b); held[b] = true end end
local function release(b) if held[b] then pad.clearOverride(b); held[b] = nil end end
local function release_all() for b, _ in pairs(held) do pad.clearOverride(b) end; held = {} end

-- ---------------------------------------------------------------- state
local function aya()
  local a = U32(0x8009D20C)
  local n = 0
  while a ~= 0 and a >= 0x80000000 and a < 0x80200000 and n < 64 do
    if U32(a + 0x190) == 0x80035C84 then
      return { addr = a, flags = U32(a + 0x98), x = S32(a + 0x28) / 65536.0,
               y = S32(a + 0x2C) / 65536.0, z = S32(a + 0x30) / 65536.0 }
    end
    a = U32(a + 4); n = n + 1
  end
  return nil
end
local function in_control()
  local a = aya()
  return a ~= nil and a.flags == 8 and band(U32(0x800B0CD8), 0x2000) == 0, a
end
local function battle() return band(U32(0x8009D1A0), 2) ~= 0 end
local function mode() return U32(0x8009D28C) end

-- ---------------------------------------------------------------- driver
local frame = 0
local phase = "boot"
local t = 0            -- frames spent in the current phase step
local step = 0
local tap_timer = 0
local movie_seen = nil
local waypoints = { {-1790, 3420}, {-1790, 3100}, {-2490, 3100}, {-2490, 2500},
                    {-1700, 1200}, {-1400, 50}, {-600, 100} }
local wp = 1
local gain = {}
local probe = nil
local walk_key = nil
local walk_t = 0
local done = false

local function tap(b, every)
  -- press b for 6 frames every `every` frames
  tap_timer = tap_timer + 1
  if tap_timer % every < 6 then press(b) else release(b) end
end

local function set_phase(p)
  say(string.format('{"frame":%d,"phase":"%s"}', frame, p))
  release_all(); phase = p; t = 0; step = 0; tap_timer = 0; walk_key = nil; walk_t = 0; gain = {}; probe = nil
end

local DIRS = { PAD.UP, PAD.DOWN, PAD.LEFT, PAD.RIGHT }
local DIRNAME = { [PAD.UP] = "Up", [PAD.DOWN] = "Down", [PAD.LEFT] = "Left", [PAD.RIGHT] = "Right" }

-- position-feedback walk toward (tx,tz); returns true when within tol
local function walk_to(tx, tz, tol)
  local a = aya()
  if not a then return false end
  local dx, dz = tx - a.x, tz - a.z
  if math.abs(dx) <= tol and math.abs(dz) <= tol then release_all(); walk_key = nil; return true end
  if probe then
    -- probing: hold probe.key for 24 frames, then measure
    probe.t = probe.t + 1
    if probe.t <= 24 then press(probe.key)
    elseif probe.t == 25 then release(probe.key)
    elseif probe.t >= 45 then
      gain[probe.key] = { a.x - probe.x, a.z - probe.z }
      local idx = probe.i + 1
      if idx <= #DIRS then probe = { key = DIRS[idx], i = idx, t = 0, x = a.x, z = a.z } else probe = nil end
    end
    return false
  end
  local best, bestk = nil, nil
  for _, k in ipairs(DIRS) do
    local g = gain[k]
    if g then
      local s = g[1] * dx + g[2] * dz
      if best == nil or s > best then best, bestk = s, k end
    end
  end
  if bestk == nil or best <= 0 then
    release_all(); walk_key = nil
    probe = { key = DIRS[1], i = 1, t = 0, x = a.x, z = a.z }
    return false
  end
  if walk_key ~= bestk then release_all(); walk_key = bestk; walk_t = 0 end
  walk_t = walk_t + 1
  if walk_t % 30 < 24 then press(bestk) else release(bestk) end
  return false
end

local function dump_actors(tag)
  local out = {}
  local a = U32(0x8009D20C)
  local n = 0
  while a ~= 0 and a >= 0x80000000 and a < 0x80200000 and n < 64 do
    out[#out + 1] = string.format(
      '{"addr":"%08X","fn":"%08X","flags":"%08X","type":%d,"id":%d,"x":%d,"y":%d,"z":%d,"dest9C":"%04X","dest9E":%d,"dest9F":%d,"level9A":%d,"cnt8C":%d,"model":"%08X","anim_cmd":%d,"anim_len":%d,"anim_cur":%d,"anim_target":%d}',
      a, U32(a + 0x190), U32(a + 0x98), U8(a + 0xC), U8(a + 0xD),
      math.floor(S32(a + 0x28) / 65536), math.floor(S32(a + 0x2C) / 65536), math.floor(S32(a + 0x30) / 65536),
      U16(a + 0x250), U8(a + 0x252), U8(a + 0x253), U16(a + 0x24E), U8(a + 0x240), U32(a + 0x1B4),
      U8(a + 0xE), U8(a + 0xF), U16(a + 0x16), U16(a + 0x12))
    a = U32(a + 4); n = n + 1
  end
  say(string.format('{"frame":%d,"tag":"%s","D1A0":"%08X","mode":%d,"B0CD8":"%08X","BCF88":"%08X","actors":[%s]}',
    frame, tag, U32(0x8009D1A0), mode(), U32(0x800B0CD8), U32(0x800BCF88), table.concat(out, ",")))
end

local function finish()
  dump_actors("final")
  local shot = PCSX.GPU.takeScreenShot()
  if shot then
    local f = Support.File.open(OUT .. "-shot.bin", "TRUNCATE")
    f:write(shot.data)
    f:close()
    say(string.format('{"frame":%d,"shot":"%s-shot.bin","w":%d,"h":%d,"bpp":%d}', frame, OUT, shot.width, shot.height, shot.bpp))
  end
  log:close()
  done = true
  PCSX.quit(0)
end

PCSX.Events.createEventListener("GPU::Vsync", function()
  if done then return end
  frame = frame + 1
  t = t + 1
  if frame > 60 * 60 * 25 then say("timeout"); finish(); return end
  if frame % 60 == 0 then
    say(string.format('{"frame":%d,"phase":"%s","B0DBA":%d,"D280":"%08X","D154":"%08X","held":%d}',
      frame, phase, U8(0x800B0DBA), U32(0x8009D280), U32(0x8009D154), (function() local n = 0; for _ in pairs(held) do n = n + 1 end; return n end)()))
  end

  if phase == "boot" then
    -- Title map token is A9400048. Tap Cross on the title until New Game
    -- takes us to another token, then tap Start every 30 frames so the STR
    -- movie is skipped (uninterrupted STR playback segfaults this build).
    local tok = U32(0x8009D280)
    local media = U8(0x800B0DBA)
    if media ~= 0 and movie_seen == nil then
      movie_seen = frame; say(string.format('{"frame":%d,"media":%d,"token":"%08X"}', frame, media, tok)); release_all(); tap_timer = 0
    elseif media == 0 and movie_seen ~= nil then
      say(string.format('{"frame":%d,"movie_skipped":"%08X"}', frame, tok)); movie_seen = nil; step = step + 1; release_all(); tap_timer = 0
    end
    if movie_seen then
      -- STR playing: one 8-frame Start press at START_DELAY, then retry every 40
      local dt = frame - movie_seen - START_DELAY
      if dt >= 0 and dt % 40 < 8 then press(PAD.START) else release(PAD.START) end
    elseif frame > 300 then
      tap(PAD.CROSS, 40)
    end
    if aya() ~= nil and step >= 1 then set_phase("opening") end

  elseif phase == "opening" then
    if U32(0x8009D154) ~= 0 then set_phase("name") else tap(PAD.CROSS, 70) end

  elseif phase == "name" then
    if t < 30 then release_all()
    elseif t < 36 then press(PAD.START)
    elseif t < 60 then release(PAD.START)
    elseif t < 66 then press(PAD.CROSS)
    else release(PAD.CROSS) end
    if t > 70 and U32(0x8009D154) == 0 then set_phase("courtyard") end

  elseif phase == "courtyard" then
    local c, a = in_control()
    if c and a.z < -4000 then set_phase("lobby") else tap(PAD.CROSS, 70) end

  elseif phase == "lobby" then
    local a = aya()
    if a and a.z > -1000 and a.z < 4000 and a.y < 0 then set_phase("auditorium") else press(PAD.UP) end

  elseif phase == "auditorium" then
    if aya() == nil then set_phase("performance") else press(PAD.UP) end

  elseif phase == "performance" then
    local c = in_control()
    if c then set_phase("aisle") else tap(PAD.CROSS, 70) end

  elseif phase == "aisle" then
    if battle() then set_phase("battle") end
    if wp > #waypoints then set_phase("battle")
    elseif walk_to(waypoints[wp][1], waypoints[wp][2], 60) then
      say(string.format('{"frame":%d,"waypoint":%d}', frame, wp)); wp = wp + 1
    end

  elseif phase == "battle" then
    if not battle() and mode() == 12 then dump_actors("battle_exit"); set_phase("post") else tap(PAD.CROSS, 48) end

  elseif phase == "post" then
    local c, a = in_control()
    if c and a and math.abs(a.x - 345) < 5 then set_phase("settle") else tap(PAD.CROSS, 70) end

  elseif phase == "settle" then
    release_all()
    if t == 90 then finish() end
  end
end)

do
  local keys = {}
  for k, v in pairs(PAD) do keys[#keys + 1] = tostring(k) .. "=" .. tostring(v) end
  table.sort(keys)
  say("pad buttons: " .. table.concat(keys, " "))
  say("pad methods: setOverride=" .. tostring(pad.setOverride) .. " clearOverride=" .. tostring(pad.clearOverride))
end
say("pe_retail_replay armed")
