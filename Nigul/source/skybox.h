#pragma once

#include "mesh.h"
#include "shader.h"

#include <stb/stb_image.h>

class Skybox
{
public:
	GLuint cubemapTexture;
	GLuint slot;

	unsigned int VAO;
	bool isLoaded = false;

	std::string folderPath;

	Skybox(const std::string& folderPath, int slot);
	void draw(Camera* camera, Shader* shader);
};

