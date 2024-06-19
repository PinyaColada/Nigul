#include "scene.h"
#include <glm/gtx/string_cast.hpp>

void SceneManager::loadScene() {
    std::filesystem::path dirPath("skyboxes");
    skyboxes["None"] = nullptr;

    if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_directory()) {
                skyboxes[entry.path().filename().string()] = nullptr;
            }
        }
    }

    std::filesystem::path modelPath("models");
    models["None"] = nullptr;

    if (std::filesystem::exists(modelPath) && std::filesystem::is_directory(modelPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(modelPath)) {
            if (entry.is_directory()) {
                for (const auto& file : std::filesystem::directory_iterator(entry.path())) {
                    if (file.path().extension() == ".gltf") {
                        auto fullPathFile = modelPath / entry.path().filename() / file.path().filename();
                        std::unique_ptr<Model> model = std::make_unique<Model>();
                        model->file = fullPathFile.string();
                        models[file.path().stem().string()] = std::move(model);
                        break;
                    }
                }
            }
        }
    }
}

void SceneManager::loadSkybox(std::string name)
{
    if (name != "None" && skyboxes[name] == nullptr) {
        skyboxes[name] = std::make_unique<Skybox>("skyboxes/" + name, 90 + skyboxes.size());
    }

    mainSkybox = name;
}

void SceneManager::loadModel(std::string path)
{
    if (path != "None") {
        if (!models[path]->loaded)
            models[path]->load();
	}

	mainModel = path;
}

std::vector<std::string> SceneManager::getAllSkyboxes()
{
	std::vector<std::string> skyboxNames;
    for (const auto& [key, value] : skyboxes) {
		skyboxNames.push_back(key);
	}
	return skyboxNames;
}

std::vector<std::string> SceneManager::getAllModels()
{
	std::vector<std::string> modelNames;
    for (const auto& [key, value] : models) {
		modelNames.push_back(key);
	}
	return modelNames;
}
