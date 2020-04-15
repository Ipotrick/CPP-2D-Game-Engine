#version 460 core

layout(location = 5) uniform vec4 frag_color;
layout(location = 6) uniform int circle_mode;
layout(location = 7) uniform vec2 world_center_pos;
layout(location = 8) uniform float rad;
layout(location = 10) uniform sampler2D texSampler;

in vec4 world_frag_pos;
in vec2 v_texCoord;

layout (location = 0)out vec4 color;

void main() 
{
	if (circle_mode == 1) {
		float len = length(world_frag_pos.xy - world_center_pos);
		if (len < rad) {
			color = frag_color * texture2D(texSampler, v_texCoord);
		}
		else {
			color = vec4(1,0,1,0);
		}
	}
	else{
		color = frag_color * texture2D(texSampler, v_texCoord);
	}
}