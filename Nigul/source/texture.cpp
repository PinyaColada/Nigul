#include "texture.h"
#include <chrono>

Texture::Texture(const char* image, GLuint slot){
	// Reads the image from a file and stores it in bytes
	bytes = stbi_load(image, &width, &height, &numColCh, 0);
	unit = slot;

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);
	// Assigns the texture to a Texture Unit
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);

	// Configures the type of algorithm that is used to make the image smaller or bigger
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Configures the way the texture repeats (if it does at all)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Assigns the image to the OpenGL Texture object
	if (numColCh == 4)
		glTexImage2D
		(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width,
			height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			bytes
		);
	else if (numColCh == 3)
		glTexImage2D
		(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width,
			height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			bytes
		);
	else if (numColCh == 1)
		glTexImage2D
		(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width,
			height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			bytes
		);
	else
		throw std::invalid_argument("Automatic Texture type recognition failed");

	// Generates MipMaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Deletes the image data as it is already in the OpenGL Texture object
	stbi_image_free(bytes);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &ID);
}

std::unique_ptr<Texture> Texture::createShadowMapTexture(int width, int height, GLuint slot) {
	std::unique_ptr<Texture> tex = std::make_unique<Texture>();
	tex->unit = slot;
	tex->width = width;
	tex->height = height;

	glGenTextures(1, &tex->ID);
	glBindTexture(GL_TEXTURE_2D, tex->ID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

std::unique_ptr<Texture> Texture::createColorTexture(int width, int height, GLuint slot) {
	std::unique_ptr<Texture> tex = std::make_unique<Texture>();
	tex->unit = slot;
	tex->width = width;
	tex->height = height;

	glGenTextures(1, &tex->ID);
	glBindTexture(GL_TEXTURE_2D, tex->ID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

std::unique_ptr<Texture> Texture::createMultisampleTexture(int width, int height, GLuint slot) {
	std::unique_ptr<Texture> tex = std::make_unique<Texture>();
	tex->unit = slot;
	tex->width = width;
	tex->height = height;
	tex->isMultisampled = true;

	glGenTextures(1, &tex->ID);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex->ID);

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, SAMPLES, GL_RGBA32F, width, height, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return tex;
}

void Texture::texUnit(Shader* shader, const char* uniform)
{
	// Gets the location of the uniform
	GLuint texUni = glGetUniformLocation(shader->ID, uniform);
	// Shader needs to be activated before changing the value of a uniform
	shader->activate();
	// Sets the value of the uniform
	glUniform1i(texUni, unit);
}

void Texture::bind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	if (isMultisampled)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
	else
		glBindTexture(GL_TEXTURE_2D, ID);
}