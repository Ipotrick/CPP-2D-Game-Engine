#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;
layout(location = 10) uniform float strength;
layout(location = 11) uniform int stage;
layout(location = 12) uniform int kernelWidth;

in vec2 v_uv;

float turboGauss(float x) {
	return 1.0f - min(x*x*16,sqrt(x));
}
	
void main() 
{
	int kernelHalfWidth = kernelWidth/2;
	vec2 pixelSize = vec2(1,1) / textureSize(samplerSlot,0); /* we can go bigger steps than 1 because we are using the texture hardware interpolation */

	vec3 average = vec3(0.0f);

	if (stage == 0) {
		for (int i = 0; i < kernelHalfWidth; i++) {
			float place = float(i) / float(kernelHalfWidth);

			float weight = turboGauss(place);// (1.0f-pow(place,0.5));
			vec4 texA = texture2D(samplerSlot, v_uv + vec2(pixelSize.x * i, 0));
			average += texA.xyz * weight;
			vec4 texB = texture2D(samplerSlot, v_uv + vec2(-pixelSize.x * i, 0));
			average += texB.xyz * weight;
		}
	} else {
		for (int i = 0; i < kernelHalfWidth; i++) {
			float place = float(i) / float(kernelHalfWidth);

			float weight = turboGauss(place);// (1.0f-pow(place,0.5));
			vec4 texA = texture2D(samplerSlot, v_uv + vec2(0, pixelSize.y * i));
			average += texA.xyz * weight;
			vec4 texB = texture2D(samplerSlot, v_uv + vec2(0, -pixelSize.y) * i);
			average += texB.xyz * weight;
		}
	}

	average /= float(kernelHalfWidth);
	average *= strength * 1.5;

	gl_FragColor = vec4(average.xyz, 1.0f);
}