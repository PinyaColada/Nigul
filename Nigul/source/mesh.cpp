#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, Material* material)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::material = material;

	VAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(indices);
	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}


void Mesh::Draw(Shader* shader, Camera* camera, glm::mat4 matrix)
{
	// Bind shader to be able to access uniforms
	shader->Activate();
	VAO.Bind();

	if (material)
		material->bind(shader);

	if (camera) {
		shader->setVec3("camPos", camera->getPosition());
		shader->setMat4("camMatrix", camera->cameraMatrix);
	}

	shader->setMat4("model", matrix);

	if (material) {
		if (material->doubleSided) {
			glDisable(GL_CULL_FACE);
		}
		if (material->alphaMode == BLEND_MODE) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	if (material) {
		if (material->doubleSided) {
			glEnable(GL_CULL_FACE);
		}
		if (material->alphaMode == BLEND_MODE) {
			glDisable(GL_BLEND);
		}
	}
}