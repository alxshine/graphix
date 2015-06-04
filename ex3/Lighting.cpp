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

#define GLM_FORCE_RADIANS

/* OpenGL includes */
#include <GL/glew.h>
#include <GL/freeglut.h>

/* GLM includes */
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* Local includes */
extern "C" {
#include "source/LoadShader.h"    /* Loading function for shader code */
#include "source/OBJParser.h"     /* Loading function for triangle meshes in OBJ format */
};

#include "source/DrawObject.hpp"

using namespace glm;

/*----------------------------------------------------------------*/


DrawObject *carousel, *ground = 0;
DrawObject *cups[4] = {0, 0, 0, 0};

/* Strings for loading and storing shader code */
static const char *VertexShaderString;
static const char *FragmentShaderString;

GLuint ShaderProgram;


/* Matrices for uniform variables in vertex shader */
/* Perspective projection matrix */
mat4 ProjectionMatrix;

/* Camera view matrix */
mat4 ViewMatrix;

/* Transformation matrices for model rotation */
mat4 TranslationMatrixUp;
mat4 TranslationMatrixDown;
mat4 CarouselRotationMatrix;

/* Variables for storing current rotation angles */
float rotAngle = 0.0f;
float yMotion = 0;
float yPhase = 0;

float cameraDispositionZ = 20.f;
float cameraDispositionY = 5.f;

vec3 LightPosition = vec3(5, 2, 5);

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

