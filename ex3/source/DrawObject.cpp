#include "DrawObject.hpp"

using namespace glm;

DrawObject::DrawObject(const obj_scene_data *data, const vec4 material[]) {
    v_size = data->vertex_count;
    i_size = data->face_count;
    n_size = data->vertex_normal_count;

    vertices = (GLfloat *) calloc((size_t) (v_size * 3), sizeof(GLfloat));
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

    memcpy(Material, material, sizeof(Material));

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
}

DrawObject::~DrawObject() {
    delete vertices;
    delete normals;
    delete indices;
}

void DrawObject::draw(GLuint ShaderProgram) {
    bindBuffers();

    GLint size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    bindMatrices(ShaderProgram);
    bindVectors(ShaderProgram);

    glDrawElements(GL_TRIANGLES, (GLsizei) (size / sizeof(GLushort)), GL_UNSIGNED_SHORT, 0);

    unbindBuffers();
}

void DrawObject::bindBuffers() const {
    glEnableVertexAttribArray(vPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(vNormal);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void DrawObject::unbindBuffers() const {
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
}

void DrawObject::bindMatrices(GLuint ShaderProgram) const {
    GLint ModelMatrixID = glGetUniformLocation(ShaderProgram, "ModelMatrix");
    if (ModelMatrixID == -1) {
        fprintf(stderr, "Could not bind uniform ModelMatrix\n");
        exit(-1);
    }
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(DispositionMatrix * InitialTransform));
}

void DrawObject::bindVectors(GLuint ShaderProgram) {
    GLint AmbientID = glGetUniformLocation(ShaderProgram, "ambient");
    if(AmbientID == -1){
        fprintf(stderr, "Could not bind ambient vector\n");
        exit(-1);
    }
    glUniform4fv(AmbientID, 1, value_ptr(Material[0]));

    GLint DiffuseID = glGetUniformLocation(ShaderProgram, "diffuse");
    if(DiffuseID == -1){
        fprintf(stderr, "Could not bind diffuse vector\n");
        exit(-1);
    }
    glUniform4fv(DiffuseID, 1, value_ptr(Material[1]));

    GLint SpecularID = glGetUniformLocation(ShaderProgram, "specular");
    if(SpecularID == -1){
        fprintf(stderr, "Could not bind specular vector\n");
        exit(-1);
    }
    glUniform4fv(SpecularID, 1, value_ptr(Material[2]));
}
