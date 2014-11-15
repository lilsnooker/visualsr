all:
	gcc -std=c99 main.c -o visualizer -O3 -lpulse -lm -lfftw3 -lGLEW -lGLU -lGL -lSDL2

run: all
	./visualizer