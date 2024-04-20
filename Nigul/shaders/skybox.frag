#version 460 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;

vec3 degamma(vec3 c)
{
	return pow(c,vec3(2.2));
}

void main()
{   
	vec4 skyboxColor = texture(skybox, texCoords);
	skyboxColor.rgb = degamma(skyboxColor.rgb);
    FragColor = skyboxColor;
}