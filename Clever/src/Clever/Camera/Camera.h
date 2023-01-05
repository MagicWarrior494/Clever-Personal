#pragma once
#include <glm.hpp>
#include <gtx/transform.hpp>
#include <array>
#include <iostream>
#include <GLFW/glfw3.h>

enum class Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{

public:
	Camera()
	{

	}
	Camera(float fov, float width, float height, float fnear, float ffar, glm::vec3 position, GLFWwindow* window);

	void update(float time);

	std::pair<float, float> getMousePosition();

	const glm::vec3& GetPosition() const { return m_Position; }
	void SetPosition(const glm::vec3& position) { m_Position = position; RecaluclateViewMatrix(); }

	const glm::vec3& GetRotation() const { return m_Front; }
	void SetRotation(const glm::vec3& rotation) { m_Front = rotation; RecaluclateViewMatrix(); }

	const void Translate(Camera_Movement direction, float deltaTime);

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	void updateCameraVectors();

	inline glm::vec3 getFront() const { return m_Front; }
	inline glm::vec3 getRight()const { return m_Right; }
	inline glm::vec3 getWorldUp()const { return m_WorldUp; }

	inline glm::vec3 getVelocity() const { return m_Velocity; }
	inline void getVelocity(glm::vec3 velocity) { m_Velocity = velocity; }

	const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	void RecaluclateViewMatrix();
	const void SetMovementSpeed(float CameraSpeed) { m_CameraSpeed = CameraSpeed; }
	const float GetMovementSpeed() { return m_CameraSpeed; }
	const void SetRotationSpeed(float CameraSpeed) { MouseSensitivity = CameraSpeed; }
	const float GetRotationSpeed() { return MouseSensitivity; }
private:
	int m_Width, m_Height;

	bool m_LockMouse = true;
	std::pair<float, float> m_MiddleOfScreen = { m_Width / 2, m_Height / 2};
	float m_LastX = m_Width / 2;
	float m_LastY = m_Height / 2;
	bool m_FirstMouse = true;

	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ViewProjectionMatrix;
	glm::vec3 m_Up = glm::vec3(0, 1, 0);
	glm::vec3 m_WorldUp = m_Up;
	float m_Fov = 45.0f;

	glm::vec3 m_Position = { 0.0f,0.0f,0.0f };
	glm::vec3 m_Velocity = { 0,0,0 };
	glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
	glm::vec3 m_Right = { -1, 0, 0 };
	float m_CameraSpeed = 1000.0f;

	float Yaw = -90.0f;
	float Pitch = 0.0f;
	float SPEED = 2.5f;
	float MouseSensitivity = 0.1f;
	float ZOOM = 45.0f;

	GLFWwindow* m_Window;
};