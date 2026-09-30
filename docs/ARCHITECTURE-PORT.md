# Port architecture standard

Owner decision, 2026-09-28. The Parasite Eve port follows this pattern, and
the other ports on this dev box should follow it too. The goal is a port
people can customize: better graphics, mods, cheats.

## Accuracy standard (owner, 2026-09-28)

**Purpose: preservation.** The port exists to preserve Parasite Eve exactly as
it shipped, which is why it must be 1:1.

The port must behave **1:1 like retail** — gameplay, logic, visuals and audio.
No approximations, invented behaviour or silent cuts; the earlier
"playable before matching" priority is superseded. Game code runs the
byte-matched decomp C wherever it exists; hand-written stand-ins are temporary
and listed. The only allowed differences are the platform layer (which must
reproduce PS1 hardware behaviour faithfully) and opt-in mods/cheats that are
off by default and inert when off. Every divergence is listed, with its reason,
in [`docs/port/KNOWN_DIVERGENCES.md`](port/KNOWN_DIVERGENCES.md); "matches
retail" is claimed only with objective evidence.

## Layers

```
 game C (matched decomp TUs + hand ports under pc_port/game, pc_port/bootstrap)
   │  calls ONLY the pe_plat_* interfaces below (no PsyCross, no host APIs)
 ┌─┴──────────────────────────────────────────────────────────────┐
 │ platform interfaces (pc_port/include/pe_plat/*.h)              │
 │  renderer · audio · input · disc/storage · timing · mods       │
 └─┬──────────────────────────────────────────────────────────────┘
   │  one or more backends per interface, picked at startup/config
 backends: PsyCross (MIT, fetched, not committed) · in-house (pe_gpu, pe_spu,
           pe_mdec, pe_disc, host_*) · later: Vulkan/OpenGL renderer, other audio
```

Rules:
1. Game code never includes a PsyCross header, a host/OS header or a Sony
   Psy-Q header. Where the retail code called a Psy-Q library function
   (`LoadImage`, `CdRead`, `SsSeqPlay`, …), the port calls a `pe_plat_*`
   function with the same meaning. A backend then implements it.
2. Interfaces are defined by **meaning, not by PS1 registers**. The renderer
   takes primitives, draw environments and VRAM uploads, not GP0 words, so a
   modern renderer can do upscaling, texture replacement, widescreen and
   60 fps interpolation. The PS1-accurate backend is one implementation.
3. No Sony Psy-Q code or tables in any backend (see the psyq audit / guard).
   Game data is never committed or shipped; it is read from the user's disc at
   runtime (see the run-off-the-disc policy).
4. Backends can be swapped without touching game code. The route regression
   (`pc_port/tools/day1_route_regress.sh`) must pass on the default backend
   before every build or swap.

## Interfaces (initial set)

| Interface | Covers | Default backend now |
|---|---|---|
| renderer | draw env/disp env, primitive lists (OT), VRAM upload/download, present, internal resolution, aspect | in-house `pe_gpu` → PsyCross GPU as it lands |
| audio | voices/SPU, sequencer tick, XA/CD audio stream, output device | in-house `pe_spu` + AKAO (matched C) + `host_audio` |
| input | pads (keyboard, joystick), rebinding, turbo/fast-forward keys | `host_pad` |
| disc/storage | sector reads, file lookup on the disc, memory card saves, disc cache | `pe_disc` / `pe_cdreg` → PsyCross libcd shim |
| timing | VSync, root counters, frame pacing, fast-forward, frame-rate target | `host_frame_pacer`, `pe_rcnt2` |
| mods | mod loading, hooks, asset replacement, cheats (below) | new |

## Mod hooks (added step by step)

- `mods/` folder next to the binary (and `~/.local/share/parasite-eve-port/mods/`),
  scanned at startup. Each mod has a manifest (name, version, load order).
- Event hooks: named game events (room enter, battle start/end, item get,
  frame tick, before/after render) that plugins can subscribe to. C plugins
  (shared objects, versioned ABI) first; Lua scripting can sit on the same
  hook table later.
- Texture/asset replacement keyed by a hash of the original data, served
  from `mods/`. Replacement files come from modders; nothing is shipped
  from the disc.
- A cheat menu and console. Cheats are plugins or scripts that use the same
  hooks and guest-state accessors.
- Config file + command-line options: widescreen, internal resolution, frame
  rate (30/60 with interpolation), fast-forward speed, backend selection.
- Test/harness aids (route autopilot, HP-lock) stay behind test flags, not
  in the mod system's default state.

## Migration order (does not stall current work)

1. Add the interface headers and route the existing in-house backends
   through them as each area is touched (audio first, since it is being
   fixed now).
2. Bring in PsyCross behind the same interfaces, replacing the flagged
   Psy-Q-derived pieces (`pe_libcd.c`, `pe_libgpu.c`, `pe_libetc.c`, the
   libpress VLC path).
3. Add the config options and the `mods/` loader with the first hooks
   (room enter, frame tick), then asset replacement, then the cheat console.
4. A modern renderer backend comes later and only needs the renderer interface.
