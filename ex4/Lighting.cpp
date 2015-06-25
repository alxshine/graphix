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
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

/* Local includes */
extern "C" {
#include "source/LoadShader.h"    /* Loading function for shader code */
#include "source/OBJParser.h"     /* Loading function for triangle meshes in OBJ format */
#include "source/LoadTexture.h"
};

#include "source/DrawObject.hpp"

using namespace glm;

/*----------------------------------------------------------------*/


DrawObject *carousel = 0, *ground = 0, *back = 0;
DrawObject *cups[4] = {0, 0, 0, 0};
DrawObject *billboard = 0;

/* Strings for loading and storing shader code */
static const char *VertexShaderString;
static const char *FragmentShaderString;

GLuint ShaderProgram;

GLuint LightShader;


/* Matrices for uniform variables in vertex shader */
/* Perspective projection matrix */
mat4 ProjectionMatrix;

/* Camera view matrix */
mat4 ViewMatrix;

/* Transformation matrices for model rotation */
mat4 TranslationMatrixUp;
mat4 TranslationMatrixDown;
mat4 CarouselRotationMatrix;


mat4 light_view_matrix;
mat4 light_projection_matrix;

mat4 scale_bias_matrix =
        mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
             vec4(0.0f, 0.5f, 0.0f, 0.0f),
             vec4(0.0f, 0.0f, 0.5f, 0.0f),
             vec4(0.5f, 0.5f, 0.5f, 1.0f));

/* Variables for storing current rotation angles */
float rotAngle = 0.0f;
float yMotion = 0;
float yPhase = 0;

float cameraDispositionZ = 20.f;
float cameraDispositionY = 5.f;

vec3 lightPosition1 = vec3(5, 2, 5);
vec4 lightIntensity1 = vec4(1, 1, 1, 1);

vec4 initialLightPosition2 = vec4(2, 2, 5, 1);
mat4 lightMatrix2 = mat4(1);
vec4 lightIntensity2 = vec4(0, 0, 0, 1);
DrawObject *light2 = 0;

vec4 fogColor = vec4(0.5, 0.6, 0.6, 1);

//ambient diffuse and specular terms for turning them on and off;
float ambient = 1, diffuse = 1, specular = 1;


/* Structures for loading of OBJ data */
obj_scene_data data;

/* Texture */

TextureData *Texture;
GLuint TextureID;
GLint TextureUniform;

/* Reference time for animation */
int oldTime = 0;

GLuint depth_texture;
GLint depth_uniform;

GLsizei texture_size = 1024;

GLuint depth_fbo;


