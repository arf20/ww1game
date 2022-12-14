# ww1game
Generic WW1 game (?)
![image](https://user-images.githubusercontent.com/35542215/189532904-5564c8af-a8f2-49a3-9d2b-37e51142986f.png)

This is my first SDL2 2D "game".

Language will be, of course, C++ version 17, for \<filesystem>. Built with the CMake build system (sorry not sorry).

The game engine is going to be completely separate from the game content, just like Quake.
All the assets will be in individual PNG files, and the maps will be described by a character matrix driven text file format.

Dependencies will be just SDL2, SDL2_image, SDL2_ttf and SDL2_mixer. But you will need some kind of a working graphical backend, like OpenGL.

## Build
Install dependencies (debian example)
```
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```
And as any CMake project
```
mkdir build && cd build
cmake ..
make
```

## Run
```
./ww1game
```
In the future you might be able to install it

## Asset directory structure (example)
```
assets/
 - missing_texture.png           #* something to render in case a texture is missing so you know its missing, 32x32
 - missing_sound.ogg             #* something to play in case a sound is missing so you know its missing, 440Hz 1s
 - textures/                     #* textures
    - bullet.png                 #* bullet texture
    - terrain/                   #* terrain variants
       - summer/                 #  variant (naming convention is lowercase and '_' separated)
          - a_dirt.png           #  texture (first character is the texture id, make sure they are all unique like this) 32x32
          - b_uphill.png
          - c_downhill.png
          - t_trench             #* id 't' is special for marking a trench
    - factions/                  #* factions
       - german_empire/          #  faction (naming convention is lowercase and '_' separated)
          - flag.png             #* faction flag
          - rifleman/            #  character (lowercase)
             - properties.cfg    #* character properties, described below
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
    - backgrounds/               #* backgrounds folder
       - cambrei.png             #  background
 - campaigns/                    #* campaigns
    - western_front/             #  campaign (naming convention is lowercase and '_' separated, displays "Western Front") contains filenames N.map N starting at 0
       - 0.map                   #  map (according to the format specification below, includes a title like "Cambrei")
       - 1.map
 - fonts/                        #* font directory
    - default.ttf                #* default font
 - sounds/                       #* sound files
    - missing_sound.ogg          #* missing sound sound
    - music/                     #* music tracks
       - menu.ogg                #* track for the menu
       - factions/
          - british_empire/      #  music for specific faction
             - victory.ogg       #* to be played on victory
             - 0.ogg             #  X.ogg is to be played randomly during gameplay
    - sfx/                       #* sound effects
       - factions/               #* 
          - british_empire/      #  per faction (must exist in assets/factions/)
             - rifleman/         #  per character
                - fire.ogg       #* weapon fire sound
```
*Fixed, N = a number starting at 0, X = a string

## Map file format specification
```
Cambrei                                                         # map title
summer                                                          # terrain variation
cambrei                                                         # background name
german_empire                                                   # friendly faction
british_empire                                                  # enemy faction
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

## Character properties specification
```
6        # fire frame, at what frame the weapon is fired in the animation
10       # rounds per minute
66       # round damage
300      # muzzle velocity (pix/s)
0.1      # bullet spread (angular standard deviation)
60       # march speed (pix/s)
20       # range (in tiles)
100      # health
```

## Global configuration (future)
Will brobably be at /usr/local/ww1game/ or something, and the assets at /usr/local/ww1game/assets or idk
```
assets/  # asset directory path
200      # gravity (reserved for future use)
7        # animation framerate
```

