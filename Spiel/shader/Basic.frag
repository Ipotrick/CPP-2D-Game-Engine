#version 430 core

in vec4 world_frag_pos;

layout(location = 2) uniform vec4 frag_color;
layout(location = 3) uniform int circle_mode;
layout(location = 4) uniform vec2 world_center_pos;
layout(location = 5) uniform float rad;


out vec4 color;

void main() 
{
	if (circle_mode == 1) {
		float len = length(world_frag_pos.xy - world_center_pos);
		if (len < rad) {
			color = frag_color;
		}
		else {
			color = vec4(0,0,0,0);
		}
	}
	else{
		color = frag_color;
	}
}