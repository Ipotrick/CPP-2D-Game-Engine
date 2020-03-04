#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) uniform mat4 modelViewProjectionMatrix;
layout(location = 2) uniform vec4 vertex_color;
out vec4 frag_color;
void main() 
{
	frag_color = vertex_color;
	gl_Position = modelViewProjectionMatrix * position;
}