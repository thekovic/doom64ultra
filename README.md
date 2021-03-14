Merciless Edition is a fork of Doom 64 RE where new features for Doom 64 are implemented with the goal of maintaining Nintendo 64 hardware and vanilla gameplay compatibility. Where Doom 64 RE is a complete reverse engineering of Doom 64 by Erick194, which has provided the pathway to achieve such modifications. This branch is primarily hardware focused, however emulation compatibility is still a goal.

## Current Differences from Doom 64 RE and Original Doom 64
### "Be Merciless!" Difficulty
The “Be Merciless!” difficulty setting combines parts of the “Nightmare!” mode introduced in Doom 64 RE with the "Hardcore!" mode of Doom 64 EX and other tweaks. As opposed to exactly copying the difficulties of other Doom 64 ports, it is now an experience focused on brutal action. Enemies are more relentless, unpredictable, faster, and agile. However, the ammo bonuses are increased and the weapon switching is faster. Overall, it is a more violent experience which may appeal to fans of the newer iterations of the Doom series.

### Menu Reorganization and Defaults Menu
Many of the menus have been reorganized and renamed to accomodate the additional options and accessibility. A new "Video" menu has been added to toggle the various new video modes. Then many of the new accessibility options have been added to the "Display" menu. There is also a new "Status HUD" menu to change how the HUD looks. Further, the "Password" menu option has been moved to the main menu for easier access. There is also a new "Defaults" menu to restore the game to the original Doom 64 settings, Merciless Edition settings, Immorpher's settings, or Accessible settings for increased accessibility.

### Movement Menu Options
Many players have expressed motion sickness when playing Doom 64. There is an overall swaying motion which happens when the player is moving. This also causes the weapon to bob up and down which may be making some players sick. Now there is a slider which can reduce this bobbing motion to the point of being turned off. Digital movement in Doom 64 was restricted to "walking" speed, unless the "speed" button is pressed. The new "Autorun" setting defaults digital movement to running, where now the "speed" button slows the player down.

### Video Menu Options
The option to turn off the classic N64 3-point linear filtering has been moved from the "Features" cheat menu in Doom 64 RE, to the main "Video" menu in the options. This allows it to be accessible without beating "Hectic" or using a pass code. Further, this option has been extended to filter skies only (like Doom 64 Remaster) or completely off (like Doom 64 EX). Doom 64 is known for being a particularly dark. Even at maximum brightness many screens still struggle with it. Now the brightness option has been extended. By default, this version sets the brightness to the original maximum value. However it now can be extended even beyond that by twice without oversaturating the screen. Further, there are additional video options which utilize the Nintendo 64 hardware features, such as anti-aliasing, video dithering, interlacing, and color dithering. Some combinations have a nicely retro look!

### Display Menu Options
The "Display" options now contains a flash brightness option to make the game more accessible. Some parts of the game have intense flashing, particularly the first megasphere in "Holding Area", which has the risk of inducing seizures in some players. Now there is an option to reduce or turn off these flashes. And if you get tired of waiting through the intermission text screens, there is a new option to skip them. Finally, there is an option to display the map statistics in the automap screen, if you're a completionist.

### Status HUD Options
There are more options now to modify the status bar and HUD. By default the new colored HUD will be enabled, which changes color depending on armor type, ammo type, and health status. This can be disabled in the menu. Doom 64 by default has a margin around the screen to displace the HUD elements for older TV safe zones. Now this can be adjusted. Also the transparency of the HUD can be adjusted.

### Other New Default Options
As mentioned the brightness has a new default setting, but also the sound volume default has been set to the maximum. This sets it slightly louder than the ambient music. Further, to honor Retro Fighter’s commitment to continuously improving their N64 controller designs, there is a new default option in the “Gamepad” menu. For controllers where all of the N64 buttons are accessible in one position, like the Retro Fighters controller, variants of the Super Pad 64, or the Hori controller, then the new “Retro Fighters” option might be the optimal layout.

### Story Screens Update
The story screens now have the menu ambience as opposed to silence. When a new game is started, there is a abbreviated introductory screen adapted from the N64 manual and Doom 64 Reloaded. The "Demon" text has been changed to "Bull Demon" in the ending sequence and also the "Mother Demon" makes an appearance. The intermissions can be turned off any time in the "Display" menu, or by pressing buttons the screens can be sped up.

### Messaging System Update
Inspired by the messaging system of the Doom 64 Remaster, now up to three messages (as opposed to one) will appear on screen at once. Hopefully this makes it easier to figure out what has been picked up or if you have found a secret area. The messaging system also supports colors now, where secrets and secret items will appear as blue. If you happen to die, stick on that screen for a while to see special messages from the Doom 64 community.

### Game and Save Management
Now the memory pak save files include abbreviations for the difficulty. It is easier to keep all of those Doom 64 saves in order. Also if your playthrough of Doom 64 is getting too hard or easy, you can restart your game at the current level with the difficulty of your choosing. Which means you can change your difficulty midway through the game.

