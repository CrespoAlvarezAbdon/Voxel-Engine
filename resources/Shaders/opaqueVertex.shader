#version 420 core

// Example of use of location
// If the vertex structure is
// struct vertex
// {
//
//	  float positions[3];
//	  float textureCoords[2];
//
// };
// Then location = 0 refers to float positions[3] and
// location = 1 refers to float textureCoords[2].
layout(location = 0) in vec4 position; // Vertices' positions.
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec3 normal;
// The things above this line are also denominated as render targets.

// This are output varying variables. These are variables that are shared between shader programs.
out vec2 v_TexCoord;
out vec3 v_fragPos;
out vec3 v_normal;
out vec4 v_color;

uniform int u_renderMode;
uniform mat4 u_MVP; // u_MVP stands for u_Model_view_projection_matrix although only the view and projection matrix are currently used.
uniform mat4 u_MVPGUI;

void main() {

	if (u_renderMode == 0) { // 3D rendering.

		// Export variables to fragment shader.
		v_TexCoord = texCoord;
		v_fragPos = position.xyz;
		v_normal = normal;
		//v_normal = mat3(transpose(inverse(mat3(1)))) * decodeNormal(normal);
		v_color = vertexColor;
	
		gl_Position = u_MVP * position;

	}
	else if (u_renderMode == 1) { // 2D rendering.
	
		v_TexCoord = texCoord;

		gl_Position = u_MVPGUI * vec4(position.xy, 0.0, 1.0);
	
	}

};
