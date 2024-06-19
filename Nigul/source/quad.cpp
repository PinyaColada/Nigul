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

FXQuad::FXQuad(int width, int height) : width(width), height(height) {}

FXQuad::~FXQuad() {
}

void FXQuad::toViewport() const {
    for (int i = 0; i < fbo->colorTextures.size(); i++) {
        fbo->colorTextures[i]->bind();
    }

    glBindVertexArray(quad->vao);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

FXTonemap::FXTonemap(int width, int height) : FXQuad(width, height) {
    fbo = std::make_unique<FBO>(width, height, 0, FBO_ONE_COLOR);
    shader = std::make_unique<Shader>("quad.vert", "tonemapping.frag");
}

FXTonemap::~FXTonemap() {
}

void FXTonemap::passUniforms() const {
    shader->activate();

    shader->setInt("screenTexture", fbo->colorTextures[0]->unit);
    shader->setBool("isToneMappingApplied", isToneMappingApplied);
    shader->setFloat("exposure", exposure);
}

FXType FXTonemap::getType() const {
	return FX_TONEMAP;
}

FXAberration::FXAberration(int width, int height) : FXQuad(width, height) {
	fbo = std::make_unique<FBO>(width, height, 0, FBO_ONE_COLOR);
	shader = std::make_unique<Shader>("quad.vert", "aberration.frag");
}

FXAberration::~FXAberration() {
}

void FXAberration::passUniforms() const {
	shader->activate();

	shader->setInt("screenTexture", fbo->colorTextures[0]->unit);
	shader->setBool("isAberrationApplied", isAberrationApplied);
	shader->setFloat("aberration", aberration);
}

FXType FXAberration::getType() const {
	return FX_ABERRATION;
}

FXMsaa::FXMsaa(int width, int height) : FXQuad(width, height) {
	fbo = std::make_unique<FBO>(width, height, 1, FBO_MULTISAMPLE);
    shader = std::make_unique<Shader>("quad.vert", "basic.frag");
}

FXMsaa::~FXMsaa() {
}

FXType FXMsaa::getType() const {
	return FX_MSAA;
}

void FXMsaa::passUniforms() const {
}

FXSsao::FXSsao(int width, int height, int numSamples, float radius, bool hemi) : FXQuad(width, height), numSamples(numSamples), radius(radius), hemi(hemi) {
	fbo = std::make_unique<FBO>(width, height, 1, FBO_ONE_COLOR);
	shader = std::make_unique<Shader>("quad.vert", "ssao.frag");
}

std::vector<glm::vec3> FXSsao::generateSamples() const{
    std::vector<glm::vec3> points;
    points.resize(numSamples);
    for (int i = 0; i < numSamples; i += 1)
    {
        glm::vec3& p = points[i];
        float u = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float v = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float theta = u * 2.0 * 3.1415926535897;
        float phi = acos(2.0 * v - 1.0);
        float r = cbrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.9 + 0.1) * radius;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);
        p.x = r * sinPhi * cosTheta;
        p.y = r * sinPhi * sinTheta;
        p.z = r * cosPhi;
        if (hemi && p.z < 0)
            p.z *= -1.0;
    }
    return points;
}

void FXSsao::passUniforms(Camera* camera, FBO* depthCamera, FBO* normalCamera) const {
    shader->activate();

    shader->setInt("cameraDepth", depthCamera->depthTex->unit);
    shader->setInt("cameraNormal", normalCamera->colorTextures[0]->unit);
    shader->setMat4("viewProjection", camera->cameraMatrix);
    shader->setMat4("inverseViewProjection", glm::inverse(camera->cameraMatrix));
    shader->setFloat("radius", radius);
    glm::vec3 samplesUni[64];
    auto samples = generateSamples();
    for (int i = 0; i < 64; i++) {
        samplesUni[i] = samples[i];
    } 
    shader->setVecs3("samples", samplesUni, numSamples);
}