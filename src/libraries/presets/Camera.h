#ifndef __CAMERA_H
#define __CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>
#include <GLFW/glfw3.h>

enum class CamMode
{
	trackball,
	pilot
};

class Camera
{
public:

    Camera(int width, int height, CamMode mode);
	~Camera();

	void update( GLFWwindow* window, glm::mat4 &viewmatrix);
	void update( GLFWwindow* window, glm::mat4 &viewmatrix, float deltatime);
	void setCenter( glm::vec3 *center);
	void setCameraPos(glm::vec3 pos);
	void setRadius( float radius);

	glm::vec3 getCameraPos();
	glm::mat4 getView();
	CamMode getCamMode();

private:

    CamMode m_camMode;
    glm::vec3 m_cameraPos, m_center, m_up, m_direction;

	float m_oldX, m_oldY;
	float m_sensitivity;
	float m_theta, m_phi, m_radius;
	float m_speed;

    int m_width, m_height;
    glm::mat4 m_viewmatrix;

};

#endif /* __CAMERA_H */
