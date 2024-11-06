#version 420 core

// shader inputs
in vec2 texture_coords;

// shader outputs
layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D screen;

// screen image
//uniform sampler2D screen;

void main()
{
	color = vec4(texture(screen, texture_coords).rgb, 1.0f);

	//float depthValue = texture(depthMap, texture_coords).r;
	//color = vec4(vec3(depthValue), 1.0);
}