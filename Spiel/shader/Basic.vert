#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) uniform mat4 modelMatrix;
layout(location = 6) uniform mat4 viewProjectionMatrix;

out vec4 world_frag_pos;

void main() 
{
	world_frag_pos = modelMatrix * position;
	gl_Position = viewProjectionMatrix * modelMatrix * position;
}