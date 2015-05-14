#ifndef dObject
#define dObject

//include GL stuff
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "OBJParser.h"

class DrawObject{		
	private:
		void setupDataBuffers();
	
	public:
		GLuint vbo, ibo;
		float InitialTransform[16];
		
		GLfloat *vertices;
		GLushort *indices;
		int v_size, i_size;
		
		enum DataID{vPosition = 0, vColor = 1};
		DrawObject(obj_scene_data* data);
		DrawObject(GLfloat* vertex_array, int vertex_size, GLushort* index_array, int index_size);
		~DrawObject();
		
		void bindDataBuffers();
};

#endif
