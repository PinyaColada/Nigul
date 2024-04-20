#include "Quad.h"

Quad::Quad() {
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

Quad::~Quad() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void Quad::renderTexture(GLuint texture, int width, int height) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(vao);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
}

postProcessing::postProcessing(int width, int height) : width(width), height(height){
	fbo = std::make_unique<FBO>(width, height, false);
	quad = std::make_unique<Quad>();
    shader = std::make_unique<Shader>("quad.vert", "quad.frag");
}

postProcessing::~postProcessing() {
}

void postProcessing::render() const {
    shader->Activate();

    shader->setInt("screenTexture", 0);

    shader->setInt("sampleMSAA", sampleMSAA);

    shader->setBool("isAberrationApplied", isAberrationApplied);
    shader->setBool("isToneMappingApplied", isToneMappingApplied);

    shader->setFloat("amountAberration", amountAberration);

	quad->renderTexture(fbo->texture, width, height);
}
