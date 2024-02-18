name = pong-clone
version = -std=c++23
warnings = -Wall -Wextra -Wconversion -Wsign-conversion
SDL_path = C:/cpplibraries/SDL2


debug:
	g++ $(version) $(warnings) src/*.cpp -o$(name).exe -I$(SDL_path)/include -L$(SDL_path)/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image

release:
	g++ $(version) -O3 -mwindows src/*.cpp -o$(name).exe -I$(SDL_path)/include -L$(SDL_path)/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lSDL2_image