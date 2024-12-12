#version 450 core

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
layout(location = 0) in vec3 position; // Vertices' positions.
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec4 additionalData; // First byte is material index.
layout(location = 4) in vec3 globalPos;
// The things above this line are also denominated as render targets.

// This are output varying variables. These are variables that are shared between shader programs.
out vec2 v_TexCoord;
out vec3 v_pos;
out vec4 v_color;
out vec4 v_LightSpacePos;
flat out int v_materialIndex;
flat out vec3 v_globalPos;

// Structs.
struct LightInstance {
    vec3 pos;
	float padding1;
    vec3 dir;
	float lightTypeIndex;
    mat4 MVP;
};

// SSBOs.
layout(std430, binding = 1) buffer DirectionalLightsInstances {
    LightInstance directionalLightsInstances[];
};

// Uniforms.
uniform int u_renderMode;
uniform mat4 u_MVP; // u_MVP stands for u_Model_view_projection_matrix although only the view and projection matrix are currently used.
uniform mat4 u_MVPGUI;

void main() {

	if (u_renderMode == 0) { // 3D rendering.

		LightInstance lightInstance = directionalLightsInstances[0];

		// Export variables to fragment shader.
		v_TexCoord = texCoord;
		v_pos = position;
		v_LightSpacePos = lightInstance.MVP * vec4(position, 1.0);

		v_color = vertexColor;
		v_materialIndex = int(additionalData.x);
		v_globalPos = globalPos;

		gl_Position = u_MVP * vec4(position, 1.0);

	}
	else if (u_renderMode == 1) { // 2D rendering.
	
		v_TexCoord = texCoord;

		gl_Position = u_MVPGUI * vec4(position.xy, 0.0, 1.0);
	
	}

};
