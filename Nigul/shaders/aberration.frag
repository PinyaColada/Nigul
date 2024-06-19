#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform bool isAberrationApplied;

uniform float aberration;


void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    if (isAberrationApplied) {
		vec3 red = texture(screenTexture, TexCoords + aberration).rgb;
		vec3 green = texture(screenTexture, TexCoords).rgb;
		vec3 blue = texture(screenTexture, TexCoords - aberration).rgb;

		color = vec3(red.r, green.g, blue.b);
    }

    FragColor.rgb = color;
}