/* standard includes */
#include <stdio.h>
#include <stdlib.>
#include <string.h>
#include <math.h>

/* OpenGL includes */
#include <GL/glew.h>
#include <GL/freeglut.h>

/* local includes */
#include "LoadShader.h"
#include "Matrix.h"

/*---------------------------------------------------------*/

/*vertex buffer handle*/
GLuint VBO;

/*color buffer object*/
GLuint CBO;

/*index buffer object*/
GLuint IBO;

/* Indices to vertex attributes; in this case positon and color */
enum DataID {vPosition = 0, vColor = 1};

/* Strings for loading and storing shader code */
static const char* VertexShaderString;
static const char* FragmentShaderString;

GLuint ShaderProgram;


