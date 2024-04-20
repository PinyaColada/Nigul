#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float amountAberration;

uniform int sampleMSAA;

uniform bool isAberrationApplied;
uniform bool isToneMappingApplied;

vec3 chromaticAberration()
{
    float r = texture(screenTexture, TexCoords + amountAberration).r;
    float g = texture(screenTexture, TexCoords).g;
    float b = texture(screenTexture, TexCoords - amountAberration).b;
    return vec3(r, g, b);
}

float luminance(vec3 v)
{
    return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 reinhard_jodie(vec3 v)
{
    float l = luminance(v);
    vec3 tv = v / (1.0f + v);
    return mix(v / (1.0f + l), tv, tv);
}

vec3 gamma(vec3 c)
{
	return pow(c,vec3(1.0/2.2));
}

void main()
{
	vec3 color;
    if (isAberrationApplied)
	{
		color = chromaticAberration();
	} else {
        color = texture(screenTexture, TexCoords).rgb;
    }

    if (isToneMappingApplied){
        color = reinhard_jodie(color);
    }

    color.xyz = gamma(color.xyz);
    FragColor = vec4(color, 1.0);
}