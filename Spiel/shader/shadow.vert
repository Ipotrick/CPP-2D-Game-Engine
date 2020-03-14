#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) uniform mat4 modelMatrix;
layout(location = 6) uniform mat4 viewProjectionMatrix;

void main() {
	gl_Position = viewProjectionMatrix * modelMatrix * position;
}