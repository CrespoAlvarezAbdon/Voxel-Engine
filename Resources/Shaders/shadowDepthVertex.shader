#version 450 core

layout(location = 0) in vec4 position; // Vertices' positions.

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

void main()
{
    LightInstance lightInstance = directionalLightsInstances[0];
    gl_Position = lightInstance.MVP * position;
}