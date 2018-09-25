#ifndef __CVK_GEOMETRY_H
#define __CVK_GEOMETRY_H

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>



class Geometry
{
public:
	Geometry();
	~Geometry();

	void createBuffers();
	virtual void render();
	void computeTangents();

	std::vector<glm::vec4>* getVertices(); 
	std::vector<glm::vec3>* getNormals(); 
	std::vector<glm::vec2>* getUVs();
	std::vector<glm::vec3>* getUVW();
	std::vector<unsigned int>* getIndex();
	std::vector<glm::vec3>* getTangents();

protected:
    int m_geotype;
	GLuint m_vao;
	GLuint m_vertexbuffer;
	GLuint m_normalbuffer;
	GLuint m_uvbuffer;
	GLuint m_uvwbuffer;
	GLuint m_indexlist;
	GLuint m_tangentbuffer;

	int m_points;
	int m_indices;

	std::vector<glm::vec4> m_vertices; // vertice positions
	std::vector<glm::vec3> m_normals; // normals
	std::vector<glm::vec2> m_uvs; // texture map
	std::vector<glm::vec3> m_uvw; // uvw map

	std::vector<unsigned int> m_index; //index map
	std::vector<glm::vec3> m_tangents;

};

#endif /* __CVK_GEOMETRY_H */

