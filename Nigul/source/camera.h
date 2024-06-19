#pragma once

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/vector_angle.hpp>

enum CAMERA_TYPE
{
	ORTHOGRAPHIC,
	PERSPECTIVE
};

class Camera
{
public:
    Camera() = default;

    float nearPlane = 0.1;
    float farPlane = 1000;

    glm::mat4 cameraMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    glm::vec3 getPosition() const;
    virtual CAMERA_TYPE getType() = 0;

    bool hasChanged = true;
    bool enabled = true;
    int index = 0;

    void updateMatrix(); 
    void updateView(const glm::mat4& matrix);
    void updateView(const glm::vec3& position, const glm::vec3& target);
    virtual void updateProjection() = 0;

};

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera() = default;
    CAMERA_TYPE getType() {return CAMERA_TYPE::ORTHOGRAPHIC;}

    glm::vec2 size = glm::vec2(25.0f, 25.0f);


	void updateProjection() override;
};

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera() = default;
    CAMERA_TYPE getType() {return CAMERA_TYPE::PERSPECTIVE;}

    float fov = 45.0f;
    float aspectRatio = 1.0f;

	void updateProjection() override;
};

