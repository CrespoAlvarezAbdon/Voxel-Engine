#version 420 core

// shader inputs
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

// shader outputs
out vec2 texture_coords;

void main()
{
	texture_coords = uv;

	gl_Position = vec4(position, 0.0f, 1.0f);
}