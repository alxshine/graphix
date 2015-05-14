/******************************************************************
*
* Interaction.c  
*
* Description: This file demonstrates the loading of external 
* triangle meshes provided in OBJ format. In addition, user
* interaction via mouse and keyboard is employed.
*
* The loaded triangle mesh is draw in wireframe mode and rotated
* around a reference axis. The user can control the rotation axis
* via the mouse and start/stop/reset animation via the keyboard.
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
#include "LoadShader.h"    /* Loading function for shader code */
#include "Matrix.h"        /* Functions for matrix handling */
#include "OBJParser.h"     /* Loading function for triangle meshes in OBJ format */

#include "DrawObject.hpp"


/*----------------------------------------------------------------*/

/* Flag for starting/stopping animation */
GLboolean anim, zoomIn, zoomOut = GL_FALSE;

DrawObject* carousel, *ground = 0;
DrawObject *cups[4] = {0,0,0,0};

/* Indices to vertex attributes; in this case positon only */ 
enum DataID {vPosition = 0}; 

/* Strings for loading and storing shader code */
static const char* VertexShaderString;
static const char* FragmentShaderString;

GLuint ShaderProgram;


/* Matrices for uniform variables in vertex shader */
float ProjectionMatrix[16]; /* Perspective projection matrix */
float ViewMatrix[16];       /* Camera view matrix */ 
float ModelMatrix[16];      /* Model matrix */
float CupModel[4][16];
  
/* Transformation matrices for model rotation */
float RotationMatrixAnimX[16];
float RotationMatrixAnimY[16];
float RotationMatrixAnimZ[16];
float RotationMatrixAnim[16];

float TranslationMatrixUp[16];
float TranslationMatrixDown[16];
float ZoomMatrix[16];
float StdRotMat[16];

/* Variables for storing current rotation angles */
float angleX, angleY, angleZ, rotAngle = 0.0f;
float yMotion = 0;
float yPhase = 0;

float camera_disp = -20.0;

/* Indices to active rotation axes */
enum {Xpos=0, Ypos=1, Zpos=2,Xneg=3, Yneg=4, Zneg=5};
int axis = Ypos;

/* Indices to active triangle mesh */
enum {Model1=0, Model2=1};
int model = Model1; 

/* Structures for loading of OBJ data */
obj_scene_data data;

/* Reference time for animation */
int oldTime = 0;


/*----------------------------------------------------------------*/



/******************************************************************
*
* Display
*
* This function is called when the content of the window needs to be
* drawn/redrawn. It has been specified through 'glutDisplayFunc()';
* Enable vertex attributes, create binding between C program and 
* attribute name in shader, provide data for uniform variables
*
*******************************************************************/

void Display()
{
    /* Clear window; color specified in 'Initialize()' */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(vPosition);

    carousel->bindDataBuffers();

    GLint size; 
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    /* Associate program with uniform shader matrices */
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
    glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, carousel->InitialTransform);  

    /* Set state to only draw wireframe (no lighting used, yet) */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /* Issue draw command, using indexed triangle list */
    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);


	/* ground */
	ground->bindDataBuffers();

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, ground->InitialTransform);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	
	/* cups */
	for(int i = 0; i<4; i++){
		cups[i]->bindDataBuffers();
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

		glUniformMatrix4fv(RotationUniform, 1, GL_TRUE, CupModel[i]);
		glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	}

    /* Disable attributes */
    glDisableVertexAttribArray(vPosition);

    /* Swap between front and back buffer */ 
    glutSwapBuffers();
}


/******************************************************************
*
* Mouse
*
* Function is called on mouse button press; has been seta
* with glutMouseFunc(), x and y specify mouse coordinates,
* but are not used here.
*
*******************************************************************/

void Mouse(int button, int state, int x, int y) 
{
    if(state == GLUT_DOWN) 
    {
      /* Depending on button pressed, set rotation axis,
       * turn on animation */
        switch(button) 
	{
	    case GLUT_LEFT_BUTTON:    
	        axis = Xpos;
		break;

	    case GLUT_MIDDLE_BUTTON:  
  	        axis = Ypos;
	        break;
		
	    case GLUT_RIGHT_BUTTON: 
	        axis = Zpos;
		break;
	}
	anim = GL_TRUE;
    }
}


