#version 460 core

// Outputs colors in RGBA
out vec4 FragColor; // Imports the current position from the Vertex Shader

in vec3 crntPos; 
in vec3 Normal; 
in vec3 color; 
in vec2 texCoord;

// Gets the Texture Units from the main function
uniform sampler2D albedo;
uniform sampler2D metallicRoughness; 
uniform sampler2D emissive;
uniform sampler2D normalMap;
uniform sampler2D occlusion;

uniform samplerCube skybox;

// Bools to know if the texture is being used
uniform bool hasColorTexture;
uniform bool hasMetallicRoughnessTexture;
uniform bool hasEmissiveTexture;
uniform bool hasNormalTexture;
uniform bool hasOcclusionTexture;
uniform bool hasSkybox;

// aux variables
int specularPower = 8; // for specular calculations
float specularIntensity = 0.5f; // for specular calculations

// factor for PBR
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec4 baseColorFactor;

// Scene uniforms
uniform vec3 camPos;

// Ambient light
uniform float ambientLight;
uniform vec3 ambientColor;
uniform float shadowDarkness;
uniform float reflectionFactor;

// Light singlepass
#define MAX_LIGHTS 10

// Math constants
#define RECIPROCAL_PI 0.3183098861837697
#define PI 3.141592653589793

// For lights and shadows
uniform int numLights;
uniform vec3 lightColors[MAX_LIGHTS];
uniform int lightTypes[MAX_LIGHTS];
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightDirections[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];
uniform float lightRanges[MAX_LIGHTS];
uniform float lightAttenuations[MAX_LIGHTS];
uniform float lightInnerConeAngles[MAX_LIGHTS];
uniform float lightOuterConeAngles[MAX_LIGHTS];
uniform sampler2D lightShadowMapSamples[MAX_LIGHTS];
in vec4 fragPositionLights[MAX_LIGHTS];
uniform float lightShadowBiases[MAX_LIGHTS];

// Gamma functions
vec3 degamma(vec3 c)
{
	return pow(c,vec3(2.2));
}

// PBR functions
// Fresnel and colorized fresnel
vec3 F_Schlick(float VoH, vec3 f0)
{
	float f = pow(1.0 - VoH, 5.0);
	return f0 + (vec3(1.0) - f0) * f;
}

float F_Schlick(float VoH, float f0)
{
	float f = pow(1.0 - VoH, 5.0);
	return f0 + (1.0 - f0) * f;
}

// GGX distribution
float D_GGX (float NoH, float linearRoughness)
{
	float a2 = linearRoughness * linearRoughness;
	float f = (NoH * NoH) * (a2 - 1.0) + 1.0;
	return a2 / (PI * f * f);
}

// Smith's shadowing-masking function and GGX
float GGX(float NdotV, float k){
	return NdotV / (NdotV * (1.0 - k) + k);
}
	
float G_Smith( float NdotV, float NdotL, float roughness)
{
	float k = pow(roughness + 1.0, 2.0) / 8.0;
	return GGX(NdotL, k) * GGX(NdotV, k);
}


// Burley diffuse
float diffuseBurley( float NoV, float NoL, float LoH, float roughness)
{
        float f90 = 0.5 + 2.0 * roughness * roughness * LoH * LoH;
        float lightScatter = F_Schlick(NoL, f90);
        float viewScatter  = F_Schlick(NoV, f90);
        return lightScatter * viewScatter * RECIPROCAL_PI;
}

// BRDF specular
vec3 specularBRDF( float roughness, vec3 f0, float NoH, float NoV, float NoL, float LoH ){
	float a = roughness * roughness;

	// Normal Distribution Function
	float D = D_GGX( NoH, a );

	// Fresnel Function
	vec3 F = F_Schlick( LoH, f0 );

	// Visibility Function (shadowing/masking)
	float G = G_Smith( NoV, NoL, roughness );
		
	// Norm factor
	vec3 spec = D * G * F;
	spec /= (4.0 * NoL * NoV + 1e-6);

	return spec;
}

