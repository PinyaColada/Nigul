#pragma once

#include <glad/glad.h>
#include "VBO.h"

/**
 * @class VAO
 * @brief Represents an OpenGL Vertex Array Object (VAO) used for managing vertex attributes.
 */
class VAO
{
public:
    GLuint ID; ///< ID reference for the Vertex Array Object (VAO)

    /**
     * @brief Constructs a VAO and generates an ID for it.
     */
    VAO();

    /**
     * @brief Links a Vertex Buffer Object (VBO) to the VAO using a specified layout.
     *
     * @param VBO The VBO to link to the VAO.
     * @param layout The layout index of the vertex attribute to be linked.
     * @param numComponent number of components per vertex
     * @param type of component
     * @param stride
     * @param offset
     */
    void linkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

    /**
     * @brief Binds the VAO to the current OpenGL context.
     */
    void bind();

    /**
     * @brief Unbinds the VAO from the current OpenGL context.
     */
    void unbind();

    /**
     * @brief Deletes the VAO and frees its resources.
     */
    void Delete();
};
