#include "scene.h"
#include <glm/gtx/string_cast.hpp>

Scene::Scene(){
}

json Scene::serializeSceneProperties() {
    json j;

    j["lastModel"] = model->file;

    return j;
}

void Scene::saveScene() {
    // Get the JSON representation of the scene
    json sceneJson = serializeSceneProperties();

    // Write the JSON object to a file
    std::ofstream file("../.config.json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file for writing.");
    }

    // Dump the JSON content into the file
    file << sceneJson.dump(4); // The '4' specifies pretty printing with an indent of 4 spaces

    // Close the file
    file.close();
}

void Scene::loadScene() {
    // Open the JSON file
    std::ifstream file(".config.json");
    if (!file) {
        std::cerr << "Failed to open config file " << std::endl;
    }

    // Parse the JSON file
    json sceneJson;
    file >> sceneJson;

    // Close the file
    file.close();

    skyboxShader = std::make_unique<Shader>("skybox.vert", "skybox.frag");
    defaultShader = std::make_unique<Shader>("default.vert", "default.frag");
    shadowShader = std::make_unique<Shader>("shadow.vert", "shadow.frag");

    if (sceneJson.contains("lastModel")) {
        auto modelInfo = std::make_unique<Model>();
        modelInfo->file = sceneJson["lastModel"].get<std::string>();
        modelInfo->load();
        if (modelInfo->mainCameraId == -1)
			modelInfo->AddCameraNode();

        camera = modelInfo->getNodeByID(modelInfo->mainCameraId)->camera;
        model = std::move(modelInfo);
    }

    std::filesystem::path dirPath("skyboxes/");
    skyboxes["None"] = nullptr;

    if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_directory()) {
                skyboxes[entry.path().filename().string()] = nullptr;
            }
        }
    }
}

void Scene::passSceneProperties()
{
    if (model) {
        model->renderShadowMaps(shadowShader.get());
        model->passLightUniforms(defaultShader.get());

        bool hasSkybox = (skyboxes[mainSkybox] != nullptr);
		defaultShader->setBool("hasSkybox", hasSkybox);
        if (hasSkybox) {
            defaultShader->setCubemap("skybox", skyboxes[mainSkybox]->cubemapTexture, skyboxes[mainSkybox]->slot);
        }

        defaultShader->setFloat("ambientLight", ambientLight);
        defaultShader->setVec3("ambientColor", ambientColor);
        defaultShader->setFloat("shadowDarkness", shadowDarkness);
        defaultShader->setFloat("reflectionFactor", reflectionFactor);
    }
}

void Scene::processInput(GLFWwindow* window)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        Node* cameraNode = model->getNodeByID(model->mainCameraId);
        glm::mat4 cameraMatrix = cameraNode->matrix;

        // Get the position and rotation
        glm::vec3 position = glm::vec3(cameraMatrix[3]);
        glm::quat rotation = glm::quat_cast(cameraMatrix);
        glm::vec3 orientation = glm::vec3(cameraMatrix[0][2], cameraMatrix[1][2], cameraMatrix[2][2]);
        glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);

        // Handles key inputs
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position += speed * glm::vec3(rotation * glm::vec4(0, 0, -1, 0));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position -= speed * glm::vec3(rotation * glm::vec4(0, 0, -1, 0));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position += speed * glm::vec3(rotation * glm::vec4(-1, 0, 0, 0));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position -= speed * glm::vec3(rotation * glm::vec4(-1, 0, 0, 0));
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			position += speed * glm::vec3(rotation * glm::vec4(0, 1, 0, 0));
		}
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            position -= speed * glm::vec3(rotation * glm::vec4(0, 1, 0, 0));
        }

        if (firstClick) {
            // Fetches the coordinates of the cursor
            glfwGetCursorPos(window, &axisX, &axisY);
            firstClick = false;
        }

        // Stores the coordinates of the cursor
        double mouseX;
        double mouseY;
        // Fetches the coordinates of the cursor
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
        // and then "transforms" them into degrees 
        float yaw = sensitivity * (float)(mouseY - axisY);
        float pitch = sensitivity * (float)(mouseX - axisX);

        glm::quat pitchQuad = glm::rotate(rotation, glm::radians(-pitch), glm::normalize(glm::cross(orientation, up)));
        if (glm::angle(up * pitchQuad, up) < glm::radians(85.0f)) {
            rotation = pitchQuad;
        }

        glm::quat yawQuad = glm::rotate(rotation, glm::radians(-yaw), up);
        if (glm::angle(orientation * yawQuad, orientation) < glm::radians(85.0f)) {
			rotation = yawQuad;
		}

        glfwSetCursorPos(window, axisX, axisY);

        // Apply to the camera matrix
        cameraNode->matrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);

    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}

void Scene::drawScene()
{
    if (skyboxes[mainSkybox]) {
        skyboxes[mainSkybox]->draw(camera, skyboxShader.get());
    }

    if (model) {
        model->Draw(defaultShader.get(), camera);
    }
}

void Scene::loadSkybox(std::string name)
{
    if (name != "None" && skyboxes[name] == nullptr) {
        skyboxes[name] = std::make_unique<Skybox>("skyboxes/" + name, loadedSkyboxes++);
    }

    mainSkybox = name;
}

std::vector<std::string> Scene::getAllSkyboxes()
{
	std::vector<std::string> skyboxNames;
    for (const auto& [key, value] : skyboxes) {
		skyboxNames.push_back(key);
	}
	return skyboxNames;
}
