#version 460 core

// function defines:
void pos_to_mat3(vec2 pos, out mat3 mat) {
	mat = mat3(1.0f);	// makes identity matrix
	mat[0][2] = pos.x;
	mat[1][2] = pos.y;
}

void scale_to_mat3(vec2 scale, out mat3 mat) {
	mat = mat3(1.0f);	// makes identity matrix
	mat[0][0] = scale.x;
	mat[1][1] = scale.y;
}

void rotavec_to_mat3(vec2 rotavec, out mat3 mat) {
	mat = mat3(1.0f);	// makes identity matrix
	mat[0][0] = rotavec.x;		// cos
	mat[0][1] = -rotavec.y;		// -sin
	mat[1][0] = rotavec.y;		// sin
	mat[1][1] = rotavec.x;		// cos
}

// uniforms:
layout(location = 0) uniform mat3 viewProj;

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

// vertex data:
layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 corner;
layout(location = 2) in int modelIndex;

// fragment out:
layout(location = 3) out vec2 f_texCoord;
layout(location = 4) out vec2 f_circlePos;
layout(location = 5) out int f_modelIndex;

void main() {
	f_texCoord = texCoord;
	f_circlePos = corner;		// the corner coordinates are the same as the circle coordinates
	f_modelIndex = modelIndex;
	
	mat3x3 translationMatrix;
	pos_to_mat3(models[modelIndex].position, translationMatrix);
	mat3x3 rotationMatrix;
	rotavec_to_mat3(models[modelIndex].rotation, rotationMatrix);
	mat3x3 scaleMatrix;
	scale_to_mat3(models[modelIndex].scale, scaleMatrix);
	
	mat3x3 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	
	vec3 position3;
	position3.xy = corner.xy;
	position3.z = 1.0f;
	
	vec4 position4;
	position4.xyz = viewProj * modelMatrix * position3;
	position4.w = 1.0f;

	gl_Position = position4;
}