#version 450 core

layout(location = 0) out vec4 color; // Final result.

layout(binding = 0) uniform sampler2D textureAtlas;
layout(binding = 1) uniform sampler2D depthMap;

#define MAX_MATERIALS 256 // TODO. MAKE THIS DYNAMIC.
#define MAX_DIRECTIONAL_LIGHTS 256
#define MAX_POINT_LIGHTS 256
#define MAX_SPOT_LIGHTS 256

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_pos;
in vec4 v_color;
in vec4 v_LightSpacePos;
flat in int v_materialIndex;
flat in vec3 v_globalPos;

// Uniforms.
uniform vec3 u_viewPos;
uniform int u_renderMode;
uniform int u_useComplexLighting;
uniform int u_NPointLights;

// Structs.
struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 shininess;
};

struct DirectionalLight {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct PointLight {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float maxDistance;
    float padding1;
    float padding2;
    float padding3;
};

struct SpotLight {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutOffAngle;
    float outerCutOffAngle;
    float maxDistance;
    float padding;
};

struct LightInstance {
    vec3 pos;
	float padding1;
    vec3 dir;
	float lightTypeIndex;
    mat4 MVP;
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

// SSBOs.
layout(std430, binding = 1) buffer DirectionalLightsInstances {
    LightInstance directionalLightsInstances[];
};

layout(std430, binding = 2) buffer PointLightsInstances {
    LightInstance pointLightsInstances[];
};

layout(std430, binding = 3) buffer SpotLightsInstances {
    LightInstance spotLightsInstances[];
};

// Variables.
float shadow = 0.0;

// Functions.
vec4 CalcDirLight(DirectionalLight light, LightInstance lightInstance, vec3 n, vec3 viewDir, Material material) {

    vec3 lightDir = normalize(-lightInstance.dir);

    // diffuse shading
    float diff = max(dot(n, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, n);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x); // Remember that shininess is a vec4 for padding but 'x' is the real shininess value.

    // combine results
    vec4 ambient = light.ambient * material.ambient;
    vec4 diffuse  = light.diffuse  * diff * material.diffuse;
    vec4 specular = light.specular * spec * material.specular;

    diffuse *= shadow;
    specular *= (shadow < 0.75) ? 0 : 1;

    return (ambient + diffuse + specular);

}

vec4 CalcPointLight(PointLight light, LightInstance lightInstance, vec3 n, vec3 viewDir, Material material) {

    vec3 lightDir = normalize(lightInstance.pos - v_pos);

    // diffuse shading
    float diff = max(dot(n, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, n);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x);

    // attenuation
    float distance = length(lightInstance.pos - v_pos);
    float attenuation = max(1.0 - distance / (light.maxDistance), 0.0);

    // combine results
    vec4 ambient = light.ambient * material.ambient;
    vec4 diffuse  = light.diffuse  * diff * material.diffuse;
    vec4 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);

}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 n, LightInstance lightInstance)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(depthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lightInstance.pos - v_pos);
    // check whether current frag pos is in shadow
    float s = currentDepth - 0.001 > closestDepth  ? 0.0 : 1.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        s = 1.0;
        
    return s;

    // PCF
    s = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            s += currentDepth - 0.001 > pcfDepth  ? 0.0 : 1.0;        
        }    
    }
    s /= 9.0;

}

// Main.
void main() {
	
	if (u_renderMode == 0) {
	
		/*
		3D rendering.
		*/

		Material material = materials[v_materialIndex];

		LightInstance lightInstance = directionalLightsInstances[0];
        DirectionalLight light = directionalLights[int(lightInstance.lightTypeIndex)];

		vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
		vec3 viewDir = normalize(u_viewPos - v_pos);
		vec4 textureColor = texture(textureAtlas, v_TexCoord);
		if (textureColor.a < 0.1) // Discard transparent fragments.
			discard;

        color = vec4(0.0);

        // Apply shadows.
        shadow = ShadowCalculation(v_LightSpacePos, norm, lightInstance);

		// Apply directional lights.
	    color += CalcDirLight(light, lightInstance, norm, viewDir, material) * u_useComplexLighting;

		// Apply point lights.
        vec4 acumPointLights = vec4(0.0);
        for(int i = 0; i < u_NPointLights; i++)
        {
            LightInstance lightInstance2 = pointLightsInstances[i];
            PointLight light2 = pointLights[int(lightInstance2.lightTypeIndex)];
            acumPointLights += CalcPointLight(light2, lightInstance2, norm, viewDir, material);
        }

		// Apply spot lights.

		// Finally apply texture and v_color
		color = (color + acumPointLights * u_useComplexLighting) * textureColor * v_color;

	}
	else {
	
		/*
		2D rendering
		*/
		color = texture(textureAtlas, v_TexCoord);
	
	}

};