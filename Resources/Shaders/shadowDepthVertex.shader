#version 450 core

layout(location = 0) in vec3 position; // Vertices' positions.
layout(location = 1) in vec2 texCoord;

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

// Output varyings.
out vec2 v_TexCoord; 
out vec3 v_pos;

void main()
{
    LightInstance lightInstance = directionalLightsInstances[0];
    gl_Position = lightInstance.MVP * vec4(position, 1.0);

    v_TexCoord = texCoord;
    v_pos = position;
}