#include "Mesh.h"

Primitive::Primitive(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, Material* material)
{
	Primitive::vertices = vertices;
	Primitive::indices = indices;
	Primitive::material = material;

	vao.bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(indices);
	// Links VBO attributes such as coordinates and colors to VAO
	vao.linkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	vao.linkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	vao.linkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	vao.linkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	vao.unbind();
	VBO.Unbind();
	EBO.Unbind();
}