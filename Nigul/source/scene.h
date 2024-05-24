#pragma once

#include <vector>
#include "model.h"
#include "skybox.h"

class SceneManager {
public:

    std::map<std::string, std::unique_ptr<Skybox>> skyboxes;
    std::string mainSkybox = "None";

    std::map<std::string, std::unique_ptr<Model>> models;
    std::string mainModel = "None";

    SceneManager() = default;
    void loadScene();

    // Dynamic skybox methods
    void loadSkybox(std::string name);
    std::vector<std::string> getAllSkyboxes();

    // Dynamic model methods
    void loadModel(std::string path);
    std::vector<std::string> getAllModels();

    // Getters 
    inline Model* getMainModel() { return models[mainModel].get(); }
    inline Skybox* getMainSkybox() { return skyboxes[mainSkybox].get(); }

};

