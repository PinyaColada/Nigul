#pragma once

#include "FBO.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum LIGHT_TYPE {
	POINTLIGHT = 0,
	SPOTLIGHT = 1,
	DIRECTIONAL = 2
};

class Light {
public:
	Light() = default;

	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0;
	float range = 3.0;
	glm::vec3 position = glm::vec3(0.0f);

	bool castShadows = true;
	float shadowBias = 0.00001f;
	std::unique_ptr<FBO> shadowMap = nullptr;
	Camera* camera = nullptr;

	int index = 0;

	virtual void updateProjection() = 0;
	virtual void updatePosition(const glm::mat4& mat) = 0;
	virtual LIGHT_TYPE getType() = 0;
};

class SpotLight : public Light {
public:
	SpotLight() = default;

	glm::vec3 direction = glm::vec3(0.0f);
	float innerConeAngle = 0.9;
	float outerConeAngle = 0.95;

	void updateProjection() override;
	void updatePosition(const glm::mat4& mat) override;
	LIGHT_TYPE getType() override;
};

class DirectionalLight : public Light {
public:
	DirectionalLight() = default;

	float distance = 5.0f;

	void updateProjection() override;
	void updatePosition(const glm::mat4& mat) override;
	LIGHT_TYPE getType() override;
};

class PointLight : public Light {
public:
	PointLight() = default;

	float attenuation = 1.0f;

	void updateProjection() override;
	void updatePosition(const glm::mat4& mat) override;
	LIGHT_TYPE getType() override;
};
