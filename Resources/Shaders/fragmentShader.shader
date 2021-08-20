#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord; // This is an input varying

uniform sampler2D u_Texture;

void main()
{

	color = texture(u_Texture, v_TexCoord) * vec4(1, 1, 1, 1); // Texture sampling * ambient light color

};