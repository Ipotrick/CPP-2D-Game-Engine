#version 460 core

layout(location = 50) uniform sampler2D texSampler[32];

in vec2 v_texCoord;
in vec2 v_circleCoord;
flat in int v_modelID;

layout (location = 0) out vec4 color;

struct ModelUniform {
	vec4 color;
	vec2 position;
	vec2 rotation;
	vec2 scale;
	int texId;
	int isCircle;
	mat4 viewProj;
};

layout(std430, binding = 2) buffer ModelData {
	ModelUniform[] models;
};

void main() 
{
	ModelUniform model = models[v_modelID];
	if (model.isCircle == 1) {
		vec2 relativePosToCenter = vec2(v_texCoord.x * 2 -1, v_texCoord.y * 2 -1);
		float relativeRadiusToCenter = length(relativePosToCenter);
		if (relativeRadiusToCenter < 1.0f) {
			//inner part of circle
			if (model.texId >= 0) {
				color = model.color * texture2D(texSampler[model.texId], v_texCoord);
			}
			else {
				color = model.color;
			}
		}
		else{
			// corner/outer part of circle
			color = vec4(0,0,0,0);
		}
	}
	else{
		if (model.texId >= 0) {
			color = model.color * texture2D(texSampler[model.texId], v_texCoord);
		}
		else { 
			color = model.color;
		}
	}
}