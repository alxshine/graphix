cmake_minimum_required(VERSION 3.2)
project(ex4 C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    source/DrawObject.cpp
    source/DrawObject.hpp
    source/List.c
    source/List.h
    source/LoadShader.c
    source/LoadShader.h
    source/OBJParser.c
    source/OBJParser.h
    source/StringExtra.c
    source/StringExtra.h
    source/LoadTexture.c
    source/LoadTexture.h
    Lighting.cpp)

add_executable(ex4 ${SOURCE_FILES})
target_compile_features(ex4 PRIVATE cxx_range_for)
target_link_libraries(ex4 "-lm -lglut -lGLEW -lGL")