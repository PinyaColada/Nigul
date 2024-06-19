#pragma once
#include <glad/glad.h>
#include <random>

#include "FBO.h"
#include "Shader.h" 
#include "camera.h"

enum FXType {
	FX_TONEMAP,
	FX_ABERRATION,
    FX_MSAA,
    FX_SSAO
};

class MeshQuad {
public:
    MeshQuad();
    ~MeshQuad();

    GLuint vao, vbo;
};

class FXQuad {
public:
    FXQuad(int width, int height);
    virtual ~FXQuad();

    virtual void passUniforms() const = 0;
    virtual FXType getType() const = 0;
    virtual void toViewport() const;

    std::unique_ptr<FBO> fbo = nullptr;
    std::unique_ptr<FBO> fboAux = nullptr;
    std::unique_ptr<MeshQuad> quad = std::make_unique<MeshQuad>();
    std::unique_ptr<Shader> shader = nullptr;
    std::unique_ptr<FXQuad> nextFX = nullptr;

    int width;
    int height;
};

class FXTonemap : public FXQuad {
public:
    FXTonemap(int width, int height);
    ~FXTonemap() override;

    void passUniforms() const override;
    FXType getType() const override;

    bool isToneMappingApplied = false;
    float exposure = 1.0f;
};

class FXAberration : public FXQuad {
public:
    FXAberration(int width, int height);
    ~FXAberration() override;

    void passUniforms() const override;
    FXType getType() const override;

    float aberration = 0.0f;
    bool isAberrationApplied = false;
};

class FXMsaa : public FXQuad {
public:
    FXMsaa(int width, int height);
    ~FXMsaa() override;

    FXType getType() const override;
    void passUniforms() const override;
};

class FXSsao : public FXQuad {
public:
    FXSsao(int width, int height, int numSamples, float radius, bool hemi);
    ~FXSsao() override;

    FXType getType() const override;
    void passUniforms(Camera* camera, FBO* depthCamera, FBO* normalCamera) const; // TODO: Change values, lets use the two FBO

    int numSamples = 64;
    float radius = 0.5f;
    bool hemi = true;
    Camera* camera;

    std::vector<glm::vec3> generateSamples() const;
};