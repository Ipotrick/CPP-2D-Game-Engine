#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;

in vec2 v_uv;

layout(location = 0) out vec4 o_color;

void main() 
{
	o_color = texture2D(samplerSlot, v_uv);
}