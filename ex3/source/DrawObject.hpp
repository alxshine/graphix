#ifndef dObject
#define dObject

//include GL stuff
#include <GL/glew.h>

//include GLM stuff
#define GLM_FORCE_RADIANS

#include <glm/gtc/type_ptr.hpp>

//include local stuff
#include "OBJParser.h"


class DrawObject {
private:
    void setupDataBuffers();

public:
    GLuint vbo, ibo;
    glm::mat4 InitialTransform, DispositionMatrix;

    GLfloat *vertices;
    GLushort *indices;
    int v_size, i_size;

    enum DataID {
        vPosition = 0, vColor = 1
    };

    DrawObject(obj_scene_data *data);

    DrawObject(GLfloat *vertex_array, int vertex_size, GLushort *index_array, int index_size);

    ~DrawObject();

    void draw(GLuint ShaderProgram);
};

#endif
