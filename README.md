# ww1game
Generic WW1 game (?)

This is my first SDL2 2D "game".

Language will be, of course, C++ version 17, for \<filesystem>. Built with the CMake build system (sorry not sorry).

The game engine is going to be completely separate from the game content, just like Quake.
All the assets will be in individual PNG files, and the maps will be described by a character matrix driven text file format.

Dependencies will be SDL2, SDL2_image and probably GL.

## Build
Install dependencies (debian example)
```
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```
As any other CMake project
```
mkdir build && cd build
cmake ..
make
make install
```

## Asset directory structure (example)
```
assets/
 - missing_texture.png        #* something to render in case a texture is missing so you know its missing, 32x32
 - terrain/                   #* terrain variants
    - summer/                 #  variant (naming convention is lowercase and '_' separated)
       - a_dirt.png           #  texture (first character is the texture id, make sure they are all unique like this) 32x32
       - b_uphill.png
       - c_downhill.png
       - t_trench             #* id 't' is special for marking a trench
 - campaigns/                 #* campaigns
    - western_front/          #  campaign (naming convention is lowercase and '_' separated, displays "Western Front") contains filenames N.map N starting at 0
       - 0.map                #  map (according to the format specification below, includes a title like "Cambrei")
       - 1.map
 - factions/                  #* factions
    - german_empire/          #  faction
       - rifleman/            #  character
          - idle.png          #* idle texture
          - walk/             #* walking animation, contains N.png N starting at 0
             - 0.png          #  animation frame texture
             - 1.png
             - 2.png
             - 3.png
          - fire/             #* firing animation
             - 0.png
             - 1.png
             - 2.png
          - death/            #* death animation
             - 0.png
             - 1.png
 - fonts/                     #* font directory
    - default.ttf             #* default font
 - sounds/                    #* sound files
    - missing_sound.ogg       #* missing sound sound
    - music/                  #* music tracks
       - menu.ogg             #* track for the menu
       - factions/
          - british_empire/   #  music for specific faction
             - victory.ogg    #* to be played on victory
             - 0.ogg          #  N.ogg N starting at 0 is to be played randomly during gameplay
    - sfx/                    #* sound effects
       - factions/            #* 
          - british_empire/   #  per faction (must exist in assets/factions/)
             - rifleman/      #  per character
                - fire.ogg    #* weapon fire sound
```
*Fixed

## Map file format specification
```
Cambrei                                                         # map title
summer                                                          # terrain variation
      cbbbe                                                     # map matrix
     cdaaafe                                    cbbe     
    cdaaaaafe                                  cdaafe    
bbbbdaaaaaaafbbbbghibbbbbghibbbbbbbbbbbbbghibbbdaaaafbbbb
aaaaaaaaaaaaaaaaaajaaaaaaajaaaaaaaaaaaaaaajaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
```
The first line is the map title, and the second is the terrain variation. 
Then the map is described by a matrix of characters which correspond with the first letter of the terrain texture filename.

On map load, a path through the map is found, in which soldiers travel by their bottom center point.

## Todo
```
Soldiers
 - Animation
 - Movement
Scroll
```
