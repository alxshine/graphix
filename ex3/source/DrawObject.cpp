#include "DrawObject.hpp"
#include "OBJParser.h"

using namespace glm;

DrawObject::DrawObject(obj_scene_data *data, vec3 color) {
    v_size = data->vertex_count;
    i_size = data->face_count;
    n_size = data->vertex_normal_count;

    vertices = (GLfloat *) calloc((size_t) (v_size * 3), sizeof(GLfloat));
    colors = (GLfloat *) calloc((size_t) (v_size * 3), sizeof(GLfloat));
    normals = (GLfloat *) calloc((size_t) (n_size * 3), sizeof(GLfloat));
    indices = (GLushort *) calloc((size_t) (i_size * 3), sizeof(GLushort));

    //copy vertices
    for (int i = 0; i < v_size; i++) {
        vertices[i * 3] = (GLfloat) (*data->vertex_list[i]).e[0];
        vertices[i * 3 + 1] = (GLfloat) (*data->vertex_list[i]).e[1];
        vertices[i * 3 + 2] = (GLfloat) (*data->vertex_list[i]).e[2];
    }

    for (int i = 0; i < n_size; i++) {
        normals[i * 3] = (GLfloat) (*data->vertex_normal_list[i]).e[0];
        normals[i * 3 + 1] = (GLfloat) (*data->vertex_normal_list[i]).e[1];
        normals[i * 3 + 2] = (GLfloat) (*data->vertex_normal_list[i]).e[2];
    }

    //copy indices
    for (int i = 0; i < i_size; i++) {
        indices[i * 3] = (GLushort) (*data->face_list[i]).vertex_index[0];
        indices[i * 3 + 1] = (GLushort) (*data->face_list[i]).vertex_index[1];
        indices[i * 3 + 2] = (GLushort) (*data->face_list[i]).vertex_index[2];
    }

    //set up color buffer
    for (int i = 0; i < v_size; i++) {
        colors[i * 3] = color.r;
        colors[i * 3 + 1] = color.g;
        colors[i * 3 + 2] = color.b;
    }

    setupDataBuffers();
    InitialTransform = mat4(1);
    DispositionMatrix = mat4(1);
}

void DrawObject::setupDataBuffers() {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v_size * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, n_size * 3 * sizeof(GLfloat), normals, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, i_size * 3 * sizeof(GLushort), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, v_size * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
}

DrawObject::~DrawObject() {
    delete vertices;
    delete normals;
    delete indices;
    delete colors;
}

void DrawObject::draw(GLuint ShaderProgram) {
    glEnableVertexAttribArray(vPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vNormal);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vColor);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    GLint size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    GLint ModelMatrixID = glGetUniformLocation(ShaderProgram, "ModelMatrix");
    if (ModelMatrixID == -1) {
        fprintf(stderr, "Could not bind uniform ModelMatrix");
        exit(-1);
    }
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(DispositionMatrix * InitialTransform));

    glDrawElements(GL_TRIANGLES, (GLsizei) (size / sizeof(GLushort)), GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}
