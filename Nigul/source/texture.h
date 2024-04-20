#pragma once
#include <glad/glad.h>
#include <stb/stb_image.h>

#include "shader.h"


class Texture
{
public:
	GLuint ID;
	GLuint unit;
	unsigned char* bytes;

	int width;
	int height;
	int numColCh;

	Texture(const char* image, GLuint slot); // Loads image

	// Assigns a texture unit to a texture
	void texUnit(Shader* shader, const char* uniform);
	// Sends the data to OpenGL
	void texToOpenGL();
	// Binds a texture
	void Bind();
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();
};