float computeShadow(int index, vec3 n, vec3 l){
	float shadow = 0.0;
	vec4 fragPosLight = fragPositionLights[index];
	vec3 fragPos = fragPosLight.xyz / fragPosLight.w;
	if (fragPos.z > 1.0)
		return 1.0;
	fragPos = fragPos * 0.5 + 0.5;
	float currentDepth = fragPos.z;
	float bias = mix(lightShadowBiases[index], 0.0, dot(n, -l));
	// bias = max(bias * (1.0f - dot(n, l)), 0.000001f);

	// Smooth the shadow
	int sampleRadius = 4;
	vec2 pixelSize = 1.0 / textureSize(lightShadowMapSamples[index], 0);
	for (int x = -sampleRadius; x <= sampleRadius; ++x) {
		for (int y = -sampleRadius; y <= sampleRadius; ++y) {
			vec2 offset = vec2(x, y) * pixelSize;
			float closestDepth = texture(lightShadowMapSamples[index], fragPos.xy + offset).r;
			if (currentDepth > (closestDepth + bias)) {
				shadow += 1.0f * shadowDarkness;
			}
		}
	}
	shadow /= pow((sampleRadius * 2 + 1), 2);
	return 1 - shadow;
}

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );
	
	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

vec3 perturbNormal(vec3 N, vec3 WP, vec2 uv, vec3 normal_pixel)
{
	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, WP, uv);
	return normalize(TBN * normal_pixel);
}

vec3 pointLight(int index, vec4 baseColor)
{	
	// intensity of light with respect to distance
	float dist = length(lightPositions[index] - crntPos);
	float inten = 1.0f / (lightAttenuations[index] * dist * dist + 1.0f);

	// PBR values
	baseColor *= baseColorFactor;
	float metalness = metallicFactor;
	float roughness = roughnessFactor;
	if (hasMetallicRoughnessTexture) {
		metalness = metalness * texture(metallicRoughness, texCoord).b;
		roughness = roughness * texture(metallicRoughness, texCoord).g;
	} 

	// Light equation vectors
	vec3 n = hasNormalTexture ? perturbNormal(Normal, crntPos, texCoord, texture(normalMap, texCoord).xyz) : normalize(Normal);
	vec3 l = normalize(lightPositions[index] - crntPos);
	vec3 v = normalize(camPos - crntPos);
	vec3 h = normalize(l + v);

	float NoL = max(dot(n, l), 0.0);
	float NoV = max(dot(n, v), 0.0);
	float NoH = max(dot(n, h), 0.0);
	float LoH = max(dot(l, h), 0.0);

	// diffuse lighting
	vec3 diffuseColor = (1.0 - metalness) * baseColor.xyz;
	vec3 diffuse = diffuseColor * diffuseBurley(NoV, NoL, LoH, roughness); 

	// specular lighting
	vec3 f0 = mix(vec3(0.5), baseColor.xyz, metalness);
	vec3 specular = specularBRDF(roughness, f0, NoH, NoV, NoL, LoH);

	// light params
	vec3 lightParams = lightColors[index] * lightIntensities[index] * inten;

	// final light color
	vec3 light = (diffuse + specular) * lightParams;
	return light;
}

