#version 330 core

// Example of use of location
// If the vertex structure is
//struct vertex
//{
//
//	float positions[3];
//	float textureCoords[2];
//
//};
// Then location = 0 refers to float positions[3] and
// location = 1 refers to float textureCoords[2]
layout(location = 0) in vec4 position; // Vertices' positions
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord; // This is an output varying

uniform mat4 u_MVP; // u_MVP stands for u_Model_view_projection_matrix

void main()
{

	v_TexCoord = texCoord; // We "export" the 'texture_coordinates' variable to the fragment shader
	gl_Position = u_MVP * position;

};
