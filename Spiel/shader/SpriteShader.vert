#version 460 core

// buffer defines:
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

vec4 rotate_vec4_z_axis(vec4 vec, in vec2 rota) {
	// rota.x = sin, rota.y = cos
	vec4 r;
	r[0] = vec.x * rota.x - vec.y * rota.y;
	r[1] = vec.x * rota.y + vec.y * rota.x;
	r[2] = vec[2];
	r[3] = vec[3];
	return r;
}

vec2 idToCorner(const int id, const vec2 minimum, const vec2 maximum)
{
	switch (id) {
	case 0:
		return vec2( minimum.x, maximum.y );	// tl
	case 1:
		return vec2( maximum.x, maximum.y );	// tr
	case 2:
		return vec2( minimum.x, minimum.y );	// bl
	case 3:
		return vec2( maximum.x, minimum.y );	// br
	default:
		return vec2( 0, 0 );
	}
}

// vertex in
layout(location = 0) in int cornerNr;
layout(location = 1) in int modelID;

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

	// THIS IS UGLY AS HELL, replace it with one uniform that contains all renderspaces:
	mat4 projectionViewMatrices[4];
	projectionViewMatrices[0] = worldSpaceVP;
	projectionViewMatrices[1] = mat4(1.0f);
	projectionViewMatrices[2] = uniformWindowSpaceVP;
	projectionViewMatrices[3] = pixelSpaceVP;

	ModelUniform model = models[modelID];
	
	vec2 corner = idToCorner(cornerNr, vec2(-0.5,-0.5), vec2(0.5, 0.5));
	vec2 texcoord = idToCorner(cornerNr, model.texmin, model.texmax);

	vec4 corner_coord = vec4(corner, 0, 1);
	vec4 scaled_corner_coord = corner_coord * vec4(model.scale.xy, 1, 1);
	vec4 rotated_corner_coord = rotate_vec4_z_axis(scaled_corner_coord, model.rotation);
	vec4 translated_corner_coord = rotated_corner_coord + vec4(model.position.xyz, 0);
	
	v_texCoord = texcoord;
	v_circleCoord = corner;
	v_modelID = modelID;
	gl_Position = projectionViewMatrices[model.renderSpace] * translated_corner_coord;
}