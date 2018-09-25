#include "Camera.h"


Camera::Camera(int width, int height, CamMode mode) : m_width(width), m_height(height),m_camMode(mode)
{
    m_cameraPos = glm::vec3(0.0f,0.0f, 5.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
    m_center = glm::vec3(0.0f, 0.0f, 0.0f);

    m_theta = glm::pi<float>() / 2.0f;
    m_oldX = width / 2.f;
    m_oldY = height / 2.f;

    if (mode == CamMode::trackball)
    {
        m_phi = 0.0f;
        m_radius = 1.5;
        m_sensitivity = 0.005f;
        m_viewmatrix = glm::lookAt(m_center + m_cameraPos, m_center, m_up);
    }
    if (mode == CamMode::pilot)
    {
        m_phi = glm::pi<float>();
        m_speed = 0.0f;
        m_sensitivity = 0.005f;
        m_viewmatrix = glm::lookAt(m_cameraPos, m_cameraPos + m_direction, m_up);
//        m_projectionMat = glm::perspective(45.f, (m_width / float(m_height)), 0.1f, 100.f);
    }


}

Camera::~Camera()
{

}

void Camera::update(GLFWwindow* window, glm::mat4 &viewmatrix)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        float changeX = ((float) x - m_oldX) * m_sensitivity;
        float changeY = ((float) y - m_oldY) * m_sensitivity;

        m_theta -= changeY;
        if (m_theta < 0.01f) m_theta = 0.01f;
        else if (m_theta > glm::pi<float>() - 0.01f) m_theta = glm::pi<float>() - 0.01f;

        m_phi -= changeX;
        if (m_phi < 0) m_phi += 2 * glm::pi<float>();
        else if (m_phi > 2 * glm::pi<float>()) m_phi -= 2 * glm::pi<float>();
    }

    m_oldX = (float) x;
    m_oldY = (float) y;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_radius -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_radius += 0.1f;
    if (m_radius < 0.1f) m_radius = 0.1f;

    m_cameraPos.x = m_center.x + m_radius * sin(m_theta) * sin(m_phi);
    m_cameraPos.y = m_center.y + m_radius * cos(m_theta);
    m_cameraPos.z = m_center.z + m_radius * sin(m_theta) * cos(m_phi);

    viewmatrix = glm::lookAt(m_cameraPos, m_center, m_up);
    m_viewmatrix = viewmatrix;
}

void Camera::update(GLFWwindow* window, glm::mat4 &viewmatrix, float deltaTime)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    y = m_height - y;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        float changeX = ((float) x - m_oldX) * m_sensitivity;
        float changeY = ((float) y - m_oldY) * m_sensitivity;

        m_theta -= changeY;
        if (m_theta < 0.01f) m_theta = 0.01f;
        else if (m_theta > glm::pi<float>() - 0.01f) m_theta = glm::pi<float>() - 0.01f;

        m_phi -= changeX;
        if (m_phi < 0) m_phi += 2 * glm::pi<float>();
        else if (m_phi > 2 * glm::pi<float>()) m_phi -= 2 * glm::pi<float>();

        m_oldX = (float) x;
        m_oldY = (float) y;

        m_direction.x = sin(m_theta) * sin(m_phi);
        m_direction.y = cos(m_theta);
        m_direction.z = sin(m_theta) * cos(m_phi);
    } else
    {
        m_oldX = (float) x;
        m_oldY = (float) y;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_speed +=  deltaTime*10;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_speed -=  deltaTime*10;
    else m_speed = 0.0f;

    m_cameraPos += m_speed * m_direction * deltaTime;
    viewmatrix = glm::lookAt(m_cameraPos, m_cameraPos + m_direction, m_up);
    m_viewmatrix = viewmatrix;
}


void Camera::setCenter(glm::vec3* center)
{
    m_center = *center;
}

CamMode Camera::getCamMode()
{
    return m_camMode;
}

glm::vec3 Camera::getCameraPos()
{
    return m_cameraPos;
}

glm::mat4 Camera::getView()
{
    return m_viewmatrix;
}

void Camera::setRadius(float radius)
{
    m_radius = radius;
}
void Camera::setCameraPos(glm::vec3 pos)
{
    m_cameraPos = pos;
}
