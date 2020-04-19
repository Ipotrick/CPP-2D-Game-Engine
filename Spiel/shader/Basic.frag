#version 460 core

layout(location = 50) uniform sampler2D texSampler[32];

in vec2 v_texCoord;
in vec4 a_color;
in float a_texID;
in float a_isCircle;

layout (location = 0) out vec4 color;

void main() 
{
	int texID = int(a_texID);
	int isCircle = int(a_isCircle);
	if (isCircle == 1) {
		vec2 relativePosToCenter = vec2(v_texCoord.x * 2 -1, v_texCoord.y * 2 -1);
		float relativeRadiusToCenter = length(relativePosToCenter);
		if (relativeRadiusToCenter < 1.0f) {
			//inner part of circle
			if (a_texID >= 0) {
				color = a_color * texture2D(texSampler[texID], v_texCoord);
			}
			else {
				color = a_color;
			}
		}
		else{
			// corner/outer part of circle
			color = vec4(0,0,0,0);
		}
	}
	else{
		if (a_texID >= 0) {
			color = a_color * texture2D(texSampler[texID], v_texCoord);
		}
		else { 
			color = a_color;
		}
	}
}