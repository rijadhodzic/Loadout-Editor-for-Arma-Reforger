# Loadout Editor - Arma Reforger Mod

A multiplayer-compatible loadout editor accessible directly from the in-game arsenal. Build, save, and apply full gear presets without leaving the arsenal screen.

## Features

- **Full gear editing** - primary/secondary weapon, launcher, uniform, vest, backpack, headgear, facewear, misc items
- **Named presets** - save up to 20 presets per player, stored locally in your profile
- **Arsenal-aware** - only shows items available in the current arsenal's pool
- **Multiplayer safe** - all item application is server-validated via RPC

## Installation

1. Subscribe on the Arma Reforger Workshop, or copy the mod folder into your addons directory
2. Enable the mod in the launcher
3. In your scenario's game mode entity, add the `SCR_LoadoutEditorManager` component

## Usage

1. Walk up to an arsenal and open it
2. The Loadout Editor panel appears alongside the arsenal UI
3. Cycle through items per category using the Prev/Next buttons
4. Type a preset name and click **Save**
5. Select a saved preset and click **Load**, then **Apply** to equip it

## Requirements

- Arma Reforger (latest)
- No dependencies

## Notes

- Presets are saved to `$profile:LoadoutEditorPresets.json` (client-side)
- Items are validated server-side before being equipped — invalid or unavailable items are silently skipped
- Existing inventory is **not** cleared before applying a preset (items are added on top)
