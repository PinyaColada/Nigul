#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform bool isToneMappingApplied;

uniform float exposure;

vec3 gamma(vec3 c)
{
	return pow(c,vec3(1.0/2.2));
}

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // We map the color to the [0, 1] range using the exposure value
    if (isToneMappingApplied){
        color = vec3(1.0) - exp(-color * exposure);
    }

    FragColor.rgb = gamma(color.xyz);
}