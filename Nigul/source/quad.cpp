#include "Quad.h"

MeshQuad::MeshQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // And unbind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

MeshQuad::~MeshQuad() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

FXQuad::FXQuad(int width, int height) : width(width), height(height){
	fbo = std::make_unique<FBO>(width, height, 0, false);
	quad = std::make_unique<MeshQuad>();
    shader = std::make_unique<Shader>("quad.vert", "tonemapping.frag");
}

FXQuad::~FXQuad() {
}

void FXQuad::toViewport() const{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo->tex->ID);

    glBindVertexArray(quad->vao);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void FXQuad::passUniforms() const {
    shader->activate();

    shader->setInt("screenTexture", 0);

    shader->setBool("isToneMappingApplied", isToneMappingApplied);
    shader->setFloat("exposure", exposure);
}