vec3 directLight(int index, vec4 baseColor)
{
	// PBR values
	baseColor *= baseColorFactor;
	float metalness = metallicFactor;
	float roughness = roughnessFactor;
	if (hasMetallicRoughnessTexture) {
		metalness = metalness * texture(metallicRoughness, texCoord).b;
		roughness = roughness * texture(metallicRoughness, texCoord).g;
	} 

	// Light equation vectors
	vec3 n = hasNormalTexture ? perturbNormal(Normal, crntPos, texCoord, texture(normalMap, texCoord).xyz) : normalize(Normal);
	vec3 l = normalize(lightPositions[index]);
	vec3 v = normalize(camPos - crntPos);
	vec3 h = normalize(l + v);
	vec3 r = reflect(v, n);

	float NoL = max(dot(n, l), 0.0);
	float NoV = max(dot(n, v), 0.0);
	float NoH = max(dot(n, h), 0.0);
	float LoH = max(dot(l, h), 0.0);

	// diffuse lighting
	vec3 diffuseColor = (1.0 - metalness) * baseColor.xyz;
	vec3 diffuse = diffuseColor * diffuseBurley(NoV, NoL, LoH, roughness); 

	// specular lighting
	vec3 f0 = mix(vec3(0.5), baseColor.xyz, metalness);
	vec3 specular = specularBRDF(roughness, f0, NoH, NoV, NoL, LoH);

	// light params
	vec3 lightParams = lightColors[index] * lightIntensities[index];

	float occlusionValue = 1.0;
	if (hasOcclusionTexture) {
		occlusionValue = texture(occlusion, texCoord).r;
	}

	// compute shadow
	float shadow = computeShadow(index, n, l);

	// final light color
	vec3 light = (diffuse + specular) * lightParams * shadow * occlusionValue;

	if (hasSkybox) {
		float mipLevel = roughness * 4.0;
		vec3 reflection = textureLod(skybox, r, mipLevel).rgb;
		reflection = degamma(reflection);
		light = mix(light, reflection, metalness * reflectionFactor);
	}

	return light;
}

vec3 spotLight(int index, vec4 baseColor)
{
	// controls how big the area that is lit up is
	float outerCone = lightOuterConeAngles[index];
	float innerCone = lightInnerConeAngles[index];

	// PBR values
	baseColor *= baseColorFactor;
	float metalness = metallicFactor;
	float roughness = roughnessFactor;
	if (hasMetallicRoughnessTexture) {
		metalness = metalness * texture(metallicRoughness, texCoord).b;
		roughness = roughness * texture(metallicRoughness, texCoord).g;
	} 

	// Light equation vectors
	vec3 n = hasNormalTexture ? perturbNormal(Normal, crntPos, texCoord, texture(normalMap, texCoord).xyz) : normalize(Normal);
	vec3 l = normalize(lightPositions[index] - crntPos);
	vec3 v = normalize(camPos - crntPos);
	vec3 h = normalize(l + v);

	float NoL = max(dot(n, l), 0.0);
	float NoV = max(dot(n, v), 0.0);
	float NoH = max(dot(n, h), 0.0);
	float LoH = max(dot(l, h), 0.0);

	// diffuse lighting
	vec3 diffuseColor = (1.0 - metalness) * baseColor.xyz;
	vec3 diffuse = diffuseColor * diffuseBurley(NoV, NoL, LoH, roughness); 

	// specular lighting
	vec3 f0 = mix(vec3(0.5f), baseColor.xyz, metalness);
	vec3 specular = specularBRDF(roughness, f0, NoH, NoV, NoL, LoH);

	// calculates the intensity of the crntPos based on its angle to the center of the light cone
	float angle = dot(lightDirections[index], l);
	float inten = 1 - clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

	// light params
	vec3 lightParams = lightColors[index] * lightIntensities[index] * inten;

	// final light color
	vec3 light = (diffuse + specular) * lightParams;
	return light;
}

void main()
{
	// outputs final color
	vec4 color = vec4(0.0f);
	if (hasColorTexture){
		color = texture(albedo, texCoord);
		color.xyz = degamma(color.xyz); // Apply degamma to the input color
	} 

	vec3 light = ambientLight * ambientColor;
	for (int i = 0; i < numLights; i++) {
		switch (lightTypes[i]) {
			case 0:
				light += pointLight(i, color);
				break;
			case 1:
				light += spotLight(i, color);
				break;
			case 2:
				light += directLight(i, color);
				break;
			default:
				break;
		}
	}

	if (hasEmissiveTexture) {
        vec3 emissiveColor = texture(emissive, texCoord).rgb;
		emissiveColor = degamma(emissiveColor);
        light += emissiveColor;
    }
	color = vec4(light * color.rgb, color.a);
	FragColor = color;
}