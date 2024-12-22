#version 450 core

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

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

// Uniforms.
uniform vec3 u_viewPos;
uniform sampler2D u_textureAtlas;
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
float shadow = 1.0;
vec4 color;

// Functions

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

void ShadowCalculation(vec4 fragPosLightSpace, vec3 n, LightInstance lightInstance)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(depthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lightInstance.pos - v_pos);

    float bias = 0.001;
    shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 1.0;

}

// Main.
void main() {

	// Get material.
	Material material = materials[v_materialIndex];

	LightInstance lightInstance = directionalLightsInstances[0];
	DirectionalLight light = directionalLights[int(lightInstance.lightTypeIndex)];

	vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
	vec3 viewDir = normalize(u_viewPos - v_pos);
	vec4 textureColor = texture(u_textureAtlas, v_TexCoord);
	if (textureColor.a < 0.1)
		discard;

	color = vec4(0.0);

    ShadowCalculation(v_LightSpacePos, norm, lightInstance);

	// Apply directional lights.
	color += CalcDirLight(light, lightInstance, norm, viewDir, material);

    // Apply point lights.
    vec4 acumPointLights = vec4(0.0);
    for(int i = 0; i < u_NPointLights; i++)
    {
        LightInstance lightInstance2 = pointLightsInstances[i];
        PointLight light2 = pointLights[int(lightInstance2.lightTypeIndex)];
        acumPointLights += CalcPointLight(light2, lightInstance2, norm, viewDir, material);
    }

	// Apply spot lights.

	// Final color calculation.
	color = (color + acumPointLights * u_useComplexLighting) * textureColor * v_color;
	color.a = textureColor.a;

	// Weight function
	//float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	
	// store pixel color accumulation
	accum = vec4(color.rgb, color.a) / 10;

	// store pixel revealage threshold
	reveal = color.a;

};