void DrawShadowMap(void) {
    light_view_matrix = glm::lookAt(lightPosition1, vec3(0.0f), vec3(0, 1, 0));
    light_projection_matrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 50.0f);
    glUseProgram(LightShader);

    GLint VP_matrix = glGetUniformLocation(LightShader, "VP_matrix");
    if (VP_matrix == -1) {
        fprintf(stderr, "Could not bind uniform MVP_matrix\n");
        exit(-1);
    }

    glUniformMatrix4fv(VP_matrix, 1, GL_FALSE, glm::value_ptr(
            light_projection_matrix * light_view_matrix));

    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glViewport(0, 0, texture_size, texture_size);
    glClearDepth(1.0f
    );

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    /* Draw objects */
    ground->draw2(LightShader);
    carousel->draw2(LightShader);
    for (int i = 0; i < 4; i++)
        cups[i]->draw2(LightShader);
    light2->draw2(LightShader);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 600, 600);

    glUseProgram(ShaderProgram);
}

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

    DrawShadowMap();

    /* Clear window; color specified in 'Initialize()' */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Get texture uniform handle from fragment shader */
    TextureUniform = glGetUniformLocation(ShaderProgram, "textureSampler");
    if (TextureUniform == -1) {
        fprintf(stderr, "Could not bind uniform textureSampler");
        exit(-1);
    }


    /* Get texture uniform handle from fragment shader */
    depth_uniform = glGetUniformLocation(ShaderProgram, "depth_texture");
    if (depth_uniform == -1) {
        fprintf(stderr, "Could not bind uniform depth_texture");
        exit(-1);
    }

    /* Set location of uniform sampler variable */
    glUniform1i(TextureUniform, 0);
    glUniform1i(depth_uniform, 1);

    /* Activate first (and only) texture unit */
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    /* Bind current texture  */
    glBindTexture(GL_TEXTURE_2D, TextureID);

    /* Activate second texture unit */
    glActiveTexture(GL_TEXTURE1);

    /* Bind current texture  */
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    GLint size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    /* Associate program with uniform shader matrices */
    GLint PVMatrixID = glGetUniformLocation(ShaderProgram, "ProjectionViewMatrix");
    if (PVMatrixID == -1) {
        fprintf(stderr, "Could not bind uniform ProjectionViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(PVMatrixID, 1, GL_FALSE, value_ptr(ProjectionMatrix * ViewMatrix));

    GLint ShaddowID = glGetUniformLocation(ShaderProgram, "shadow_matrix");
    if (ShaddowID == -1) {
        fprintf(stderr, "Could not bind uniform ShaddowID\n");
        exit(-1);
    }

    glUniformMatrix4fv(ShaddowID, 1, GL_FALSE,
                       glm::value_ptr(scale_bias_matrix * light_projection_matrix * light_view_matrix));

    /* Associate program with uniform shader matrices */
    /*
    GLint VMatrixID = glGetUniformLocation(ShaderProgram, "ViewMatrix");
    if (VMatrixID == -1) {
        fprintf(stderr, "Could not bind uniform ViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(VMatrixID, 1, GL_FALSE, value_ptr(ViewMatrix));
    */

    /* associate program with light */
    //light 1 (immobile, changable colors)
    GLint lP1ID = glGetUniformLocation(ShaderProgram, "lP1");
    if (lP1ID == -1) {
        fprintf(stderr, "Could not bind uniform lightPosition1 1\n");
        exit(-1);
    }
    glUniform3fv(lP1ID, 1, value_ptr(lightPosition1));
    GLint lI1ID = glGetUniformLocation(ShaderProgram, "lI1");
    if (lI1ID == -1) {
        fprintf(stderr, "Could not bind uniform lightIntensity 1\n");
        exit(-1);
    }
    glUniform4fv(lI1ID, 1, value_ptr(lightIntensity1));

    //light 2 (mobile, fixed color)
    vec4 currentLightPosition2 = lightMatrix2 * initialLightPosition2;
    vec3 lP2 = vec3(currentLightPosition2.x, currentLightPosition2.y, currentLightPosition2.z);
    GLint lP2ID = glGetUniformLocation(ShaderProgram, "lP2");
    if (lP2ID == -1) {
        fprintf(stderr, "Could not bind uniform lightPosition 2\n");
        exit(-1);
    }
    glUniform3fv(lP2ID, 1, value_ptr(lP2));

    GLint lI2ID = glGetUniformLocation(ShaderProgram, "lI2");
    if (lI2ID == -1) {
        fprintf(stderr, "Could not bind uniform lightIntenstity 2\n");
        exit(-1);
    }
    glUniform4fv(lI2ID, 1, value_ptr(lightIntensity2));

    //lighting components
    GLint ambientID = glGetUniformLocation(ShaderProgram, "showAmbient");
    if (ambientID == -1) {
        fprintf(stderr, "Could not bind uniform showAmbient\n");
        exit(-1);
    }
    glUniform1f(ambientID, ambient);

    GLint diffuseID = glGetUniformLocation(ShaderProgram, "showDiffuse");
    if (diffuseID == -1) {
        fprintf(stderr, "Could not bind uniform showDiffuse\n");
        exit(-1);
    }
    glUniform1f(diffuseID, diffuse);

    GLint specularID = glGetUniformLocation(ShaderProgram, "showSpecular");
    if (specularID == -1) {
        fprintf(stderr, "Could not bind uniform showSpecular\n");
        exit(-1);
    }
    glUniform1f(specularID, specular);


    /* Draw objects */
    ground->draw(ShaderProgram);
    carousel->draw(ShaderProgram);
    for (int i = 0; i < 4; i++)
        cups[i]->draw(ShaderProgram);
    light2->draw(ShaderProgram);

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
    switch (key) {
        case 'r':
        case '1':
            lightIntensity1[0] = !lightIntensity1[0];
            break;
        case 'g':
        case '2':
            lightIntensity1[1] = !lightIntensity1[1];
            break;
        case 'b':
        case '3':
            lightIntensity1[2] = !lightIntensity1[2];
            break;
        case 'c':
            exit(0);
        case 'a':
            ambient = !ambient;
            break;
        case 'd':
            diffuse = !diffuse;
            break;
        case 's':
            specular = !diffuse;
            break;
        default:
            break;
    }

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
//    rotAngle = 0.0;
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

    lightMatrix2 = CarouselRotationMatrix;
    light2->DispositionMatrix = CarouselRotationMatrix;


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

void CreateLightShader() {
    /* Allocate shader object */
    LightShader = glCreateProgram();

    if (LightShader == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    /* Load shader code from file */
    VertexShaderString = LoadShader("shaders/lightvertexshader.vs");
    FragmentShaderString = LoadShader("shaders/lightfragmentshader.fs");

    /* Separately add vertex and fragment shader to program */
    AddShader(LightShader, VertexShaderString, GL_VERTEX_SHADER);
    AddShader(LightShader, FragmentShaderString, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024];

    /* Link shader code into executable shader program */
    glLinkProgram(LightShader);

    /* Check results of linking step */
    glGetProgramiv(LightShader, GL_LINK_STATUS, &Success);

    if (Success == 0) {
        glGetProgramInfoLog(LightShader, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Check if shader program can be executed */
    glValidateProgram(LightShader);
    glGetProgramiv(LightShader, GL_VALIDATE_STATUS, &Success);

    if (!Success) {
        glGetProgramInfoLog(LightShader, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    /* Put linked shader program into drawing pipeline */
    glUseProgram(LightShader);
}

void SetupTexture() {
    /* Allocate texture container */
    Texture = (TextureData *) malloc(sizeof(TextureData *));

    int success = LoadTexture("data/uvtemplate.bmp", Texture);
    if (!success) {
        printf("Error loading texture. Exiting.\n");
        exit(-1);
    }

    /* Create texture name and store in handle */
    glGenTextures(1, &TextureID);

    /* Bind texture */
    glBindTexture(GL_TEXTURE_2D, TextureID);

    /* Load texture image into memory */
    glTexImage2D(GL_TEXTURE_2D,     /* Target texture */
                 0,                 /* Base level */
                 GL_RGB,            /* Each element is RGB triple */
                 Texture->width,    /* Texture dimensions */
                 Texture->height,
                 0,                 /* Border should be zero */
                 GL_BGR,            /* Data storage format for BMP file */
                 GL_UNSIGNED_BYTE,  /* Type of pixel data, one byte per channel */
                 Texture->data);    /* Pointer to image data  */

    /* Next set up texturing parameters */

    /* Repeat texture on edges when tiling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* Linear interpolation for magnification */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Trilinear MIP mapping for minification */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    /* Note: MIP mapping not visible due to fixed, i.e. static camera */
}

void SetupShadowMap(void)
{
    glGenTextures(1,&depth_texture);
    glBindTexture(GL_TEXTURE_2D,depth_texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,texture_size, texture_size,0,GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);


    glDrawBuffer(GL_NONE);
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
                        vec3(0, 1, 0));  /* Up vector */

    int success;

    /* define materials */
    vec4 carouselMaterial[3] = {vec4(0.4f, 0.1f, 0.65f, 1), vec4(0.4f, 0.1f, 0.65f, 1), vec4(1, 1, 1, 1)};
    vec4 groundMaterial[3] = {vec4(0.6f, 0.4f, 0.3f, 1), vec4(0.6f, 0.4f, 0.3f, 1), vec4(1, 1, 1, 1)};
    vec4 cupMaterial[3] = {vec4(0.4f, 0.5f, 0.1f, 1), vec4(0.4f, 0.5f, 0.1f, 1), vec4(1, 1, 1, 1)};

    /* Load Objects */
    success = parse_obj_scene(&data, (char *) "models/carousel.obj");
    if (!success)
        printf("Could not load file. Exiting.\n");
    carousel = new DrawObject(&data, carouselMaterial);

    success = parse_obj_scene(&data, (char *) "models/ground.obj");
    if (!success)
        printf("Could not load file. Exiting.\n");
    ground = new DrawObject(&data, groundMaterial);
    ground->InitialTransform = translate(mat4(1), vec3(0, -3.5f, 0));

    success = parse_obj_scene(&data, (char *) "models/capsule.obj");
    if (!success)
        printf("Could not load file. Exiting.\n");

    for (int i = 0; i < 4; i++) {
        cups[i] = new DrawObject(&data, cupMaterial);
    }

    cups[0]->InitialTransform = translate(mat4(1), vec3(4, 0, 0));
    cups[1]->InitialTransform = translate(mat4(1), vec3(-4, 0, 0));
    cups[2]->InitialTransform = translate(mat4(1), vec3(0, 0, 4));
    cups[3]->InitialTransform = translate(mat4(1), vec3(0, 0, -4));

    //set light visualization
    success = parse_obj_scene(&data, (char *) "models/sphere.obj");
    if (!success) {
        printf("Could not load file. Exiting\n");
        exit(-1);
    }

    vec4 lightMaterial[3] = {vec4(1, 1, 1, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 1)};
    light2 = new DrawObject(&data, lightMaterial);
    light2->InitialTransform = translate(mat4(1), vec3(initialLightPosition2));

    /* Set background (clear) color to Black */
    glClearColor(fogColor.r, fogColor.g, fogColor.b, fogColor.a);

    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Setup shaders and shader program */
    CreateLightShader();
    CreateShaderProgram();

    GLint cPID = glGetUniformLocation(ShaderProgram, "cP");
    if (cPID == -1) {
        fprintf(stderr, "Could not locate uniform CameraPosition");
        exit(-1);
    }
    glUniform4fv(cPID, 1, value_ptr(vec4(0, cameraDispositionY, cameraDispositionZ, 1)));

    GLint PVMatrixID = glGetUniformLocation(ShaderProgram, "ProjectionViewMatrix");
    if (PVMatrixID == -1) {
        fprintf(stderr, "Could not locate uniform ProjectionViewMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(PVMatrixID, 1, GL_FALSE, value_ptr(ProjectionMatrix * ViewMatrix));

    GLint fogColorID = glGetUniformLocation(ShaderProgram, "fogColor");
    if (fogColorID == -1) {
        fprintf(stderr, "Could not locate uniform FogColor\n");
        exit(-1);
    }
    glUniform4fv(fogColorID, 1, value_ptr(fogColor));

    /* set up texture */
    SetupShadowMap();
    SetupTexture();
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
    glutKeyboardFunc(Keyboard);
//    glutKeyboardUpFunc(KeyUp);

    glutMainLoop();

    /* ISO C requires main to return int */
    return 0;
}
