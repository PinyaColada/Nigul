#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

float near = 0.1; // Near plane distance
float far = 15.0; // Far plane distance

uniform sampler2D depthMap;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

void main()
{             
    float depth = texture(depthMap, TexCoords).r;
    depth = linearizeDepth(depth);
    FragColor = vec4(vec3(depth), 1.0);
}