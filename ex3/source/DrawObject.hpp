#ifndef dObject
#define dObject

//include GL stuff
#include <GL/glew.h>

//include GLM stuff
#define GLM_FORCE_RADIANS

#include <glm/gtc/type_ptr.hpp>

//include local stuff
#include "OBJParser.h"

using namespace glm;

class DrawObject {
private:
    void setupDataBuffers();

public:
    GLuint vbo, nbo, ibo, cbo;
    mat4 InitialTransform, DispositionMatrix;

    GLfloat *vertices, *colors, *normals;
    GLushort *indices;
    int v_size, i_size, n_size;

    enum DataID {
        vPosition = 0, vColor = 1, vNormal = 2
    };

    DrawObject(obj_scene_data *data, glm::vec3 color);

    ~DrawObject();

    void draw(GLuint ShaderProgram);
};

#endif
