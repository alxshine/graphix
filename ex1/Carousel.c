/******************************************************************
*
* RotatingCube.c
*
* Description: This example demonstrates a colored, rotating
* cube in shader-based OpenGL. The use of transformation
* matrices, perspective projection, and indexed triangle sets 
* are shown.
*
* Computer Graphics Proseminar SS 2015
* 
* Interactive Graphics and Simulation Group
* Institute of Computer Science
* University of Innsbruck
*
*******************************************************************/


/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* OpenGL includes */
#include <GL/glew.h>
#include <GL/freeglut.h>


/* Local includes */
#include "LoadShader.h"   /* Provides loading function for shader code */
#include "Matrix.h"  

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

/*----------------------------------------------------------------*/

/* Define handle to a vertex buffer object */
GLuint vboCarousel, vboElements;

/* Define handle to a color buffer object */
GLuint cboCarousel, cboElements; 

/* Define handle to an index buffer object */
GLuint iboCarousel, iboElementsUp, iboElementsDown;


/* Indices to vertex attributes; in this case positon and color */ 
enum DataID {vPosition = 0, vColor = 1}; 

/* Strings for loading and storing shader code */
static const char* VertexShaderString;
static const char* FragmentShaderString;

GLuint ShaderProgram;

float ProjectionMatrix[16]; /* Perspective projection matrix */
float ViewMatrix[16]; /* Camera view matrix */ 
float ModelMatrix[16]; /* Model matrix */ 
float zMatrixUp[16], zMatrixDown[16];

/* Transformation matrices for initial position */
float TranslateOrigin[16];
float TranslateDown[16];
float TranslateZ[16];
float InitialTransform[16];

int offset;


//18 vertices per octagon times 2 octagons times 3 coordinates
GLfloat verticesCarousel[18*2*3];

//handle small objects seperately
GLfloat verticesElements[18*4*3];

GLfloat colorCarousel[18*2*3]; /* RGB color values for all vertices */
GLfloat colorElements[18*4*3];

//32 triangles for 2 octagons with 3 coordinates each
GLushort indicesCarousel[32*2*3];

GLushort indicesElements[32*4*3];

/*----------------------------------------------------------------*/


/******************************************************************
*
* Display
*
* This function is called when the content of the window needs to be
* drawn/redrawn. It has been specified through 'glutDisplayFunc()';
* Enable vertex attributes, create binding between C program and 
* attribute name in shader
*
*******************************************************************/

void Display()
{
	/* Clear window; color specified in 'Initialize()' */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(vPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboCarousel);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(vColor);
	glBindBuffer(GL_ARRAY_BUFFER, cboCarousel);
	glVertexAttribPointer(vColor, 3, GL_FLOAT,GL_FALSE, 0, 0);   

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCarousel);
	GLint size; 
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	/* Associate program with shader matrices */
	GLint projectionUniform = glGetUniformLocation(ShaderProgram, "ProjectionMatrix");
	if (projectionUniform == -1) 
	{
		fprintf(stderr, "Could not bind uniform ProjectionMatrix\n");
	exit(-1);
	}
	glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, ProjectionMatrix);
	
	GLint ViewUniform = glGetUniformLocation(ShaderProgram, "ViewMatrix");
	if (ViewUniform == -1) 
	{
		fprintf(stderr, "Could not bind uniform ViewMatrix\n");
		exit(-1);
	}
	glUniformMatrix4fv(ViewUniform, 1, GL_TRUE, ViewMatrix);
   
	GLint RotationUniform = glGetUniformLocation(ShaderProgram, "ModelMatrix");
	if (RotationUniform == -1) 
	{
		fprintf(stderr, "Could not bind uniform ModelMatrix\n");
		exit(-1);
	}
	glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, ModelMatrix);  

	/* Issue draw command, using indexed triangle list */
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

	//draw small elements
	glBindBuffer(GL_ARRAY_BUFFER, vboElements);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, cboElements);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	//going up
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboElementsUp);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	
	float ModelUp[16];
	MultiplyMatrix(ModelMatrix, zMatrixUp, ModelUp);
	glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, ModelUp);
	
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	
	//going down
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboElementsDown);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	
	float ModelDown[16];
	MultiplyMatrix(ModelMatrix, zMatrixDown, ModelDown);
	glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, ModelDown);
	
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	
	/* Disable attributes */
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);   
	

	/* Swap between front and back buffer */ 
	glutSwapBuffers();
}


