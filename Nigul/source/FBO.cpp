#include "FBO.h"

FBO::FBO(int width, int height, int slot, FBO_TYPE fboType) : width(width), height(height) {
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    if (fboType == FBO_DEPTH) {
        depthTex = Texture::createShadowMapTexture(width, height, slot);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex->ID, 0);

    } else if (FBO_ONE_COLOR <= fboType && fboType <= FBO_FOUR_COLOR) {
        for (int i = 0; i < fboType; i++) {
            colorTextures.push_back(Texture::createColorTexture(width, height, slot + i));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTextures[i]->ID, 0);
        }

        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    }
    else if (fboType == FBO_MULTISAMPLE) {
		colorTextures.push_back(Texture::createMultisampleTexture(width, height, slot));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTextures[0]->ID, 0);

        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, SAMPLES, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	}

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FBO::~FBO() {
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(1, &ID);
}

void FBO::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glViewport(0, 0, width, height);
}

void FBO::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}