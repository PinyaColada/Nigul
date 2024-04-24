#pragma once

#include <vector>
#include "model.h"
#include "skybox.h"

using json = nlohmann::json;

class Scene {
public:
    // Scene properties
    float ambientLight = 0.05f;

    glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float shadowDarkness = 1.0f;
    float reflectionFactor = 0.5f;

    std::unique_ptr<Model> model = nullptr;
    // std::unique_ptr<Skybox> skybox = std::make_unique<Skybox>("skyboxes/night/");

    std::map<std::string, std::unique_ptr<Skybox>> skyboxes;
    std::string mainSkybox = "None";
    int loadedSkyboxes = 0;

    Camera* camera = nullptr;

    std::unique_ptr<Shader> skyboxShader;
    std::unique_ptr<Shader> defaultShader;
    std::unique_ptr<Shader> shadowShader;

    std::string modelPath;
    std::string skyboxPath;

    bool firstClick = true;

    float speed = 0.1f;
    float sensitivity = 0.01f;
    double axisX = 600;
    double axisY = 600;

    Scene();

    // Method to pass scene properties to the shader
    void passSceneProperties();

    void loadSkybox(std::string name);
    std::vector<std::string> getAllSkyboxes();

    // Draw method
    void drawScene();

    // Method to save the scene to a JSON file
    void saveScene();

    // Method to load the scene from a JSON file
    void loadScene();

    // Input method
    void processInput(GLFWwindow* window);

    // Helper method to serialize scene properties to JSON
    json serializeSceneProperties();
};

