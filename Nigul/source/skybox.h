#pragma once

#include <stb/stb_image.h>
#include <string>
#include <vector>
#include <iostream>
#include <glad/glad.h>

class Skybox
{
public:
	GLuint cubemapTexture;
	GLuint slot;

	unsigned int VAO;
	bool isLoaded = false;

	std::string folderPath;

	Skybox(const std::string& folderPath, int slot);
};