### Zombiemen and Text Restorations
The third sight and death sounds in Doom 64 would go unplayed due to the limited randomization in the code. Now this randomization routine has been updated such that these sounds will be played. Also the missing "You pick up a medikit that you REALLY need!" has been restored.

### Other Updates
With the new brightness setting, the infrared goggles have a less noticeable effect. However, now they have an overall boost to the gradient lighting without altering saturated colors. Exiting to "Hectic" will now will give you a stats screen and allow you to save.

### Optimizations
F3DEX rendering is replaced with the later F3DEX2 rendering. This should improve geometry calculations. Although this is not a major slowdown in Doom 64, it may result in a slightly more-stable frame rate and perhaps allow for more geometry in future custom maps. Further it is compiled with level 3 optimizations (O3), which improves code execution speed. In some code bases this can be unstable, but so far no issues have been found. Importantly compatibility with the original Doom 64 demos are retained.

## Features Menu
These are bonus additions to the features menu added by Erick194 for Doom 64 RE!
SECURITY KEYS: Inaccessible in the original game, this will give you all the keys available in the level.
WALL BLOCKING: Inaccessible in the original game, this will allow you to walk through walls.
LOCK MONSTERS: Inaccessible in the original game, stops the movement of enemies.
MUSIC TEST: Inaccessible in the original game, you can test all of the music in the game.
COLORS: Added by Erick194, this turns off the colored lighting in the game.
FULL BRIGHT: Added by Erick194, all sectors will be at maximum brightness.

And these are the features added for the "Merciless Edition".
WARP: Features menu now can warp to the final and secret levels.
GAMMA CORRECT: This utilizes the N64 brightness adjustment, making the game very bright.
MERCILESS CREDITS: Those who have helped the "Merciless Edition" make it to completion.

## Source Code Installation
### Nintendo 64
Here is an edited version of Erick194's installation instructions.

You need to download and install the N64 SDK as assembled by Crash Override at this link: https://n64.dev/

These tools require Windows XP, where a virtual machine such as Oracle's Virtual Box might be of use: https://www.virtualbox.org/

Once the N64SDK is installed, copy the "doom64" folder to the root "C:" of the hard drive.

The data extractor utility requires the USA version of Doom 64 1.0 and runs on modern Windows (such as Windows 10). Before attempting to compile, you need to copy the data of the original Doom64 (DOOM64.WAD | DOOM64.WMD | DOOM64.WSD | DOOM64.WDD) from the extractor to the data folder. To obtain them, go to the "Tools" folder and extract the content of the "Doom64Extractor.zip" file (which includes the source code). Edit the file "ExtraerDatos.bat" and change the text "Doom 64 (Usa) .z64" by the name of the rom you have, where it is important that it is in "z64" format. Then execute the file "ExtraerDatos.bat" and copy the extracted data in the folder "Data". If you can't get the rom in "z64" format there is a file in the "Tools" folder that is "Tool64_v1.11.zip" which can convert the "n64 and v64" formats to "z64".
Finally run the "MAKE_ROM.bat" file to compile which generates the Doom 64 rom as "doom64.n64".

### iQue
You need to download and install the iQue SDK: https://ultra64.ca/resources/software/

Red Hat Linux 9 is required for compilation, where a virtual machine as mentioned previously might be of use. Once the iQue SDK is installed, copy the "doom64" folder to the home folder.

The data extractor utility requires the USA version of Doom 64 1.0 as before in the "z64" format. To obtain this utility, go to the "Tools" folder and compile the Doom64 Extractor (instructions in dm64ex.cpp). Then use this command to extract the data of the rom "./dm64ex -i baserom.us.z64". Then copy the extracted data to "Data" folder, and change directory to the main doom64 folder awhere you can use the make command: "make PLATFORM=BB". The ROM will appear as "doom64.n64", and can be ran on the iQue.

## Notes
Erick194 created this project with CodeBlocks for the purposes of organization rather than compilation. Immorpher used Notepad++ to modify the code.

Interestingly, you can also use the WESSLIB.o from the original Mortal Kombat Trilogy on the Nintendo 64 as opposed to the rebuilt code here. For this go to the file "wessarc.h" and remove the slashes from the text "//#define NOUSEWESSCODE". Then go to the Makefile and remove the "#" from the following line "WESSLIB = #wesslib.o". Then add "#" in the following line "ASMFILES = wessint_s.s" to make it into "ASMFILES = #wessint_s.s". Now you can compile the Rom.

Erick194 gives special thanks to his brothers for help, the Doomworld community, and Kaiser for assistance and looking over the progress. Immorpher would like to thank Kaiser, the GEC team, and Nevander for their years in effort in the Doom community. Further, Immorpher would like to thank the Doom 64 discord community for assisting with this project.

GEC Team Discord:  https://discord.gg/aEZD4Y7
Doom 64 Discord: https://discord.gg/Ktxz8nz
