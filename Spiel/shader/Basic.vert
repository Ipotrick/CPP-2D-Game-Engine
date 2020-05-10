#version 460 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float texID;
layout(location = 4) in float isCirlce;

out vec2 v_texCoord;
out vec4 a_color;
out float a_texID;
out float a_isCircle;

void main() 
{
	v_texCoord = texCoord;
	a_color = color;
	a_texID = texID;
	a_isCircle = isCirlce;
	gl_Position =  position;
}