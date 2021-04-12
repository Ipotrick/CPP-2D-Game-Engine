#version 460 core

layout(location = 50) uniform sampler2D texSampler[32];

layout (location = 4) uniform uint screenWidth;
layout (location = 5) uniform uint screenHeight;
layout (location = 6) uniform float mtsdf_smoothing;
layout (location = 7) uniform float mtsdfBarrier;

in vec4 v_color;
in vec2 v_size;
in float v_cornerRounding;
in flat int v_isMSDF;
in vec2 v_texCoord;
in flat int v_texSamplerIndex;
in vec2 v_relativeCoord;
in float v_relative_fragment_size;
in vec2 v_clipMin;
in vec2 v_clipMax;

layout(location = 0) out vec4 o_color;


float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() 
{
	vec4 color = v_color;
	const bool isMSDF = (v_isMSDF & 1) != 0;

	if (isMSDF) {
		const vec2 samplingOffsets = vec2(0.25f,0.25f) / textureSize(texSampler[v_texSamplerIndex], 0);
		const float smoothing = mtsdf_smoothing / v_relative_fragment_size;					
		const vec2[4] offsets = {vec2(-1,-1),vec2(-1,1),vec2(1,-1),vec2(1,1)};

		float value = 0.0f;

		for (int i = 0; i < 4; i++) {
			vec4 mtsd = texture(texSampler[v_texSamplerIndex], v_texCoord + offsets[i] * samplingOffsets);
			value = max(value, median(mtsd.r, mtsd.g, mtsd.b));
		}
		color.a = smoothstep(0.5f - smoothing - mtsdfBarrier,0.5f + smoothing - mtsdfBarrier, value);
	}
	else {
		if (v_texSamplerIndex >= 0) { color *= texture2D(texSampler[v_texSamplerIndex], v_texCoord); }
		
		if (v_cornerRounding > 0.0f) {
			const vec2 model_coord = abs(v_size * v_relativeCoord);
			const float rounding_radius = max(v_cornerRounding * 2.0f, 0.00000001f);		// max cause rounding_radius must be >0 for of smoothstep stability
			const vec2 inner_size = v_size - rounding_radius;
			const vec2 clamped_inner_coord = clamp(model_coord, vec2(0,0), inner_size);
			const float distance_to_inner_rect = length(model_coord - clamped_inner_coord);
			color.a *= 1.0f - step(rounding_radius, distance_to_inner_rect);
		}
	}

	const vec2 relativeFragCoord = gl_FragCoord.xy / vec2(screenWidth, screenHeight) * 2 - vec2(1,1);
	const float insideClipRange = 
		step(v_clipMin.x, relativeFragCoord.x) * 
		step(v_clipMin.y, relativeFragCoord.y) *
		(1.0f - step(v_clipMax.x, relativeFragCoord.x)) * 
		(1.0f - step(v_clipMax.y, relativeFragCoord.y));
	color.a *= insideClipRange;

	if (color.a < 0.05f) {
		discard;
	} else {
		o_color = color;
	}
}