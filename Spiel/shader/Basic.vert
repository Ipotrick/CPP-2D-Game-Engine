#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 3) uniform mat4 modelMatrix;
layout(location = 4) uniform mat4 viewProjectionMatrix;

out vec2 v_texCoord;
out vec4 world_frag_pos;

void main() 
{
	v_texCoord = texCoord;
	world_frag_pos = modelMatrix * position;
	gl_Position = viewProjectionMatrix * modelMatrix * position;
}