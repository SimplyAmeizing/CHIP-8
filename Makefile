all:
	g++ -ISDL2/include -LSDL2/lib -o main main.cpp chip8.cpp -lmingw32 -lSDL2main -lSDL2