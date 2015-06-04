#ifndef dObject
#define dObject

//include GL stuff
#include <GL/glew.h>

//include GLM stuff
#define GLM_FORCE_RADIANS

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

//include local stuff
#include "OBJParser.h"

using namespace glm;

class DrawObject {
private:
    vec4 Material[3];

    void setupDataBuffers();
    void bindBuffers() const;
    void bindMatrices(GLuint ShaderProgram) const;

public:
    GLuint vbo, nbo, ibo, mbo;

    GLfloat *vertices, *normals;
    GLushort *indices;
    int v_size, i_size, n_size;
    mat4 InitialTransform, DispositionMatrix;

    enum DataID {
        vPosition = 0, vNormal = 2
    };

    DrawObject(const obj_scene_data *data, const vec4 Material[]);
    ~DrawObject();

    void draw(GLuint ShaderProgram);

    void unbindBuffers() const;

    void bindVectors(GLuint ShaderProgram);
};

#endif
