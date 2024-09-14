#version 420 core

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

#define MAX_MATERIALS 256

// This are input varyings.
in vec2 v_TexCoord;
in vec3 v_pos;
in vec4 v_color;
flat in int v_materialIndex;

// Uniforms.
uniform vec3 u_sunLightPos;
uniform vec3 u_viewPos;
uniform sampler2D blockTexture;
uniform int u_useComplexLighting;

// Structs.
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 shininess;
}; 

// UBOs.
layout(std140, binding = 1) uniform Materials {
    Material materials[MAX_MATERIALS];
};

// Local variables.
vec4 color;
vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1);
vec4 textureColor = vec4(0, 0, 0, 0);
vec4 lightColor = vec4(1, 1, 1, 1);
float specularStrength = 1;
float distance = length(u_sunLightPos - v_pos) / 100;

// Main.
void main() {

	// Get material.
	Material material = materials[v_materialIndex];

	// Get texture color.
	textureColor = texture(blockTexture, v_TexCoord);

	// Discard transparent fragments.
	if (textureColor.a < 0.1)
		discard;

	lightColor *= (1.0 - textureColor.a); // Higher transparency, less light attenuation

	// Ambient lighting calculation.
	vec4 ambientLighting = ambientColor * vec4(material.ambient, 1.0);

	// Diffuse lighting calculation + normal calculation.
	vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
	vec3 lightDir = normalize(u_sunLightPos - v_pos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuseLighting = lightColor * vec4(diff * material.diffuse, 1.0) / max(distance, 1.0);

	// Specular lighting calculation.
	vec3 viewDir = normalize(u_viewPos - v_pos);
	vec3 reflectLightDir = reflect(-lightDir, norm);
	float specular = pow(max(dot(viewDir, reflectLightDir), 0.0), material.shininess.x);
	vec4 specularLighting = lightColor * vec4(diff * specularStrength * specular * material.specular, 1.0) / distance;

	
	// Final color calculation.
	color = (ambientLighting + (diffuseLighting + specularLighting) * u_useComplexLighting) * textureColor * v_color;
	color.a = textureColor.a;

	// Weight function
	//float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	
	// store pixel color accumulation
	accum = vec4(color.rgb, color.a) / 10;

	// store pixel revealage threshold
	reveal = color.a;

};