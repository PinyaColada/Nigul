#pragma once

#include "GUI.h"
#include "scene.h"
#include "skybox.h"
#include "quad.h"

#include <iostream>
#include <chrono>

class Application {
public:
    // Constructor
    Application(int width, int height, const std::string& title);

    // Methods
    void init();
    void loop();
    void destroy();

    int width = 1200;
    int height = 1200;
    std::string title = "Nigul";

    GLFWwindow* window = nullptr;
    std::unique_ptr<Scene> sceneManager = nullptr;
    std::unique_ptr<GUI> menu = nullptr;
    std::unique_ptr<postProcessing> postpo = nullptr;
};
