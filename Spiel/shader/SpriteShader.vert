#version 460 core


void rotate_vec2(inout vec2 vec, in vec2 rota) {
	vec2 old = vec;
	vec[0] = old.x * rota.x - old.y * rota.y;
	vec[1] = old.x * rota.y + old.y * rota.x;
}

// buffer defines:
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

// vertex in
layout(location = 0) in vec2 corner;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in int modelID;

// fragment out
out vec2 v_texCoord;
out vec2 v_circleCoord;
flat out int v_modelID;

void main() 
{
	v_texCoord = texCoord;
	v_modelID = modelID;

	ModelUniform model = models[modelID];

	vec2 pos2 = corner;
	rotate_vec2(pos2, model.rotation);

	vec3 pos3;
	pos3.xy = (pos2 * model.scale) + model.position;
	pos3.z = 0.0f;

	vec4 pos4;
	pos4.xyz = pos3.xyz;
	pos4.w = 1.0f;
	pos4 = model.viewProj * pos4;

	gl_Position = pos4;
}