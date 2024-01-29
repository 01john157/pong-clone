SDLpath = C:/cpplibraries/SDL2

all:
	g++ -std=c++23 -Wall -Wextra -Weffc++ -Wconversion -Wsign-conversion -O3 src/*.cpp -omain.exe -Iinclude/ -I$(SDLpath)/include -L$(SDLpath)/lib -lmingw32 -lSDL2main -lSDL2