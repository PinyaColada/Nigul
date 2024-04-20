#pragma once

#include<string>
#include<vector>
#include<glm/glm.hpp>
#include <algorithm>

#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Material.h"

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Material* material = nullptr;
	// Store VAO in public so it can be used in the Draw function
	VAO VAO;

	// Properties of mesh TODO: pass this properties to a material class
	bool hasNoTexture = false;

	// Initializes the mesh
	Mesh(){};
	Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, Material* material = nullptr);

	// Draws the mesh
	void Draw(Shader* shader, Camera* camera, glm::mat4 matrix);
};

