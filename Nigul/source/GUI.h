#pragma once

#include <glm/glm.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuizmo.h>
#include <imgui/imgui.h>

#include "camera.h"
#include "scene.h"
#include "shader.h"
#include "quad.h"

enum GUI_MODE {
    MODEL,
    LIGHT,
    WORLD
};

class GUI {
public:
    GUI_MODE mode;
    int selectedNodeId;
    bool showShadowMap;
    Scene* scene = nullptr;
    postProcessing* quad = nullptr;

    float rotationRootX = 0.0f;
    float rotationRootY = 0.0f;

    GUI(Scene* scene, postProcessing* quad);

    void init(GLFWwindow* window);
    void logic();
    void displayNode(Node& node);
    void createFrame();
    void destroy();
    void input();
    void editTransform(const Camera& camera, glm::f32* matrix);

    bool isMouseAvaliable();
};