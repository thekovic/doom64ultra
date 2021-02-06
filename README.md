Merciless Edition is a fork of Doom 64 RE where new features are implemented with the goal of maintaining Nintendo 64 hardware and vanilla gameplay compatibility. Here Doom 64 RE is a complete reverse engineering of Doom 64 by Erick194, which has provided the pathway to allow such modifications.

## Current Differences from Doom 64 RE
### "Be Merciless!" Difficulty
The “Be Merciless!” difficulty setting combines the “Nightmare!” mode newly introduced in Doom 64 RE and with many additional tweaks. As opposed to emulating the difficulties of other Doom 64 ports, it is now an experience focused on brutal action. Enemies are more aggressive, unpredictable, faster, and agile. However, the ammo bonuses are doubled and the weapon switching is faster. Overall, it is a more violent experience which may appeal to fans of the newer iterations of the Doom series.

### Optimizations
F3DEX rendering is replaced with the later F3DEX2 rendering. This should improve geometry calculations. Although this is not a major slowdown in Doom 64, it may result in a slightly more-stable frame rate and perhaps allow for more geometry in future custom maps. Further it is compiled with level 3 optimizations (O3), which improves code execution speed. In some code bases this can be unstable, but so far no issues have been found. Importantly compatibility with the original Doom 64 demos are retained.

### Video Menu Options
The option to turn off the classic N64 3-point linear filtering has been moved from the "Features" cheat menu in Doom 64 RE, to the main "Video" menu in the options. This allows it to be accessible without beating "Hectic" or using a pass code. Also the filtering is no longer removed from the sky effects, allowing them to remain smooth as if at a distance (similar to the Doom 64 2020 remaster). Doom 64 is known for being a particularly dark. Even at maximum brightness many screens still struggle with it. Now the brightness option for Doom 64 has been extended. By default, this version sets the brightness to the original maximum value. However it now can be extended even beyond that by twice without oversaturating the screen. There are also new video options which utilize the Nintendo 64 hardware features, such as anti-aliasing, video dithering, interlacing, and color dithering. Some combinations have a nicely retro look!

### Display Menu Options
Many players have expressed motion sickness when playing Doom 64. There is an overall swaying motion which happens when the player is moving. This also causes the weapon to bob up and down which may be making some players sick. Now in the “Display” menu there is a slider which can reduce this bobbing motion to the point of being turned off. 

### Infrared Goggles Improvement
With the new brightness setting, the infrared goggles have a less noticeable effect. Their utility was under question anyway as they only appear in two main game levels. However, now they have an overall boost to the gradient lighting without altering saturated colors. It is similar to the classic Doom's infrared goggles without the blinding brightness. The goal is to balance utility and aesthetics, which may be useful in the future.

### Other New Default Options
As mentioned the brightness has a new default setting, but also the sound volume default has been set to the maximum. This sets it slightly louder than the ambient music. Further, to honor Retro Fighter’s commitment to continuously improving their N64 controller designs, there is a new default option in the “Game Pad” menu. For controllers where all of the N64 buttons are accessible in one position, like the Retro Fighters controller, variants of the Super Pad 64, or the Hori controller, then the new “Retro Fighters” option might be the optimal layout.

## Doom 64 RE Features Menu Additions
SECURITY KEYS: Inaccessible in the original game, this will give you all the keys available in the level.
WALL BLOCKING: Inaccessible in the original game, this will allow you to walk through walls.
LOCK MONSTERS: Inaccessible in the original game, stops the movement of enemies.
MUSIC TEST: Inaccessible in the original game, you can test all of the music in the game.
COLORS: Added by Erick194, this turns off the colored lighting in the game.
FULL BRIGHT: Added by Erick194, all sectors will be at maximum brightness.
FILTER: Added by Erick194, this activates and deactivates the 3-point linear filter. This has been moved to the "Video" menu by Immorpher.

## Installation
Here is an edited version of Erick194's installation instructions.

You need to download and install the N64 SDK as assembled by Crash Override at this link: https://n64.dev/

These tools require Windows XP, where a virtual machine such as Oracle's Virtual Box might be of use: https://www.virtualbox.org/

Once the N64SDK is installed, copy the "doom64" folder to the root "C:" of the hard drive.

The data extractor utility requires the USA version of Doom 64 1.0 and runs on modern Windows (such as Windows 10). Before attempting to compile, you need to copy the data of the original Doom64 (DOOM64.WAD | DOOM64.WMD | DOOM64.WSD | DOOM64.WDD) from the extractor to the data folder. To obtain them, go to the "Tools" folder and extract the content of the "Doom64Extractor.zip" file (which includes the source code). Edit the file "ExtraerDatos.bat" and change the text "Doom 64 (Usa) .z64" by the name of the rom you have, where it is important that it is in "z64" format. Then execute the file "ExtraerDatos.bat" and copy the extracted data in the folder "Data". If you can't get the rom in "z64" format there is a file in the "Tools" folder that is "Tool64_v1.11.zip" which can convert the "n64 and v64" formats to "z64".
Finally run the "MAKE_ROM.bat" file to compile which generates the Doom 64 rom as "doom64.n64".

## iQue Installation
You need to download and install the iQue SDK: https://ultra64.ca/resources/software/

Red Hat Linux 9 is required for compilation, where a virtual machine as mentioned previously might be of use. Once the iQue SDK is installed, copy the "doom64" folder to the home folder.

The data extractor utility requires the USA version of Doom 64 1.0 as before in the "z64" format. To obtain this utility, go to the "Tools" folder and compile the Doom64 Extractor (instructions in dm64ex.cpp). Then use this command to extract the data of the rom "./dm64ex -i baserom.us.z64". Then copy the extracted data to "Data" folder, and change directory to the main doom64 folder awhere you can use the make command: "make PLATFORM=BB". The ROM will appear as "doom64.n64", and can be ran on the iQue.

## Notes
Erick194 created this project with CodeBlocks for the purposes of organization rather than compilation. Immorpher used Notepad++ to make code updates.

Interestingly, you can also use the WESSLIB.o from the original Mortal Kombat Trilogy on the Nintendo 64 as opposed to the rebuilt code here. For this go to the file "wessarc.h" and remove the slashes from the text "//#define NOUSEWESSCODE". Then go to the Makefile and remove the "#" from the following line "WESSLIB = #wesslib.o". Then add "#" in the following line "ASMFILES = wessint_s.s" to make it into "ASMFILES = #wessint_s.s". Now you can compile the Rom.

Erick194 gives special thanks to his brothers for help, the Doomworld community, and Kaiser for assistance and looking over the progress. Immorpher would like to thank Erick194, Kaiser, and Quasar for their years in effort in the Doom community. Further, Immorpher would like to thank CrashOveride, Buu343, and Elfor for their assitance in the Nintendo 64 community.

GEC Team Discord:  https://discord.gg/aEZD4Y7

Doom 64 Discord: https://discord.gg/Ktxz8nz
