#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;
layout(location = 10) uniform float alpha;
layout(location = 11) uniform bool override_alpha;

in vec2 v_uv;

layout(location = 0) out vec4 o_color;

void main() 
{
	vec4 color = texture2D(samplerSlot, v_uv);
	o_color = vec4(color.xyz, override_alpha ? alpha : color.w);
}