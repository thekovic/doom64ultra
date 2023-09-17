# Doom 64 ULTRA

A fork of [Doom 64-RE] with the goal of creating a base for enhanced mods.

## Features

- Jumping/Crouching
- Freelook
- Rumble pak support
- Autoaim option
- Fly mode cheat
- Widescreen support
- N64 mouse support
- High resolution video modes (expansion pak required)
- Persistent saves to SRAM (no more controller pak requirement)
- Quick saves (for saving/loading during a level)
- Lots of bugfixes and optimizations
- Expanded difficulty settings
  - "Hardcore!" from Complete Edition
  - "Nightmare!" from PC Doom
  - "Ultra-Nightmare!" inspired by Doom 2016
  - "Be Merciless!" from Merciless Edition
  - Custom difficulty menu for further options
- Many additional enhancements from [Merciless], [eXpanded] and [Complete] editions
  - Autorun option
  - Map stats option
  - Options to toggle bilinear filtering, dithering, interlacing, anti-aliasing
  - Additional cheats: keys, noclip, lock monsters, colors, fullbright, gamma correct, music test

See also [TODO.md](./TODO.md) for a list of planned future features.

## Building

A Debian-based Linux distribution is required to build. WSL on Windows will also work.

First, CrashOveride's [Modern N64 SDK] must be installed. Then, install these
packages with apt-get:

```sh
sudo apt-get install binutils-mips-n64 gcc-mips-n64 newlib-mips-n64 n64sdk-common n64sdk makemask
```

The data files from the original game must be placed in the `data` folder in
order to build. The required files are: DOOM64.WAD, DOOM64.WMD, DOOM64.WSD,
DOOM64.WDD. Currently these can be extracted from the original game ROM using the
[extractor tool from DOOM64-RE](https://github.com/Erick194/DOOM64-RE/tree/f3792388b43d6fcef087b953f117f4df1bfd261e/doom64/Tools).

Once the SDK packages are installed and the data is in place, the rom can be
built by invoking "make":

```sh
make -j
```

Additional options can be provided to configure or debug the ROM:

```sh
make -j DEBUG=1                      # Turn off optimizations and enable extra debugging features

make -j DEBUGOPT=1                   # Enable extra debugging features but leave optimizations on

make -j DEBUG_DISPLAY=2              # Show debug frame counter by default

make -j REQUIRE_EXPANSION_PAK=0      # Disable expansion pak requirement screen

make -j SKIP_INTRO=1                 # Skip the intro screens on boot and go straight to the menu

make -j WARP=XX                      # Warp to map number XX on boot

make -j WARP=XX SKILL=3              # Warp to map number XX on boot, with the specified difficulty (1-5)

make -j CHEATS=CF_GODMODE+CF_NOCLIP  # Spawn with cheats enabled (see doomdef.h for more CF_ flags)

make -j USB=1                        # Enable Flashcart USB Screenshot/Demo transfers
                                     # Supports EverDrive, 64Drive, SC64
                                     # When used with DEBUG=1, enables debug logging over USB

make -j GDB=1                        # Enable GDB debugging over USB (implies DEBUG=1 and USB=1)
```
## Contact

Join the Doom 64 Discord: https://discord.gg/Ktxz8nz

[Doom 64-RE]: https://github.com/Erick194/DOOM64-RE
[Merciless]: https://github.com/jnmartin84/Doom-64-Merciless-Edition/tree/modern
[eXpanded]: https://github.com/Immorpher/DOOM64XE
[Complete]: https://github.com/azamorapl/doom64-complete-edition
[Modern N64 SDK]: https://crashoveride95.github.io/modernsdk/index.html
