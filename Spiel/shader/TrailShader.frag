#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;
layout(location = 10) uniform float time;
layout(location = 11) uniform bool rainbowMode;

const float PI = 3.14;

in vec2 v_uv;

layout(location = 0) out vec4 o_color;

float upperCos(float f) {
	return (cos(f) + 1.0f)/2.0f;
}

void main() 
{
	vec4 cur = texture2D(samplerSlot, v_uv);
	if (rainbowMode) {
		vec3 rainbow_color = vec3(upperCos(time), upperCos(time + 1.0/3.0*2*PI), upperCos(time + 2.0/3.0*2*PI)); 
		o_color = vec4(rainbow_color, cur.w);
	} else {
		o_color = cur;
	}

}