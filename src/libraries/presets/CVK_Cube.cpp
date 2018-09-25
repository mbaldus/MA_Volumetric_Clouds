#include "CVK_Cube.h"

Cube::Cube()
{
	create( 1.0f);
}

Cube::Cube(float size)
{
	create( size);
}

Cube::~Cube()
{
}

//A -1.0f,  1.0f,  1.0f
//B -1.0f, -1.0f,  1.0f
//C  1.0f, -1.0f,  1.0f
//D  1.0f,  1.0f,  1.0f
//E  1.0f,  1.0f, -1.0f
//F  1.0f, -1.0f, -1.0f
//G -1.0f, -1.0f, -1.0f
//H -1.0f,  1.0f, -1.0f

void Cube::create( float size)
{

	//A,  B,  C,  D, Front
	//H,  G,  B,  A, Left
	//D,  C,  F,  E, Right
	//H,  A,  D,  E, Top
	//B,  G,  F,  C, Bottom
	//E,  F,  G,  H, Back

	GLfloat vertices[] = { 
		-1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f
    	};
    
    GLfloat normals[] = {        
         0.0f,  0.0f,  1.0f,    0.0f,  0.0f,  1.0f,    0.0f,  0.0f,  1.0f,    0.0f,  0.0f,  1.0f,
        -1.0f,  0.0f,  0.0f,   -1.0f,  0.0f,  0.0f,   -1.0f,  0.0f,  0.0f,   -1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,    1.0f,  0.0f,  0.0f,    1.0f,  0.0f,  0.0f,    1.0f,  0.0f,  0.0f,
         0.0f,  1.0f,  0.0f,    0.0f,  1.0f,  0.0f,    0.0f,  1.0f,  0.0f,    0.0f,  1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,    0.0f, -1.0f,  0.0f,    0.0f, -1.0f,  0.0f,    0.0f, -1.0f,  0.0f,
		 0.0f,  0.0f, -1.0f,    0.0f,  0.0f, -1.0f,    0.0f,  0.0f, -1.0f,    0.0f,  0.0f, -1.0f
    	};        

    GLfloat texCoords[] = {
			0.0f,  1.0f,    0.0f,  0.0f,    1.0f,  0.0f,    1.0f,  1.0f,
			0.0f,  1.0f,    0.0f,  0.0f,    1.0f,  0.0f,    1.0f,  1.0f,
			0.0f,  1.0f,    0.0f,  0.0f,    1.0f,  0.0f,    1.0f,  1.0f,
			0.0f,  1.0f,    0.0f,  0.0f,    1.0f,  0.0f,    1.0f,  1.0f,
			0.0f,  1.0f,    0.0f,  0.0f,    1.0f,  0.0f,    1.0f,  1.0f,
			0.0f,  1.0f,    0.0f,  0.0f,    1.0f,  0.0f,    1.0f,  1.0f
	};

	GLfloat uvwCoords[] = {
			0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f ,0.0f,
			0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
			1.0f, 1.0f ,0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f ,0.0f,  1.0f, 1.0f, 1.0f,
			0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 1.0f,
	};

	m_points = 24;
	m_indices = 36;

	for(int i=0; i<m_points; i++)
	{
		m_vertices.push_back(glm::vec4( vertices[i*3] * size, vertices[i*3+1] * size, vertices[i*3+2] * size, 1.0f));
		m_normals.push_back(glm::vec3( normals[i*3], normals[i*3+1], normals[i*3+2]));
		m_uvs.push_back(glm::vec2( texCoords[i*2], texCoords[i*2+1]));
		m_uvw.push_back(glm::vec3( uvwCoords[i*3], uvwCoords[i*3+1], uvwCoords[i*3+2]));
	}

	for(int i=0; i<6; i++)
	{
		m_index.push_back( i*4+0);
		m_index.push_back( i*4+1);
		m_index.push_back( i*4+2);
		m_index.push_back( i*4+2);
		m_index.push_back( i*4+3);
		m_index.push_back( i*4+0);
	}

	createBuffers();
}

void Cube::setSize( float size)
{	
	
	m_vertices.clear();
	
	GLfloat vertices[] = { 
		-1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f
    	};
	
	for(int i=0; i<m_points; i++)
	{
		m_vertices.push_back(glm::vec4( vertices[i*3] * size, vertices[i*3+1] * size, vertices[i*3+2] * size, 1.0f));
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec4), &m_vertices[0], GL_STATIC_DRAW);
	
}
