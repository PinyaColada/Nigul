#pragma once

#include "model.h"
#include "skybox.h"
#include "quad.h"

// Estructura o clase renderCall no definida en el enunciado, se asume una estructura básica
struct renderCall {
    Mesh* mesh;
    Shader* shader;
    Camera* camera;
    glm::mat4 matrix;
};

class Renderer {
public:
    std::map<std::string, std::unique_ptr<Shader>> shaderMap;
    std::unique_ptr<FXQuad> postpo = nullptr;

    Renderer(int width, int height);

    void render(Model* model, Skybox* skybox);
    void render(FXQuad* fxQuad);
    void render(renderCall);

    void renderModel(Model* model, Shader* shader, Camera* camera);
    void renderSkybox(Skybox* skybox, Shader* shader, Camera* camera);

    std::vector<renderCall> getRenderCalls(Node* node, Shader* shader, Camera* camera);

    void setAllUniforms(Model* model, Skybox* skybox, Shader* shader);
    void setLightPositionsUniform(Shader* shader, Model* model);
    void setLightColorsUniform(Shader* shader, Model* model);
    void setLightTypesUniform(Shader* shader, Model* model);
    void setLightIntensitiesUniform(Shader* shader, Model* model);
    void setLightRangesUniform(Shader* shader, Model* model);
    void setLightShadowBiasesUniform(Shader* shader, Model* model);
    void setLightAttenuationsUniform(Shader* shader, Model* model);
    void setLightProjectionMatricesUniform(Shader* shader, Model* model);
    void setLightDirectionsUniform(Shader* shader, Model* model);
    void setLightInnerConeAnglesUniform(Shader* shader, Model* model);
    void setLightOuterConeAnglesUniform(Shader* shader, Model* model);
    void setLightCastShadowsUniform(Shader* shader, Model* model);
    void setLightShadowMapSamplesUniform(Shader* shader, Model* model);
    void setAmbientLightUniform(Shader* shader, Model* model);
    void setAmbientColorUniform(Shader* shader, Model* model);
    void setShadowDarknessUniform(Shader* shader, Model* model);
    void setReflectionFactorUniform(Shader* shader, Model* model);
    void setSkyboxUniforms(Shader* shader, Model* model, Skybox* skybox);

    void renderShadowMap(Model* model);
};