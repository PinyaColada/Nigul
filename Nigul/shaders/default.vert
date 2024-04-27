#version 460 core

layout (location = 0) in vec3 aPos; // Positions/Coordinates
layout (location = 1) in vec3 aNormal; // Normals (not necessarily normalized)
layout (location = 2) in vec3 aColor; // Colors
layout (location = 3) in vec2 aTex; // Texture Coordinates

out vec3 crntPos; // Outputs the current position for the Fragment Shader
out vec3 Normal; // Outputs the normal for the Fragment Shader
out vec3 color; // Outputs the color for the Fragment Shader
out vec2 texCoord; // Outputs the texture coordinates to the Fragment Shader

uniform mat4 camMatrix; // Imports the camera matrix from the main function
uniform mat4 model;

#define MAX_LIGHTS 4
uniform mat4 lightProjectionMatrixes[MAX_LIGHTS];
out vec4 fragPositionLights[MAX_LIGHTS];

void main()
{
	crntPos = vec3(model * vec4(aPos, 1.0f));
	Normal = aNormal; // Assigns the normal from the Vertex Data to "Normal"
	color = aColor; // Assigns the colors from the Vertex Data to "color"
	texCoord = aTex; // Assigns the texture coordinates from the Vertex Data to "texCoord"
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		fragPositionLights[i] = lightProjectionMatrixes[i] * vec4(crntPos, 1.0);
	}
	
	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * vec4(crntPos, 1.0);
}