#version 450 core

layout(location = 0) out vec4 color; // Final result.

#define MAX_MATERIALS 256 // TODO. MAKE THIS DYNAMIC.
#define MAX_DIRECTIONAL_LIGHTS 256
#define MAX_POINT_LIGHTS 256
#define MAX_SPOT_LIGHTS 256

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_pos;
in vec4 v_color;
flat in int v_materialIndex;

// Uniforms.
uniform vec3 u_viewPos;
uniform sampler2D blockTexture;
uniform int u_renderMode;
uniform int u_useComplexLighting;

// Structs.
struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 shininess;
};

struct DirectionalLight {
    vec4 diffuse;
    vec4 specular;
};

struct PointLight {
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float padding;
};

struct SpotLight {
    vec4 diffuse;
    vec4 specular;
    float cutOffAngle;
    float outerCutOffAngle;
    vec2 padding;
};

struct LightInstance {
    vec3 pos;
	float padding1;
    vec3 dir;
	float padding2;
    uint lightTypeIndex;
};

// UBOs.
layout(std140, binding = 1) uniform Materials {
    Material materials[MAX_MATERIALS];
};

layout(std140, binding = 2) uniform DirectionalLights {
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
};

layout(std140, binding = 3) uniform PointLights {
    PointLight pointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 4) uniform SpotLights {
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

layout(std430, binding = 1) buffer DirectionalLightsInstances {
    LightInstance directionalLightsInstances[];
};

layout(std430, binding = 2) buffer PointLightsInstances {
    LightInstance pointLightsInstances[];
};

layout(std430, binding = 3) buffer SpotLightsInstances {
    LightInstance spotLightsInstances[];
};

// Local variables.
vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1);
vec4 textureColor = vec4(0, 0, 0, 0);
float specularStrength = 1;

// Main.
void main() {
	
	if (u_renderMode == 0) {
	
		/*
		3D rendering.
		*/

		Material material = materials[v_materialIndex];
		LightInstance lightInstance = spotLightsInstances[0];
        SpotLight light = spotLights[lightInstance.lightTypeIndex];

		float distance = length(lightInstance.pos - v_pos) / 100;

		// Ambient lighting color.
		vec4 ambientLighting = ambientColor * material.ambient;

		// Diffuse lighting calculation.
		vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
		vec3 lightDir = normalize(lightInstance.pos - v_pos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuseLighting = light.diffuse * diff * material.diffuse / max(distance, 1.0);

		// Specular lighting calculation + normal calculation.
		vec3 viewDir = normalize(u_viewPos - v_pos);
		vec3 reflectLightDir = reflect(-lightDir, norm);
		float specular = pow(max(dot(viewDir, reflectLightDir), 0.0), material.shininess.x);
		vec4 specularLighting = light.specular * diff * specularStrength * specular * material.specular / distance;

		// Get texture color.
		textureColor = texture(blockTexture, v_TexCoord);
		
		// Discard transparent fragments.
		if (textureColor.a < 0.1)
			discard;

		// Final color calculation.
		color = (ambientLighting + (diffuseLighting + specularLighting) * u_useComplexLighting) * textureColor * v_color;

	}
	else {
	
		/*
		2D rendering
		*/
		color = texture(blockTexture, v_TexCoord);
	
	}

};