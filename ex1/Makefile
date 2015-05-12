CC = gcc
OBJ = Carousel.o LoadShader.o Matrix.o
CFLAGS = -g -Wall -Wextra -std=c99

LDLIBS=-lm -lglut -lGLEW -lGL

Carousel: $(OBJ)
	 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

clean:
	rm -f *.o Carousel
.PHONY: all clean
