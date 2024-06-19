in vec2 TexCoords;

uniform sampler2D cameraDepth;
uniform sampler2D cameraNormal;

uniform mat4 inverseViewProjection;
uniform mat4 viewProjection;
uniform vec2 pixelSize;
uniform float radius;
uniform vec3 samples[64];

void main()
{
    vec3 normal = texture(cameraNormal, TexCoords).xyz;
    float depth = texture(cameraDepth, TexCoords).r;

    // We want to center the sample in the center of the pixel
    vec2 uv = TexCoords - pixelSize * 0.5;
    
    // ignore pixels in the background
    if (depth == 1.0)
	{
        FragColor = vec4(1.0);
		discard;
	}

    // create sceenpos with the right depth
    vec4 screenPos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    // reproject
    vec4 viewPos = inverseViewProjection * screenPos;
    viewPos /= viewPos.w;

    // lets use 64 samples
    const int sampleCount = 64;
    int num = samples;

    for ( int i = 0; i < sampleCount; i++)
	{
		vec3 sample = samples[i];
		vec3 samplePos = viewPos.xyz + sample * radius;

		vec4 sampleScreenPos = viewProjection * vec4(samplePos, 1.0);
		sampleScreenPos /= sampleScreenPos.w;

		vec2 sampleUV = sampleScreenPos.xy * 0.5 + 0.5;

		vec3 sampleNormal = texture(cameraNormal, sampleUV).xyz;
		float sampleDepth = texture(cameraDepth, sampleUV).r;

		// If its outside the normal, we flip it
		if (dot(normal, sampleNormal) < 0.0)
			sample *= -1.0;

		// ignore pixels that are too far away
		if (abs(depth - sampleDepth) > 0.1)
			continue;

		num++;
	})

	float ao = 1.0 - float(num) / float(sampleCount);
    FragColor.r = ao;
}