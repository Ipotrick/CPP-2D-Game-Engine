#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;

in vec2 v_uv;
out vec4 fragColor;

void main() 
{
	fragColor = texture2D(samplerSlot, v_uv);
}