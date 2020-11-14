#version 460 core

// uniforms:
layout(location = 50) uniform sampler2D texSampler[32];

struct ModelUniform {
	vec4 color;
	vec2 position;
	vec2 rotation;
	vec2 scale;
	int texId;
	int isCircle;
};

layout(std430, binding = 2) buffer ModelData {
	ModelUniform[] models;
};

// vertex output
layout(location = 3) in vec2 f_texCoord;
layout(location = 4) in vec2 f_circleCoord;
layout(location = 5) in int f_modelIndex;

out vec4 color;

void main() {
	color = vec4(1,0,0,1);//models[0].color;// * texture2D(texSampler[models[f_modelIndex].texId], f_texCoord);
}