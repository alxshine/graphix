CC = g++
LD = g++

OBJ = Lighting.o DrawObject.o LoadShader.o StringExtra.o OBJParser.o List.o LoadTexture.o
TARGET = Lighting

CFLAGS = -g -Wall 
LDLIBS = -lm -lglut -lGLEW -lGL
INCLUDES = -Isource

SRC_DIR = source
BUILD_DIR = build
VPATH = source

# Rules
all: $(TARGET)

$(TARGET).o: $(TARGET).cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(BUILD_DIR)/DrawObject.o: DrawObject.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

clean:
	rm -f $(BUILD_DIR)/*.o *.o $(TARGET) 

.PHONY: clean

# Dependencies
$(TARGET): $(BUILD_DIR)/LoadShader.o $(BUILD_DIR)/StringExtra.o $(BUILD_DIR)/LoadTexture.o $(BUILD_DIR)/DrawObject.o $(BUILD_DIR)/OBJParser.o  $(BUILD_DIR)/List.o | $(BUILD_DIR)



