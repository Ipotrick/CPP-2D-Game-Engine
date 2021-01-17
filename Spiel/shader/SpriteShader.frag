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
	int isCircle;
	/*
	 *	render space = 0 => world space
	 *  render space = 1 => window space
	 *	render space = 2 => uniform window space
	 *	render space = 3 => pixel space
	*/
	int renderSpace;
	int entityId;
};

layout(std430, binding = 2) buffer ModelData {
	ModelUniform[] models;
};

in vec2 v_texCoord;
in vec2 v_circleCoord;
flat in int v_modelID;

layout(location = 0) out vec4 o_color;

void main() 
{
	ModelUniform model = models[v_modelID];

	vec4 color = model.color;
	if (model.texId >= 0) {
		color *= texture2D(texSampler[model.texId], v_texCoord);
	}

	if ((color.a == 0.0f /* is fragment completely transparent */) || (model.isCircle == 1 && length(v_circleCoord) > 0.5f /* is fragment in circle and is fragment in circle mode */)) {
		discard;
	} else {
		o_color = color;
	}
}