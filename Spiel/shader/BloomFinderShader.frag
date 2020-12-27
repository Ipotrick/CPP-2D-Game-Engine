#version 460 core

layout(location = 50) uniform sampler2D samplerSlot;
layout(location = 10) uniform float sensitiviy;
layout(location = 11) uniform float minBrightness;

in vec2 v_uv;

void main() 
{
	vec4 val = texture2D(samplerSlot, v_uv);
	float brightness = max(max(val.x, val.y), val.z);
	brightness = pow(brightness, sensitiviy);
	brightness *= int(brightness > minBrightness);
	gl_FragColor = vec4(val.xyz, val.w * brightness);
}