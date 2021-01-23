# DOOM64-RE-EXPERIMENTAL
This is Immorpher's experimental branch of Doom 64 RE! Where new features are tested out and perhaps one day will be seen in the main branch. Here's Erick194's introduction to cover the rest! Welcome to the complete reverse engineering of Doom 64 by [GEC], this effort took about 1 year and a half although it had not advanced almost nothing, I restarted the whole process from June of this year, theoretically 5 months to complete it from scratch.

## Current Differences in Experimental
### Nightmare Difficulty Changes
This version alters the new "Nightmare!" difficulty in a few ways. The reaction times of the monsters are randomly reduced, which means they sometimes will attack you without much warning. But they won't do this all of the time, just enough to put some anxiety in the player. Conversely the weapon switch speed in this mode is doubled and retains double ammo bonuses. So the mode is meant to put the player on edge but also also give the player all the needed tools for it to be a fair fight.

### Optimizations
F3DEX rendering is replaced with the later F3DEX2 rendering. This should improve geometry calculations. Although this is not a major slowdown in Doom 64, it may result in a slightly more-stable frame rate and perhaps allow for more geometry in future custom maps. Also it is compiled with level 3 optimizations (O3), which improves code execution speed. In some code bases this can be unstable, but so far no issues have been found. Importantly compatibility with Doom 64 original demos are retained.

### New Display Menu Options
The option to turn off the classic N64 3-point linear filtering has been moved from the "Features" cheat menu to the main "Display" menu. Playing without the linear filtering is my prefered way to play and it isn't really a cheat in my opinion. So I felt it belongs in the main game's "Display" menu. Also the default brightness has been set to "100", it's vanilla maximum. But you will see it is only half-way in the Brightness options. You can now increase the brightness beyond the original limits and importantly without over saturating. Visually this does not magically remove the darkness, but I hope increases accessibility. As when Doom 64 was released a major complaint was that it was too dark (even on maximum), and on the 2020 release I have seen a lot of people using the additional brightness slider, so this brings a similar feature to the N64 compatible version.

### New Defaults
As mentioned before the brightness has a new default setting, but the sound is also by default is set to the max. This is my preferred way to play as it's slightly louder than the ambient music. More interestingly there is an additional new defaul gamepad option. If you have a Retro Fighter's controller (or even the Hori or Super Pad 64) option 6 has my preffered layout.

## Installation

You need to download and install the N64 SDK: https://mega.nz/#!AOYDkSxA!MuAqt8iRBk0GGbaqaXVYB9tfZxsquKg5QkbCRL3VOLM
You can also go to this link if it is of your own interest https://n64.dev/

To compile it is required to use Windows XP, you can also use virtual machine for such purposes

Once the N64SDK is installed, copy the "doom64" folder to the root "C:" of your hard drive.

For now it is compatible with the USA version of Doom 64.
Of course, before compiling you need the data of the original Doom64 (DOOM64.WAD | DOOM64.WMD | DOOM64.WSD | DOOM64.WDD).
To obtain them, go to the Tools folder and extract the content of the Doom64Extractor.zip file, "source code included".
Edit the file ExtraerDatos.bat and you change the text "Doom 64 (Usa) .z64" by the name of the rom you have, it is very important that it is in "z64" format later you execute the file ExtraerDatos.bat and copy the extracted data in the folder "Data".
If you can't get the rom in "z64" format there is a file in the Tools folder that is Tool64_v1.11.zip extract them and you can convert the "n64 and v64" formats to "z64".
Finally you run the MAKE_ROM.bat file to compile and obtain a file called doom64.n64

## iQue Installation

You need to download and install the iQue SDK: https://ultra64.ca/resources/software/

To compile it is required to use Red Hat Linux 9, you can also use virtual machine for such purposes

Once the iQue SDK is installed, copy the "doom64" folder to your home folder.

For now it is compatible with the USA version of Doom 64.

Of course, before compiling you need the data of the original Doom64 (DOOM64.WAD | DOOM64.WMD | DOOM64.WSD | DOOM64.WDD).
To obtain them, go to the Tools folder and compile the Doom64 Extractor (instructions in dm64ex.cpp)
Then extract your rom ``./dm64ex -i baserom.us.z64`` 
The romname is an example, however it must be in .z64.

Then copy the extracted data to "Data" folder, then cd to the main doom64 folder and run make:

``make PLATFORM=BB``

Your ROM will be in doom64.n64, and can be ran on your iQue.

## Notes
The project was created with CodeBlocks, although it does not serve to compile, but to have the code in order and verification.

You can also use the WESSLIB.o from the original Mortal Kombat Trilogy n64 and not use the code rebuilt by me.
For this go to the file "wessarc.h" and remove the slashes from the text "//#define NOUSEWESSCODE" it should look like this "#define NOUSEWESSCODE".
Then go to the Makefile and remove the "#" from the following line (WESSLIB = #wesslib.o) it should look like this (WESSLIB = wesslib.o) and I added the "#" in the following line (ASMFILES = wessint_s.s) it should look like this (ASMFILES = #wessint_s.s) and you proceed to compile the Rom.

Special thanks to my brothers for the help to the community in DoomWorld and Kaiser since he is the only one to see the progress of my work and helps me in several occasions.
GEC Team Discord:  https://discord.gg/aEZD4Y7

Immorpher's additional note! Whether obsessed or new to Doom 64, you're welcome on the Doom 64 discord: https://discord.gg/Ktxz8nz

## News
* Features: SECURITY KEYS "locked from the original game, give all the keys available in the level"
* Features: WALL BLOCKING "locked from the original game, I consider it as Noclip"
* Features: LOCK MONSTERS "locked from the original game, stops the movement of enemies like Doom 64 Ex"
* Features: MUSIC TEST "locked from the original game, you can play the music available from the game"
* Features: Colors "new feature for me, turn colors on and off making the game experience more interesting"
* Features: FULL BRIGHT "new feature for me, all colors will be completely clear"
* Features: FILTER "new feature for me, activates and deactivates the 3-point filter of the game"
