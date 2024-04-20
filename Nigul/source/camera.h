#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"shader.h"

enum CameraType {
	PERSPECTIVE,
	ORTHOGRAPHIC
};

class Camera
{
public:
	// Stores the main vectors of the camera
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 orientation = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	float nearPlane = 0.1;
	float farPlane = 100; 
	float fov = 45;
	float aspectRatio = 1;

	CameraType type;

	// Prevents the camera from jumping around when first clicking left click
	bool firstClick = true;

	// Const for camera
	static constexpr float NORMAL_SPEED = 0.1f;
	static constexpr float FAST_SPEED = 0.25f;

	// Adjust the speed of the camera and it's sensitivity when looking around
	float speed = 0.1f;
	float sensitivity = 0.01f;
	double axisX = 600;
	double axisY = 600;

	// Updates the camera matrix to the Vertex Shader
	void updateMatrix();
	// Handles camera inputs
	void Inputs(GLFWwindow* window);
};

