# Doom 64 ULTRA

A fork of [Doom 64-RE] with the goal of creating a base for enhanced mods.

## Features

- Jumping/Crouching
- Freelook
- Autoaim option
- Revamped controls menu
- Fly mode cheat
- Widescreen support
- Lots of bugfixes and optimizations
- Many additional enhancements from [Merciless], [eXpanded] and [Complete] editions
  - "Be Merciless!" Difficulty
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

Once the SDK packages are installed, the rom can be built by invoking "make":

```sh
make
```

Additional options can be provided to configure or debug the ROM:

```sh
make DEBUG=1                          # Don't strip debug symbols
make REQUIRE_EXPANSION_PAK=0          # Disable expansion pak requirement screen
make SKIP_INTRO=1                     # Skip the intro screens on boot and go straight to the menu
make DEVWARP=XX                       # Warp to map number XX on boot
make DEVWARP=XX DEVSKILL=2            # Warp to map number XX on boot, with the specified difficulty (0-4)
```

## Contact

Join the Doom 64 Discord: https://discord.gg/Ktxz8nz

[Doom 64-RE]: https://github.com/Erick194/DOOM64-RE
[Merciless]: https://github.com/jnmartin84/Doom-64-Merciless-Edition/tree/modern
[eXpanded]: https://github.com/Immorpher/DOOM64XE
[Complete]: https://github.com/azamorapl/doom64-complete-edition
[Modern N64 SDK]: https://crashoveride95.github.io/modernsdk/index.html
