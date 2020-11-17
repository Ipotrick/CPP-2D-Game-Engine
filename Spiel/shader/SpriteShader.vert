#version 460 core

void rotate_vec2(inout vec2 vec, in vec2 rota) {
	// rota.x = sin, rota.y = cos
	vec2 old = vec;
	vec[0] = old.x * rota.y - old.y * rota.x;
	vec[1] = old.x * rota.x + old.y * rota.y;
}

// buffer defines:
struct ModelUniform {
	vec4 color;
	vec2 position;
	vec2 rotation;
	vec2 scale;
	int texId;
	int isCircle;
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

// vertex in
layout(location = 0) in vec2 corner;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in int modelID;

// fragment out
out vec2 v_texCoord;
out vec2 v_circleCoord;
/*
 * the flat keyword prevents the rasteriser interpolating the modelID, 
 * without it the shader wont work as interpolating integers is illegal on nvidia hardware
*/
flat out int v_modelID;	 

// uniforms:
layout (location = 0) uniform mat4 worldSpaceVP;
// window space has no uniform as it is the identity matrix
layout (location = 2) uniform mat4 uniformWindowSpaceVP;
layout (location = 3) uniform mat4 pixelSpaceVP;

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

	mat4 projectionViewMatrices[4];
	projectionViewMatrices[0] = worldSpaceVP;
	projectionViewMatrices[1] = mat4(1.0f);
	projectionViewMatrices[2] = uniformWindowSpaceVP;
	projectionViewMatrices[3] = pixelSpaceVP;

	vec4 pos4;
	pos4.xyz = pos3.xyz;
	pos4.w = 1.0f;
	pos4 = projectionViewMatrices[model.renderSpace] * pos4;

	gl_Position = pos4;
}