SDL_path = C:/cpplibraries/SDL2
exe_name = $(notdir $(CURDIR))

all:
	g++ -std=c++23 -Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion src/*.cpp -o$(exe_name) -Iinclude/ -I$(SDL_path)/include -L$(SDL_path)/lib -lmingw32 -lSDL2main -lSDL2