/******************************************************************
*
* Keyboard
*
* Function to be called on key press in window; set by
* glutKeyboardFunc(); x and y specify mouse position on keypress;
* not used in this example 
*
*******************************************************************/

/*
 * We use the w,a,s and d keys to rotate around the x, and y axies.
 * e and q rotate around z 
 */
void Keyboard(unsigned char key, int x, int y)   
{
    switch( key ) 
    {

	/* Reset initial rotation of object */
	case 'o':
		camera_disp = -20.0;
	    SetIdentityMatrix(RotationMatrixAnim);	    
	    angleX = angleY = angleZ = rotAngle = 0.0f;
	    break;
	    
	case 's':
		anim = GL_TRUE;
	    axis = Xpos;
		break;

	case 'd':
		anim = GL_TRUE;
  	    axis = Ypos;
	    break;
	    
	case 'w':
		anim = GL_TRUE;
	    axis = Xneg;
		break;

	case 'a':
		anim = GL_TRUE;
  	    axis = Yneg;
	    break;
	    
	case 'q':
		anim = GL_TRUE;
  	    axis = Zpos;
	    break;
	    
	case 'e':
		anim = GL_TRUE;
  	    axis = Zneg;
	    break;
	    
	case 'c': case 'C': 
	    delete carousel;
	    exit(0);    
		break;
	case '+':
		zoomIn = GL_TRUE;
		break;
	
	case '-':
		zoomOut = GL_TRUE;
		break;
    }

    glutPostRedisplay();
}

/**
 *  stops animation when key is released
 **/
void KeyUp(unsigned char key, int x, int y){
	anim = GL_FALSE;
	switch(key){
		case '+':
			zoomIn = GL_FALSE;
			break;
		case '-':
			zoomOut = GL_FALSE;
			break;
	}
}


/******************************************************************
*
* OnIdle
*
* Function executed when no other events are processed; set by
* call to glutIdleFunc(); holds code for animation  
*
*******************************************************************/

