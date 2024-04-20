#pragma once

#include<glm/glm.hpp>
#include<glad/glad.h>
#include<vector>

// Structure to standardize the vertices used in the meshes
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texUV;
};

/**
 * @class VBO
 * @brief Represents an OpenGL Vertex Buffer Object (VBO) used for storing vertices in GPU memory.
 */
class VBO
{
public:
    GLuint ID; ///< Reference ID of the Vertex Buffer Object (VBO)

    /**
     * @brief Constructs a VBO and links it to vertices.
     *
     * @param vertices Pointer to the array of vertices.
     * @param size Size of the vertices array in bytes.
     */
    VBO(std::vector<Vertex>& vertices);

    /**
     * @brief Binds the VBO to the current OpenGL context.
     */
    void Bind();

    /**
     * @brief Unbinds the VBO from the current OpenGL context.
     */
    void Unbind();

    /**
     * @brief Deletes the VBO and frees its resources.
     */
    void Delete();
};
