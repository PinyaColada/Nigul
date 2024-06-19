#pragma once
#include <glad/glad.h>
#include <stb/stb_image.h>

#include "shader.h"

#define SAMPLES 16

class Texture
{
public:
	GLuint ID;
	GLuint unit;
	unsigned char* bytes;

	int width;
	int height;
	int numColCh;

	bool isMultisampled = false;

	Texture() = default;
	Texture(const char* image, GLuint slot); // Loads image
	static std::unique_ptr<Texture> createShadowMapTexture(int width, int height, GLuint slot); // Creates a shadow map
	static std::unique_ptr<Texture> createColorTexture(int width, int height, GLuint slot); // Creates a color texture
	static std::unique_ptr<Texture> createMultisampleTexture(int width, int height, GLuint slot); // Creates a multisample texture
	~Texture();

	// Assigns a texture unit to a texture
	void texUnit(Shader* shader, const char* uniform);
	void bind();
};

