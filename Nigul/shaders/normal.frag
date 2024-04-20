#version 460 core

// Outputs colors in RGBA
out vec4 FragColor;

// Imports the normal from the Vertex Shader
in vec3 Normal;

void main()
{
    // Normalize the normal to [0,1] since colors range from 0 to 1
    vec3 normalColor = normalize(Normal) * 0.5 + 0.5;
    FragColor = vec4(normalColor, 1.0);
}