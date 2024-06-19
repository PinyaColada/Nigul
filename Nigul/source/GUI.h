#pragma once

#include <glm/glm.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuizmo.h>
#include <imgui/imgui.h>

#include "camera.h"
#include "scene.h"
#include "renderer.h"

class GUI {
public:
    bool showShadowMap = false;
    bool showNormalMap = false;

    bool firstClick = true;

    float speed = 0.1f;
    float sensitivity = 0.001f;
    double axisX = 600;
    double axisY = 600;

    GUI() = default;
    ~GUI();

    void init(GLFWwindow* window);

    void logic(SceneManager* model, Renderer* renderer);
    bool input(Model* model);

    void displayNode(Model* model, Node* node);
    bool displayGizmo(Model* model);
    void displayGeneral(Model* model);
    void displayLight(Model* model);
    void displayCamera(Model* model);
    void displayRender(Model* model, Renderer* render);
    void displayActions(SceneManager* scene);
    void displayFXAberration(FXAberration* fx);
    void displayFXTonemap(FXTonemap* fx);
    void filterFX(FXQuad* fx);
    void displayFX(Renderer* renderer);

    void createFrame(SceneManager* scene, Renderer* renderer);

    bool isMouseAvaliable();
};