/******************************************************************
*
* OnIdle
*
* 
*
*******************************************************************/

void OnIdle()
{
	float angle = (-glutGet(GLUT_ELAPSED_TIME) / 1000.0) * (180.0/M_PI); 
	float RotationMatrixAnim[16];

	/* Time dependent rotation */
	SetRotationY(angle, RotationMatrixAnim);
		
	/* Apply model rotation; finally move cube down */
	MultiplyMatrix(RotationMatrixAnim, InitialTransform, ModelMatrix);
		
	MultiplyMatrix(TranslateDown, ModelMatrix, ModelMatrix);
	
	float zMotion = sinf(glutGet(GLUT_ELAPSED_TIME)/1000.);
	SetTranslation(0, 0, -zMotion, &zMatrixDown);
	SetTranslation(0, 0, zMotion, &zMatrixUp);

	/* Request redrawing forof window content */  
	glutPostRedisplay();
}


/******************************************************************
*
* SetupDataBuffers
*
* Create buffer objects and load data into buffers
*
*******************************************************************/

void SetupDataBuffers()
{
	glGenBuffers(1, &vboCarousel);
	glBindBuffer(GL_ARRAY_BUFFER, vboCarousel);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCarousel), verticesCarousel, GL_STATIC_DRAW);
	
	glGenBuffers(1, &vboElements);
	glBindBuffer(GL_ARRAY_BUFFER, vboElements);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesElements), verticesElements, GL_STATIC_DRAW);

	glGenBuffers(1, &iboCarousel);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCarousel);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCarousel), indicesCarousel, GL_STATIC_DRAW);
	
	glGenBuffers(1, &iboElementsUp);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboElementsUp);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesElements)/2, indicesElements, GL_STATIC_DRAW);
	
	glGenBuffers(1, &iboElementsDown);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboElementsDown);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesElements)/2, 
		indicesElements+(sizeof(indicesElements)/sizeof(GLshort)/2), GL_STATIC_DRAW);

	glGenBuffers(1, &cboCarousel);
	glBindBuffer(GL_ARRAY_BUFFER, cboCarousel);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorCarousel), colorCarousel, GL_STATIC_DRAW);
	
	glGenBuffers(1, &cboElements);
	glBindBuffer(GL_ARRAY_BUFFER, cboElements);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorElements), colorElements, GL_STATIC_DRAW);
}


/******************************************************************
*
* AddShader
*
* This function creates and adds individual shaders
*
*******************************************************************/

