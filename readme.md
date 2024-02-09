A clone of the 1972 arcade version of [Pong](https://en.wikipedia.org/wiki/Pong?&useskin=vector).

# Build
## Windows
### Requirements
- [MSYS2](https://code.visualstudio.com/docs/cpp/config-mingw#_installing-the-mingww64-toolchain): follow the 'Installing the MinGW-w64 toolchain' instructions.
- [Make](https://gnuwin32.sourceforge.net/packages/make.htm): after installation, add `\bin` to your path.
- [SDL2](https://github.com/libsdl-org/SDL/releases/latest): get SDL2-devel-x.x.x-mingw.
- [SDL_Mixer](https://github.com/libsdl-org/SDL_mixer/releases/latest): get SDL2_mixer-devel-x.x.x-mingw.

For both SDL2 and SDL_Mixer, extract the contents of `\x86_64-w64-mingw32\` into a directory of your choosing. Then copy the `.dll` files from `\bin` into the root of the cloned repository folder.

Open `makefile` and change the `SDL_path` variable to your SDL2 install directory.

Run `make release`.
