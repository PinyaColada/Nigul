#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>

#include "VAO.h"
#include "EBO.h"
#include "Material.h"

struct Primitive
{
	std::vector <Vertex> vertices;
	std::vector <GLuint> indices;
	Material* material = nullptr;

	VAO vao;

	Primitive(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, Material* material = nullptr);
};

class Mesh
{
public:
	std::vector<Primitive> primitives;

	Mesh() = default;
};

