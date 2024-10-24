#version 450 core

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

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
uniform int u_useComplexLighting;

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
    vec3 padding;
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
vec4 color;
vec4 textureColor = vec4(0, 0, 0, 0);
float specularStrength = 1;

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

    return (ambient + diffuse + specular);
}

vec4 CalcPointLight(PointLight light, LightInstance lightInstance, vec3 n, vec3 viewDir, Material material)
{
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

// Main.
void main() {

	// Get material.
	Material material = materials[v_materialIndex];

	LightInstance lightInstance = directionalLightsInstances[0];
	DirectionalLight light = directionalLights[lightInstance.lightTypeIndex];

	LightInstance lightInstance2 = pointLightsInstances[0];
	PointLight light2 = pointLights[lightInstance2.lightTypeIndex];

	vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
	vec3 viewDir = normalize(u_viewPos - v_pos);
	vec4 textureColor = texture(blockTexture, v_TexCoord);
	if (textureColor.a < 0.1)
		discard;

	 color = vec4(0.0f);

	// Apply directional lights.
	color += CalcDirLight(light, lightInstance, norm, viewDir, material) * u_useComplexLighting;

	// Apply point lights.
	color += CalcPointLight(light2, lightInstance2, norm, viewDir, material);

	// Apply spot lights.

	// Final color calculation.
	color = (color * u_useComplexLighting) * textureColor * v_color;
	color.a = textureColor.a;

	// Weight function
	//float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	
	// store pixel color accumulation
	accum = vec4(color.rgb, color.a) / 10;

	// store pixel revealage threshold
	reveal = color.a;

};