void Display() {
    /* Clear window; color specified in 'Initialize()' */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    /* Associate program with uniform shader matrices */
    GLint PVMatrixID = glGetUniformLocation(ShaderProgram, "ProjectionViewMatrix");
    if (PVMatrixID == -1) {
        fprintf(stderr, "Could not bind uniform ProjectionViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(PVMatrixID, 1, GL_FALSE, value_ptr(ProjectionMatrix * ViewMatrix));

    /* associate program with light */
    GLint LightID = glGetUniformLocation(ShaderProgram, "LightPosition");
    if (LightID == -1) {
        fprintf(stderr, "Could not bind uniform LightPosition\n");
        exit(-1);
    }
    glUniform3fv(LightID, 1, value_ptr(LightPosition));

    ground->draw(ShaderProgram);
    carousel->draw(ShaderProgram);
    for (int i = 0; i < 4; i++)
        cups[i]->draw(ShaderProgram);

    /* Swap between front and back buffer */
    glutSwapBuffers();
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
void Keyboard(unsigned char key, int x, int y) {
    glutPostRedisplay();
}

/**
 *  stops animation when key is released in mode 2
 **/
void KeyUp(unsigned char key, int x, int y) {
}


/******************************************************************
*
* OnIdle
*
* Function executed when no other events are processed; set by
* call to glutIdleFunc(); holds code for animation  
*
*******************************************************************/

void OnIdle() {
    /* Determine delta time between two frames to ensure constant animation */
    int newTime = glutGet(GLUT_ELAPSED_TIME);
    int delta = newTime - oldTime;
    oldTime = newTime;

    /* Carousel turning */
    rotAngle = delta / 1200.f;
    CarouselRotationMatrix = rotate(CarouselRotationMatrix, rotAngle, vec3(0, 1, 0));
    carousel->DispositionMatrix = CarouselRotationMatrix;

    /* cups moving up and down */
    yPhase = yPhase + delta / 1000.0f;
    yMotion = sinf(yPhase);
    TranslationMatrixUp = translate(mat4(1), vec3(0, yMotion, 0));
    TranslationMatrixDown = translate(mat4(1), vec3(0, -yMotion, 0));

    for (int i = 0; i < 4; i++) {
        if (i < 2) {
            cups[i]->DispositionMatrix = TranslationMatrixUp * CarouselRotationMatrix;
        }
        else {
            cups[i]->DispositionMatrix = TranslationMatrixDown * CarouselRotationMatrix;
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

void AddShader(GLuint ShaderProgram, const char *ShaderCode, GLenum ShaderType) {
    /* Create shader object */
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
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

    if (!success) {
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

void CreateShaderProgram() {
    /* Allocate shader object */
    ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
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

    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Check if shader program can be executed */
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);

    if (!Success) {
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

void Initialize() {
    /* Set projection transform */
    float fovy = (float) (45.0 * M_PI / 180.0);
    float aspect = 1.0;
    float nearPlane = 1.0;
    float farPlane = 50.0;
    ProjectionMatrix = perspective(fovy, aspect, nearPlane, farPlane);

    /* Set viewing transform */
    ViewMatrix = lookAt(vec3(0, cameraDispositionY, cameraDispositionZ),    /* Eye vector */
                        vec3(0, 0, 0),     /* Viewing center */
                        vec3(0, 1, -1));  /* Up vector */

    int success;

    /* define materials */
    vec4 carouselMaterial[3] = {vec4(0.4f, 0.1f, 0.65f, 1), vec4(0.4f, 0.1f, 0.65f, 1), vec4(1, 1, 1, 1)};
    vec4 groundMaterial[3] = {vec4(0.2f, 0.8f, 0.8f, 1), vec4(0.2f, 0.8f, 0.8f, 1), vec4(1, 1, 1, 1)};
    vec4 cupMaterial[3] = {vec4(0.4f, 0.5f, 0.1f, 1), vec4(0.4f, 0.5f, 0.1f, 1), vec4(1, 1, 1, 1)};

    /* Load Objects */
    success = parse_obj_scene(&data, "models/carousel.obj");
    if (!success)
        printf("Could not load file. Exiting.\n");
    carousel = new DrawObject(&data, carouselMaterial);

    success = parse_obj_scene(&data, "models/ground.obj");
    if (!success)
        printf("Could not load file. Exiting.\n");
    ground = new DrawObject(&data, cupMaterial);
    ground->InitialTransform = translate(mat4(1), vec3(0, -3.5f, 0));

    success = parse_obj_scene(&data, "models/cup.obj");
    if (!success)
        printf("Could not load file. Exiting.\n");

    for (int i = 0; i < 4; i++) {
        cups[i] = new DrawObject(&data, groundMaterial);
    }

    cups[0]->InitialTransform = translate(mat4(1), vec3(4, 0, 0));
    cups[1]->InitialTransform = translate(mat4(1), vec3(-4, 0, 0));
    cups[2]->InitialTransform = translate(mat4(1), vec3(0, 0, 4));
    cups[3]->InitialTransform = translate(mat4(1), vec3(0, 0, -4));

    /* Set background (clear) color to Black */
    glClearColor(0.0, 0.0, 0.0, 0.0);

    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Setup shaders and shader program */
    CreateShaderProgram();

//    GLint CameraPositionId = glGetUniformLocation(ShaderProgram, "CameraPosition");
//    if (CameraPositionId == -1) {
//        fprintf(stderr, "Could not locate uniform CameraPosition");
//        exit(-1);
//    }
//    glUniform4fv(CameraPositionId, 1, value_ptr(vec4(0, cameraDispositionY, cameraDispositionZ, 1)));

    GLint PVMatrixID = glGetUniformLocation(ShaderProgram, "ProjectionViewMatrix");
    if(PVMatrixID == -1){
        fprintf(stderr, "Could not locate uniform ProjectionViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(PVMatrixID, 1, GL_FALSE, value_ptr(ProjectionMatrix*ViewMatrix));
}


/******************************************************************
*
* main
*
* Main function to setup GLUT, GLEW, and enter rendering loop
*
*******************************************************************/

int main(int argc, char **argv) {
    /* Initialize GLUT; set double buffered window and RGBA color model */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(400, 400);
    glutCreateWindow("CG Proseminar - User Interaction");

    /* Initialize GL extension wrangler */
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    /* Setup scene and rendering parameters */
    Initialize();

    /* Specify callback functions;enter GLUT event processing loop, 
     * handing control over to GLUT */
    glutIdleFunc(OnIdle);
    glutDisplayFunc(Display);
//    glutKeyboardFunc(Keyboard);
//    glutKeyboardUpFunc(KeyUp);

    glutMainLoop();

    /* ISO C requires main to return int */
    return 0;
}
