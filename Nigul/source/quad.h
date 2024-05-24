#pragma once
#include <glad/glad.h>
#include "FBO.h"
#include "Shader.h"

class MeshQuad {
public:
    MeshQuad();
    ~MeshQuad();

    GLuint vao, vbo;
};

class FXQuad {
public:
    FXQuad(int width, int height);
    ~FXQuad();

    std::unique_ptr<FBO> fbo;
    std::unique_ptr<MeshQuad> quad;
    std::unique_ptr<Shader> shader;

    int width;
    int height;

    void passUniforms() const;
    void toViewport() const;

    bool isToneMappingApplied = false;
    float exposure = 1.0f;
};
