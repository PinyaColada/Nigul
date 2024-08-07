#include"Camera.h"
#include <iostream>
void Camera::updateMatrix()
{
	cameraMatrix = projectionMatrix * viewMatrix;
}

void Camera::updateView(const glm::mat4& matrix){
	viewMatrix = matrix;
}

void Camera::updateView(const glm::vec3& position, const glm::vec3& target)
{
	viewMatrix = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void OrthographicCamera::updateProjection()
{
	projectionMatrix = glm::ortho(-size.x, size.x, -size.y, size.y, nearPlane, farPlane);
}

void PerspectiveCamera::updateProjection()
{
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
