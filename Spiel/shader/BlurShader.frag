#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;
layout(location = 51) uniform int hor;
layout(location = 52) uniform int screenWidth;
layout(location = 53) uniform int screenHeight;
layout(location = 54) uniform int blurStepSize;
layout(location = 55) uniform int blurSteps;

in vec2 v_uv;

float getWeight(int index) {
	return float(blurSteps-index)/float(blurSteps);
}

void main() 
{
	vec4 accColor = vec4(0,0,0,0);
	float accWeights = 0.0f;

	if (hor != 0) {
		for (int x = -blurSteps+1; x < blurSteps; x++) {
			const float weight = getWeight(abs(x));
			accColor += texture2D(samplerSlot, v_uv + vec2(x/float(screenWidth)*blurStepSize,0)) * weight;
			accWeights += weight;
		}
	}
	else {
		for (int y = -blurSteps+1; y < blurSteps; y++) {
			const float weight = getWeight(abs(y));
			accColor += texture2D(samplerSlot, v_uv + vec2(0,y/float(screenHeight)*blurStepSize)) * weight;
			accWeights += weight;
		}
	}

	accColor /= accWeights;
	accColor.a = 1.0f;

	gl_FragColor = accColor;
}