void AddShader(GLuint ShaderProgram, const char* ShaderCode, GLenum ShaderType)
{
	/* Create shader object */
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) 
	{
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	/* Associate shader source code string with shader object */
	glShaderSource(ShaderObj, 1, &ShaderCode, NULL);

	GLint success = 0;
	GLchar InfoLog[1024];

	/* Compile shader source code */
	glCompileShader(ShaderObj);
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

	if (!success) 
	{
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	/* Associate shader with shader program */
	glAttachShader(ShaderProgram, ShaderObj);
}


/******************************************************************
*
* CreateShaderProgram
*
* This function creates the shader program; vertex and fragment
* shaders are loaded and linked into program; final shader program
* is put into the rendering pipeline 
*
*******************************************************************/

void CreateShaderProgram()
{
	/* Allocate shader object */
	ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) 
	{
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	/* Load shader code from file */
	VertexShaderString = LoadShader("vertexshader.vs");
	FragmentShaderString = LoadShader("fragmentshader.fs");

	/* Separately add vertex and fragment shader to program */
	AddShader(ShaderProgram, VertexShaderString, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, FragmentShaderString, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024];

	/* Link shader code into executable shader program */
	glLinkProgram(ShaderProgram);

	/* Check results of linking step */
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) 
	{
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	/* Check if shader program can be executed */ 
	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);

	if (!Success) 
	{
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	/* Put linked shader program into drawing pipeline */
	glUseProgram(ShaderProgram);
}


/******************************************************************
*
* Initialize
*
* This function is called to initialize rendering elements, setup
* vertex buffer objects, and to setup the vertex and fragment shader
*
*******************************************************************/

void Initialize(void)
{   
	/* Set background (clear) color to dark blue */ 
	glClearColor(0.2, 0.2, 0.2, 0.0);
	
	for(int i = 0; i<sizeof(colorCarousel)/sizeof(GLfloat); i+=3){
		colorCarousel[i] = 0.6;
		colorCarousel[i+1]= 0.3;
		colorCarousel[i+2] = 0.1;
	}
		
	for(int i = 0; i<sizeof(colorElements)/sizeof(GLfloat); i+=3){
		colorElements[i] = 1.0;
		colorElements[i+1] = 1.0;
		colorElements[i+2] = 0;
	}

	/* Enable depth testing */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);	

	//initialize all the vertices (octagons will be drawn umbrella style)
	for(int i = 0; i<4; i++){
		GLfloat z = -2;
		switch(i){
			case 0:
				z = -4;
				break;
			case 1:
				z = -3.5;
				break;
			case 2:
				z = 3.5;
				break;
			case 3:
				z = 4;
				break;	
		}

		GLfloat mainVertices[] = {
			0, 0, z,
			-2, 4, z,
			2, 4, z,
			4, 2, z,
			4, -2, z,
			2, -4, z,
			-2, -4, z,
			-4, -2, z,
			-4, 2, z
		};

		memcpy(verticesCarousel+i*27, mainVertices, sizeof(mainVertices));
	}

	//offset is for the two top and bottom octagons with 18 vertices each 
	offset = 2*18*3;
	for(int i = 0; i<4; i++){
		GLfloat x, y, z;
		
		switch(i){
			case 0:
				x = -1.75;
				y = 1.75;
				break;
			case 1:
				x = 1.75;
				y = -1.75;
				break;
			case 2:
				x = -1.75;
				y = -1.75;
				break;
			case 3:
				x = 1.75;
				y = 1.75;
		}
		
		z = 2;
		
		GLfloat step = 0.25;
		GLfloat currentVertices[] = {
			//vertices for one of the small objects
			x, y, -1,
			x-step, y+3*step, -z,
			x+step, y+3*step, -z,
			x+3*step, y+step, -z,
			x+3*step, y-step, -z,
			x+step, y-3*step, -z,
			x-step, y-3*step, -z,
			x-3*step, y-step, -z,
			x-3*step, y+step, -z,
			x, y, z,
			x-step, y+3*step, z,
			x+step, y+3*step, z,
			x+3*step, y+step, z,
			x+3*step, y-step, z,
			x+step, y-3*step, z,
			x-step, y-3*step, z,
			x-3*step, y-step, z,
			x-3*step, y+step, z,
		};
		memcpy(verticesElements+i*18*3, currentVertices, sizeof(currentVertices));
	}

	//handle 18 vertices per octagon (3dimensional) for 2 octagons
	for(int c = 0; c<2; c++){
		int i = 18*c;
		int j = i+9;
		GLshort triangleIndices[] = {
			//bottom
			i, i+1, i+2,
			i, i+2, i+3,
			i, i+3, i+4,
			i, i+4, i+5,
			i, i+5, i+6,
			i, i+6, i+7,
			i, i+7, i+8,
			i, i+8, i+1,
			//sides
			i+1, i+2, i+10,
			i+10, i+11, i+2,
			i+2, i+3, i+11,
			i+11, i+12, i+3,
			i+3, i+4, i+12,
			i+12, i+13, i+4,
			i+4, i+5, i+13,
			i+13, i+14, i+5,
			i+5, i+6, i+14,
			i+14, i+15, i+6,
			i+6, i+7, i+15,
			i+15, i+16, i+7,
			i+7, i+8, i+16,
			i+16, i+17, i+8,
			i+8, i+1, i+17,
			i+17, i+10, i+1,
			//top
			j, j+1, j+2,
			j, j+2, j+3,
			j, j+3, j+4,
			j, j+4, j+5,
			j, j+5, j+6,
			j, j+6, j+7,
			j, j+7, j+8,
			j, j+8, j+1
		};
		memcpy(indicesCarousel+c*32*3, triangleIndices, sizeof(triangleIndices));
	}
	
	//handle 18 vertices per octagon (3dimensional) for 4 elements
	for(int c = 0; c<4; c++){
		int i = 18*c;
		int j = i+9;
		GLshort triangleIndices[] = {
			//bottom
			i, i+1, i+2,
			i, i+2, i+3,
			i, i+3, i+4,
			i, i+4, i+5,
			i, i+5, i+6,
			i, i+6, i+7,
			i, i+7, i+8,
			i, i+8, i+1,
			//sides
			i+1, i+2, i+10,
			i+10, i+11, i+2,
			i+2, i+3, i+11,
			i+11, i+12, i+3,
			i+3, i+4, i+12,
			i+12, i+13, i+4,
			i+4, i+5, i+13,
			i+13, i+14, i+5,
			i+5, i+6, i+14,
			i+14, i+15, i+6,
			i+6, i+7, i+15,
			i+15, i+16, i+7,
			i+7, i+8, i+16,
			i+16, i+17, i+8,
			i+8, i+1, i+17,
			i+17, i+10, i+1,
			//top
			j, j+1, j+2,
			j, j+2, j+3,
			j, j+3, j+4,
			j, j+4, j+5,
			j, j+5, j+6,
			j, j+6, j+7,
			j, j+7, j+8,
			j, j+8, j+1
		};
		memcpy(indicesElements+c*32*3, triangleIndices, sizeof(triangleIndices));
	}
		
	/* Setup vertex, color, and index buffer objects */
	SetupDataBuffers();

	/* Setup shaders and shader program */
	CreateShaderProgram();  

	/* Initialize matrices */
	SetIdentityMatrix(ProjectionMatrix);
	SetIdentityMatrix(ViewMatrix);
	SetIdentityMatrix(ModelMatrix);

	/* Set projection transform */
	float fovy = 45.0;
	float aspect = 1.0; 
	float nearPlane = 1.0; 
	float farPlane = 50.0;
	SetPerspectiveMatrix(fovy, aspect, nearPlane, farPlane, ProjectionMatrix);

	/* Set viewing transform */
	float camera_disp = -20.0;
	SetTranslation(0.0, 0.0, camera_disp, ViewMatrix);

	/* Translate down */	
	SetTranslation(0, -sqrtf(sqrtf(2.0) * 1.0), 0, TranslateDown);

	/* Initial transformation matrix */
	SetRotationX(-90, InitialTransform);
	
	SetIdentityMatrix(zMatrixDown);
	SetIdentityMatrix(zMatrixUp);
}


/******************************************************************
*
* main
*
* Main function to setup GLUT, GLEW, and enter rendering loop
*
*******************************************************************/

int main(int argc, char** argv)
{
	/* Initialize GLUT; set double buffered window and RGBA color model */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(400, 400);
	glutCreateWindow("CG Proseminar - Rotating Cube");

	/* Initialize GL extension wrangler */
	GLenum res = glewInit();
	if (res != GLEW_OK) 
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	/* Setup scene and rendering parameters */
	Initialize();

	/* Specify callback functions;enter GLUT event processing loop, 
	 * handing control over to GLUT */
	glutIdleFunc(OnIdle);
	glutDisplayFunc(Display);
	glutMainLoop();


	/* ISO C requires main to return int */
	return 0;
}
