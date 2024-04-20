#pragma once
#include <glad/glad.h>
#include "FBO.h"
#include "Shader.h"

class Quad {
public:
    Quad();
    ~Quad();

    void renderTexture(GLuint texture, int width, int height);

    GLuint vao, vbo;
};

class postProcessing {
public:
    postProcessing(int width, int height);
    ~postProcessing();

    std::unique_ptr<FBO> fbo;
    std::unique_ptr<Quad> quad;
    std::unique_ptr<Shader> shader;

    int width;
    int height;

    void render() const;

    bool isToneMappingApplied = false;
    bool isAberrationApplied = false;

    float amountAberration = 0.01f;

    int sampleMSAA = 0;
};
