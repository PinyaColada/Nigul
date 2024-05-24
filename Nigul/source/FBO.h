#pragma once
#include <glad/glad.h>
#include <iostream>

#include "texture.h"

class FBO {
public:
    FBO(int width, int height, int slot, bool useDepthTexture);
    ~FBO();

    void bind();
    void unbind();

    GLuint fbo;
    GLuint depthBuffer;

    std::unique_ptr<Texture> tex = nullptr;

    int width, height;
};