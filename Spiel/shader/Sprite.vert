#version 460 core

vec4 rotate_vec4_z_axis(vec4 vec, in vec2 rota) {
	// rota.x = sin, rota.y = cos
	vec4 r;
	r[0] = vec.x * rota.x - vec.y * rota.y;
	r[1] = vec.x * rota.y + vec.y * rota.x;
	r[2] = vec[2];
	r[3] = vec[3];
	return r;
}

// vertex in
layout(location = 0) in vec2 a_corner;
layout(location = 1) in vec2 a_uv;

// instance in:
layout(location = 2) in vec4	a_color;
layout(location = 3) in vec4	a_position;
layout(location = 4) in vec2	a_rotation;
layout(location = 5) in vec2	a_size;
layout(location = 6) in vec2	a_texMin;
layout(location = 7) in vec2	a_texMax;
layout(location = 8) in vec2	a_clipMin;
layout(location = 9) in vec2	a_clipMax;
layout(location = 10) in int	a_texSamplerIndex;
layout(location = 11) in int	a_isMSDF;
layout(location = 12) in float	a_cornerRounding;
layout(location = 13) in int	a_renderSpace;


// fragment out
out vec4 v_color;
out vec2 v_size;
out float v_cornerRounding;
out flat int v_isMSDF;
out vec2 v_texCoord;
out flat int v_texSamplerIndex;
out vec2 v_relativeCoord;
out float v_relative_fragment_size;
out vec2 v_clipMin;
out vec2 v_clipMax;

// uniforms:
layout (location = 0) uniform mat4 worldSpaceVP;
// window space has no uniform as it is the identity matrix
layout (location = 2) uniform mat4 uniformWindowSpaceVP;
layout (location = 3) uniform mat4 pixelSpaceVP;
layout (location = 4) uniform uint screenWidth;
layout (location = 5) uniform uint screenHeight;
layout (location = 6) uniform float mtsdf_smoothing;

void main() 
{
	mat4 projectionViewMatrices[4];
	projectionViewMatrices[0] = worldSpaceVP;
	projectionViewMatrices[1] = mat4(1.0f);
	projectionViewMatrices[2] = uniformWindowSpaceVP;
	projectionViewMatrices[3] = pixelSpaceVP;

	vec2 texcoord;
	texcoord.x = a_texMin.x * (1.0f - a_uv.x) + a_texMax.x * (a_uv.x);
	texcoord.y = a_texMin.y * (1.0f - a_uv.y) + a_texMax.y * (a_uv.y);

	vec4 corner_coord = vec4(a_corner, 0, 1);
	vec4 scaled_corner_coord = corner_coord * vec4(a_size, 1, 1);
	vec4 rotated_corner_coord = rotate_vec4_z_axis(scaled_corner_coord, a_rotation);
	vec4 translated_corner_coord = rotated_corner_coord + vec4(a_position.xyz, 0);
	vec4 windowspace_coord = projectionViewMatrices[a_renderSpace] * translated_corner_coord;

	vec4 center_coord = vec4(0,0,0,1);
	vec4 translated_center_coord = center_coord + vec4(a_position.xyz, 0);
	vec4 windowspace_center_coord = projectionViewMatrices[a_renderSpace] * translated_center_coord;

	v_color = a_color;
	v_size = a_size;
	v_cornerRounding = a_cornerRounding;
	v_isMSDF = a_isMSDF;
	v_texCoord = texcoord;
	v_texSamplerIndex = a_texSamplerIndex;
	v_relative_fragment_size = length(vec2(windowspace_center_coord.xy - windowspace_coord.xy) * vec2(screenWidth, screenHeight));
	v_relativeCoord = a_corner * 2.0f;
	v_clipMin = a_clipMin;
	v_clipMax = a_clipMax;
	gl_Position = windowspace_coord;
}