#include "Camera.h"

Camera::Camera(float fov, float width, float height, float fnear, float ffar, glm::vec3 position, GLFWwindow* window)
	: m_ProjectionMatrix(glm::perspective(glm::radians(fov), width / height, fnear, ffar)), m_Position(position), m_Window(window), m_Width(width), m_Height(height)
{
	m_ProjectionMatrix[1][1] *= -1;
	RecaluclateViewMatrix();
	glfwSetCursorPos(m_Window, m_LastX, m_LastY);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Camera::update(float time)
{
	GLFWwindow* window = m_Window;
	if (glfwGetKey(window, GLFW_KEY_A))
	{
		//std::cout << "left" << std::endl;
		Translate(Camera_Movement::LEFT, time);
	}
	if (glfwGetKey(window, GLFW_KEY_D))
	{
		//std::cout << "Right" << std::endl;
		Translate(Camera_Movement::RIGHT, time);
	}
	if (glfwGetKey(window, GLFW_KEY_W))
	{
		//std::cout << "Forward" << std::endl;
		Translate(Camera_Movement::FORWARD, time);
	}
	if (glfwGetKey(window, GLFW_KEY_S))
	{
		//std::cout << "Backward" << std::endl;
		Translate(Camera_Movement::BACKWARD, time);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE))
		Translate(Camera_Movement::UP, time);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		Translate(Camera_Movement::DOWN, time);

	if (glfwGetMouseButton(window, 1) && !m_LockMouse)
	{
		m_LockMouse = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(window, m_MiddleOfScreen.first, m_MiddleOfScreen.second);
		m_LastX = m_MiddleOfScreen.first;
		m_LastY = m_MiddleOfScreen.second;
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		m_LockMouse = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (m_LockMouse && getMousePosition() != m_MiddleOfScreen)
	{
		std::pair<float, float> currentMousePos = getMousePosition();

		if (m_FirstMouse)
		{
			m_LastX = currentMousePos.first;
			m_LastY = currentMousePos.second;
			m_FirstMouse = false;
		}

		float xoffset = currentMousePos.first - m_LastX;
		float yoffset = m_LastY - currentMousePos.second; //Reversed since y-coordinates go from bottom to top

		if (currentMousePos.first < 0 || currentMousePos.first > m_MiddleOfScreen.first * 2 || currentMousePos.second < 0 || currentMousePos.second > m_MiddleOfScreen.second * 2)
		{
			glfwSetCursorPos(window, m_MiddleOfScreen.first, m_MiddleOfScreen.second);
			m_LastX = m_MiddleOfScreen.first;
			m_LastY = m_MiddleOfScreen.second;
		}
		else
		{
			m_LastX = currentMousePos.first;
			m_LastY = currentMousePos.second;
		}
		ProcessMouseMovement(xoffset, yoffset);
	}
	RecaluclateViewMatrix();
}

std::pair<float, float> Camera::getMousePosition()
{
	double x, y;
	glfwGetCursorPos(m_Window, &x, &y);
	return std::pair<float, float>(x, y);
}

const void Camera::Translate(Camera_Movement direction, float deltaTime)
{
	float velocity = m_CameraSpeed * deltaTime;
	if (direction == Camera_Movement::FORWARD)
		m_Position += m_Front * velocity;
	if (direction == Camera_Movement::BACKWARD)
		m_Position -= m_Front * velocity;
	if (direction == Camera_Movement::LEFT)
		m_Position -= m_Right * velocity;
	if (direction == Camera_Movement::RIGHT)
		m_Position += m_Right * velocity;
	if (direction == Camera_Movement::UP)
		m_Position += m_WorldUp * velocity;
	if (direction == Camera_Movement::DOWN)
		m_Position -= m_WorldUp * velocity;
}

void Camera::RecaluclateViewMatrix()
{

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	//yoffset = 0.0f;

	Yaw += xoffset;
	Pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void Camera::updateCameraVectors()
{
	// calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	m_Front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	//m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}