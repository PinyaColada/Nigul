#pragma once
#include <glad/glad.h>
#include <iostream>

class FBO {
public:
    FBO(int width, int height, bool justDepthBuffer = false);
    ~FBO();

    void Bind();
    void Unbind();
    GLuint GetTexture();

    GLuint fbo;
    GLuint texture;
    GLuint depthBuffer;
    int width, height;
};