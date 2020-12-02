#version 460 core

layout(location = 1) uniform mat4 viewProj;

in vec2 v_uv;

layout(location = 0) out vec4 o_color;


const int MAX_INTERATIONS = 100;

float upperCos(float c) {
	return (cos(c) + 1) * 0.5f;
}

const float PI = 3.14;

vec4 iterToColor(int i) {
	float alt_i = i / 100.0 * 30;
	return vec4(
		upperCos(alt_i),				//r
		upperCos(1.1*alt_i),		//g
		upperCos(1.2*alt_i),		//b
		1
	);
}

vec2 squareComplex(vec2 z) {
	return vec2(
		pow(z.x,2) - pow(z.y,2),
		2*z.x*z.y
	);
}

vec4 mandelFunc(vec2 c) {
	vec2 z0 = vec2(0,0);
	vec2 z = z0;
	int iter = 0;
	for (; iter < MAX_INTERATIONS; iter++) {
		z = squareComplex(z) + c;
		float lenZ = length(z);
		if (lenZ > 2.5) {
			return iterToColor(iter);
		}
	}
	return vec4(0,0,0,1); 
}

void main() 
{
	vec2 winCoord = v_uv * 2 - vec2(1);
	o_color = mandelFunc((viewProj * vec4(winCoord,0,1)).xy);
}