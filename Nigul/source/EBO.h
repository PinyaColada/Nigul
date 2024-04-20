#pragma once

#include <glad/glad.h>
#include<vector>

/**
 * @class EBO
 * @brief Represents an OpenGL Element Buffer Object (EBO) used for indexing vertices.
 */
class EBO
{
public:
    GLuint ID; ///< ID reference of the Element Buffer Object (EBO)

    /**
     * @brief Constructs an EBO and links it to indices.
     *
     * @param indices Pointer to the array of indices.
     */
    EBO(std::vector<GLuint>& indices);

    /**
     * @brief Binds the EBO to the current OpenGL context.
     */
    void Bind();

    /**
     * @brief Unbinds the EBO from the current OpenGL context.
     */
    void Unbind();

    /**
     * @brief Deletes the EBO and frees its resources.
     */
    void Delete();
};


