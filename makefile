version = -std=c++23
warnings = -Wall -Wextra -Wconversion -Wsign-conversion
SDL_path = C:/cpplibraries/SDL2
name = pong-clone

debug:
	g++ $(version) $(warnings) src/*.cpp -o$(name).exe -Iinclude/ -I$(SDL_path)/include -L$(SDL_path)/lib -lmingw32 -lSDL2main -lSDL2

release:
	g++ $(version) -O3 -mwindows src/*.cpp -o$(name).exe -Iinclude/ -I$(SDL_path)/include -L$(SDL_path)/lib -lmingw32 -lSDL2main -lSDL2