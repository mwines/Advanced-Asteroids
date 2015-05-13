CFLAGS = -I ./include
LIB    = ./lib/fmod/libfmodex64.so ./libggfonts.so
LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm -g #-lXrandr
#LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr

all: asteroids

asteroids: asteroids.cpp fmod.c ppm.c
	g++ $(CFLAGS) asteroids.cpp ppm.c fmod.c -Wall -Wextra $(LFLAGS) -o asteroids

clean:
	rm -f asteroids 
	rm -f *.o

