#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) uniform mat4 modelMatrix;

void main() 
{
	gl_Position = modelMatrix * position;
}