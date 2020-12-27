#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;

in vec2 v_uv;

void main() 
{
	gl_FragColor = texture2D(samplerSlot, v_uv);
}