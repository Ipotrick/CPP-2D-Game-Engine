#version 460 core

// in
layout(location = 0) in vec2 corner;
layout(location = 1) in vec2 uv;

// out
out vec2 v_uv; 

void main() 
{
	v_uv = uv;
	gl_Position = vec4(corner.x, corner.y, 0.0f, 1.0f);
}