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
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```
As any other CMake project
```
mkdir build && cd build
cmake ..
make
make install
```
