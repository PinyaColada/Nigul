#pragma once
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <map>

#include "texture.h"

// Enum for Alpha Mode
enum ALPHAMODE {
    BLEND_MODE,
    OPAQUE_MODE
};

struct PbrMetallicRoughness {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);  // len = 4. default [1,1,1,1]
    Texture* baseColorTexture = nullptr;
    Texture* metallicRoughness = nullptr;
    float metallicFactor = 1.0f;   // default 1
    float roughnessFactor = 1.0f;  // default 1};
};

class Material {
public:
    std::string name = "";

    ALPHAMODE alphaMode = OPAQUE_MODE;
    float alphaCutoff = 0.5f;
    bool doubleSided = false;

    Texture* emissiveTexture = nullptr;
    glm::vec3 emissiveFactor = glm::vec3(0.0f);

    Texture* normalMap = nullptr;
    Texture* occlusionTexture = nullptr;

    PbrMetallicRoughness pbrMetallicRoughness;

    Material();

    void bind(Shader* shader);
};
