#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>

#include "VAO.h"
#include "EBO.h"
#include "Material.h"

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Material* material = nullptr;

	// Store VAO in public so it can be used in the Draw function
	VAO VAO;

	// Initializes the mesh
	Mesh() = default;
	Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, Material* material = nullptr);
};