void OnIdle()
{
    /* Determine delta time between two frames to ensure constant animation */
    int newTime = glutGet(GLUT_ELAPSED_TIME);
    int delta = newTime - oldTime;
    oldTime = newTime;
    
    /* Carousel turning */
    rotAngle = fmod(delta/15.0, 360.0);  
	SetRotationY(rotAngle, StdRotMat);
	MultiplyMatrix(StdRotMat,carousel->InitialTransform,carousel->InitialTransform);
	
	/* cups moving up and down */
	yPhase = yPhase + delta/1000.0;
	yMotion = sinf(yPhase);
	SetTranslation(0.0,yMotion,0.0,TranslationMatrixUp);
	SetTranslation(0.0,-yMotion,0.0,TranslationMatrixDown);
	
	/* reset matrizies */
	SetIdentityMatrix(RotationMatrixAnimX);
	SetIdentityMatrix(RotationMatrixAnimY);
	SetIdentityMatrix(RotationMatrixAnimZ);

	/* handels rotation when key is pressed */
    if(anim)
    {
        if(axis == Xpos)
	{
  	    angleX = fmod(delta/20.0, 360.0);  
	    SetRotationX(angleX, RotationMatrixAnimX);
	}
	else if(axis == Ypos)
	{
	    angleY = fmod(delta/20.0, 360.0); 
	    SetRotationY(angleY, RotationMatrixAnimY);  
	}
	else if(axis == Zpos)
	{			
	    angleZ = fmod(delta/20.0, 360.0); 
	    SetRotationZ(angleZ, RotationMatrixAnimZ);
	}
	else if(axis == Xneg)
	{
	    angleX = fmod(-delta/20.0, 360.0); 
	    SetRotationX(angleX, RotationMatrixAnimX);  
	}
	else if(axis == Zneg)
	{			
	    angleZ = fmod(-delta/20.0, 360.0); 
	    SetRotationZ(angleZ, RotationMatrixAnimZ);
	}else if(axis == Yneg)
	{
	    angleY = fmod(-delta/20.0, 360.0); 
	    SetRotationY(angleY, RotationMatrixAnimY);  
	}
	   
    }
    
    /* Zooming with +, and - */
    if(zoomIn && camera_disp<-10){
		camera_disp += delta/20.0;
	}
	else if(zoomOut && camera_disp>-40){
		camera_disp -= delta/20.0;
	}
		SetTranslation(0.0, 0.0, camera_disp, ZoomMatrix);

    /* Transform Model Matrix */
    MultiplyMatrix(RotationMatrixAnimX, RotationMatrixAnim, RotationMatrixAnim);
    MultiplyMatrix(RotationMatrixAnimY, RotationMatrixAnim, RotationMatrixAnim);
    MultiplyMatrix(RotationMatrixAnimZ, RotationMatrixAnim, RotationMatrixAnim);
    MultiplyMatrix(ZoomMatrix,RotationMatrixAnim,ViewMatrix);
    
    for(int i = 0; i<4; i++){
		MultiplyMatrix(StdRotMat,cups[i]->InitialTransform,cups[i]->InitialTransform);
		if(i<2){
			MultiplyMatrix(TranslationMatrixUp,cups[i]->InitialTransform, CupModel[i]);
		}
		else{
			MultiplyMatrix(TranslationMatrixDown,cups[i]->InitialTransform, CupModel[i]);
		}
	}
    
    
    /* Issue display refresh */
    glutPostRedisplay();
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
    VertexShaderString = LoadShader("shaders/vertexshader.vs");
    FragmentShaderString = LoadShader("shaders/fragmentshader.fs");

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
* vertex buffer objects, and to setup the vertex and fragment shader;
* meshes are loaded from files in OBJ format; data is copied from
* structures into vertex and index arrays
*
*******************************************************************/

void Initialize()
{   
    int success;

    /* Load Objects */
    success = parse_obj_scene(&data, "models/carousel.obj");
    if(!success)
        printf("Could not load file. Exiting.\n");
	carousel = new DrawObject(&data);
	SetIdentityMatrix(carousel->InitialTransform);
	
    success = parse_obj_scene(&data, "models/ground.obj");
    if(!success)
        printf("Could not load file. Exiting.\n");
	ground = new DrawObject(&data);
	SetTranslation(0.0,-3.5,0.0,ground->InitialTransform);
	
    success = parse_obj_scene(&data, "models/cup.obj");
    if(!success)
        printf("Could not load file. Exiting.\n");
    
    for(int i = 0; i<4; i++){
		cups[i] = new DrawObject(&data);
	}
	
	SetTranslation(4.0,0.0,0.0,cups[0]->InitialTransform);
    SetTranslation(-4.0,0.0,0.0,cups[1]->InitialTransform);
    SetTranslation(0.0,0.0,4.0,cups[2]->InitialTransform);
    SetTranslation(0.0,0.0,-4.0,cups[3]->InitialTransform);
	
    /* Set background (clear) color to Black */ 
    glClearColor(0.0, 0.0, 0.0, 0.0);

    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);    

    /* Setup shaders and shader program */
    CreateShaderProgram();  

    /* Initialize matrices */
    SetIdentityMatrix(ProjectionMatrix);
    SetIdentityMatrix(ViewMatrix);
    SetIdentityMatrix(ModelMatrix);
    for(int i = 0; i<4; i++){
		SetIdentityMatrix(CupModel[i]);
	}

    /* Initialize animation matrices */
    SetIdentityMatrix(RotationMatrixAnimX);
    SetIdentityMatrix(RotationMatrixAnimY);
    SetIdentityMatrix(RotationMatrixAnimZ);
    SetIdentityMatrix(RotationMatrixAnim);
    SetIdentityMatrix(StdRotMat);
    
    /* Set projection transform */
    float fovy = 60.0;
    float aspect = 1.0; 
    float nearPlane = 1.0; 
    float farPlane = 50.0;
    SetPerspectiveMatrix(fovy, aspect, nearPlane, farPlane, ProjectionMatrix);

    /* Set viewing transform */
    SetTranslation(0.0, 0.0, camera_disp, ViewMatrix);
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
    glutCreateWindow("CG Proseminar - User Interaction");

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
    glutKeyboardFunc(Keyboard); 
    glutKeyboardUpFunc(KeyUp);
    glutMouseFunc(Mouse);  

    glutMainLoop();

    /* ISO C requires main to return int */
    return 0;
}
