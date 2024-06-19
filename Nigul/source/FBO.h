#pragma once
#include <glad/glad.h>
#include <iostream>

#include "texture.h"

enum FBO_TYPE {
    FBO_DEPTH,
	FBO_ONE_COLOR,
    FBO_TWO_COLOR,
    FBO_THREE_COLOR,
    FBO_FOUR_COLOR,
    FBO_MULTISAMPLE
};

class FBO {
public:
    FBO(int width, int height, int slot, FBO_TYPE fboType);
    ~FBO();

    void bind();
    void unbind();

    GLuint ID;
    GLuint depthBuffer;

    std::vector<std::unique_ptr<Texture>> colorTextures = {};
    std::unique_ptr<Texture> depthTex;

    int width, height;
};