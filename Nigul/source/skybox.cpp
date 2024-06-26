#include "skybox.h"

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

Skybox::Skybox(const std::string& folderPath, int slot)
{
	this->folderPath = folderPath;
	this->slot = slot;

	unsigned int VBO, EBO;
	glGenVertexArrays(1, &VAO); // Create VAO
	glGenBuffers(1, &VBO); // Create VBO
	glGenBuffers(1, &EBO); // Create EBO
	glBindVertexArray(VAO); // Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW); // Fill VBO with data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Bind EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW); // Fill EBO with data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Set vertex attributes
	glEnableVertexAttribArray(0); // Enable vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
	glBindVertexArray(0); // Unbind VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

	std::vector<std::string> faces
	{
		folderPath + "/px.png",
		folderPath + "/nx.png",
		folderPath + "/py.png",
		folderPath + "/ny.png",
		folderPath + "/pz.png",
		folderPath + "/nz.png"
	};

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			if (nrChannels == 3)
			{
				glTexImage2D
				(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGB,
					width,
					height,
					0,
					GL_RGB,
					GL_UNSIGNED_BYTE,
					data
				);
			}
			else if (nrChannels == 4)
			{
				glTexImage2D
				(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_RGBA,
					width,
					height,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					data
				);
			}
			else
			{
				std::cout << "nrChannels not 3 or 4" << std::endl;
			}

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Unbind texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}