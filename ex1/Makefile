CC = gcc
OBJ = RotatingCube.o LoadShader.o Matrix.o
CFLAGS = -g -Wall -Wextra -std=c99

LDLIBS=-lm -lglut -lGLEW -lGL

RotatingCube: $(OBJ)
	 $(CC) -o $@ $^ $(CFLAGS) $(LDLIBS) && optirun ./$@

clean:
	rm -f *.o RotatingCube 
.PHONY: all clean
