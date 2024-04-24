#include "light.h"

void SpotLight::updateProjection()
{
}

LIGHT_TYPE SpotLight::getType()
{
	return LIGHT_TYPE::SPOTLIGHT;
}

void SpotLight::updatePosition(const glm::mat4& mat)
{
	position = glm::vec3(mat[3]);
	direction = glm::normalize(-glm::vec3(mat[0][2], mat[1][2], mat[2][2]));
}

void DirectionalLight::updateProjection()
{
	glm::vec3 pos = position * distance;
	glm::vec3 target = glm::vec3(0.0f);

	camera.updateView(pos, target);
	camera.updateProjection();
	camera.updateMatrix();

	hasChanged = false;
}

LIGHT_TYPE DirectionalLight::getType()
{
	return LIGHT_TYPE::DIRECTIONAL;
}

void DirectionalLight::updatePosition(const glm::mat4& mat)
{
	position = glm::vec3(mat[3]);
}

void PointLight::updateProjection()
{
}

LIGHT_TYPE PointLight::getType()
{
	return LIGHT_TYPE::POINTLIGHT;
}

void PointLight::updatePosition(const glm::mat4& mat)
{
	position = glm::vec3(mat[3]);
}
