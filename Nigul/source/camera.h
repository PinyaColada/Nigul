#pragma once

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/vector_angle.hpp>

class Camera
{
public:
    Camera() = default;

    float nearPlane = 0.1;
    float farPlane = 100;

    glm::mat4 cameraMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    glm::vec3 getPosition() const;

    bool hasChanged = true;

    void updateMatrix(); 
    void updateView(const glm::mat4& matrix);
    void updateView(const glm::vec3& position, const glm::vec3& target);
    virtual void updateProjection() = 0;

};

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera() = default;

    glm::vec2 size = glm::vec2(5.0f, 5.0f);

	void updateProjection() override;
};

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera() = default;

    float fov = 45.0f;
    float aspectRatio = 1.0f;

	void updateProjection() override;
};

