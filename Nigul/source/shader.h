#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

/**
 * @brief Reads the entire content of a file into a string.
 *
 * @param filename The path to the file.
 * @return std::string The content of the file as a string.
 * @throw Throws an integer error code if file reading fails.
 */
std::string get_file_contents(const char* filename);

/**
 * @class Shader
 * @brief Represents an OpenGL shader program, encapsulating vertex and fragment shaders.
 */
class Shader
{
public:
    GLuint ID; ///< ID of the shader program

    /**
     * @brief Constructs a Shader object and creates a shader program from vertex and fragment shader files.
     *
     * @param vertexFile Path to the vertex shader file.
     * @param fragmentFile Path to the fragment shader file.
     */
    Shader(const char* vertexFile, const char* fragmentFile);

    /**
     * @brief Constructs computer shader.
     * 
     * @param computer shader file
     */
    Shader(const char* computerFile);

    /**
     * @brief Activates the shader program, making it the current one used by OpenGL.
     */
    void Activate();

    /**
     * @brief Deletes the shader program, freeing up resources.
     */
    void Delete();

    /**
     * @brief checks if the shaders have been compiled succesfully
     *
     * @param shader id
     * @param type of shader
     */
    void compileErrors(unsigned int shader, const char* type);

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setCubemap(const std::string &name, GLuint texture, GLuint slot) const;
};
