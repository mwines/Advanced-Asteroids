CFLAGS = -I ./include
LIB    = ./libggfonts.so
#LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr
LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm -g #-lXrandr

all: asteroids


asteroids: asteroids.cpp ppm.c
	g++ $(CFLAGS) asteroids.cpp ppm.c -Wall -Wextra $(LFLAGS) -o asteroids ./libggfonts.so

clean:
	rm -f asteroids 
	rm -f *.o

