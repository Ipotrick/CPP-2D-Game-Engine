#version 460 core

layout(location = 50) uniform sampler2D texSampler[32];

struct ModelUniform {
	vec4 color;
	vec4 position;
	vec2 rotation;
	vec2 scale;
	vec2 texmin;
	vec2 texmax;
	int texId;
	int isMSDF;
	float cornerRounding;
	/*
	 *	render space = 0 => world space
	 *  render space = 1 => window space
	 *	render space = 2 => uniform window space
	 *	render space = 3 => pixel space
	*/
	int renderSpace;
};

layout(std430, binding = 2) buffer ModelData {
	ModelUniform[] models;
};

layout (location = 6) uniform float mtsdf_smoothing;

in vec2 v_texCoord;
in vec2 v_relativeCoord;
flat in int v_modelID;
in float v_relative_fragment_size;

layout(location = 0) out vec4 o_color;

const float EDGE_SMOOTHING = 0.05f;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() 
{
	const ModelUniform model = models[v_modelID];
	vec4 color = model.color;
	const bool isMSDF = (model.isMSDF & 1) != 0;

	if (!isMSDF) {
		if (model.texId >= 0) { color *= texture2D(texSampler[model.texId], v_texCoord); }
		
		if (model.cornerRounding > 0.0f) {
			const vec2 model_coord = abs(model.scale * v_relativeCoord);
			const float rounding_radius = max(model.cornerRounding * 2.0f, 0.00000001f);		// max cause rounding_radius must be >0 for of smoothstep stability
			const vec2 inner_size = model.scale - rounding_radius;
			const vec2 clamped_inner_coord = clamp(model_coord, vec2(0,0), inner_size);
			const float distance_to_inner_rect = length(model_coord - clamped_inner_coord);
			color.a *= smoothstep(rounding_radius * (1.0f + EDGE_SMOOTHING * 0.5f), rounding_radius * (1.0f - EDGE_SMOOTHING * 0.5f), distance_to_inner_rect);
		}
	}
	else {
		vec4 mtsd = texture(texSampler[model.texId], v_texCoord);
		const float median_msd = mtsd.a * 0.25f + median(mtsd.r, mtsd.g, mtsd.b) * 0.75f;
		const float smoothing = mtsdf_smoothing / v_relative_fragment_size;					
		color.a = smoothstep(0.5f - smoothing,0.5f + smoothing, median_msd);
	}

	if (color.a < 0.05f) {
		discard;
	} else {
		o_color = color;
	}
}