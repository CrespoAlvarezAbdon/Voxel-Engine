#version 420 core

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

// This are input varyings.
in vec2 v_TexCoord;
in vec3 v_fragPos;
in vec3 v_normal;
in vec4 v_color;

// Uniforms.
uniform vec3 u_sunLightPos;
uniform vec3 u_viewPos;
uniform sampler2D blockTexture;
uniform int u_useComplexLighting;

// Local variables.
vec4 color;
vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
vec4 textureColor = vec4(0, 0, 0, 0);
vec3 lightColor = vec3(1, 1, 1);
float specularStrength = 1;
float distance = length(u_sunLightPos - v_fragPos) / 100;

// Main.
void main() {

	// Get texture color.
	textureColor = texture(blockTexture, v_TexCoord);

	// Discard transparent fragments.
	if (textureColor.a < 0.1)
		discard;

	lightColor *= (1.0 - textureColor.a); // Higher transparency, less light attenuation

	// Diffuse lighting calculation.
	vec3 norm = normalize(v_normal);
	vec3 lightDir = normalize(u_sunLightPos - v_fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuseLighting = vec4(diff * lightColor, 1.0) / distance;

	// Specular lighting calculation.
	vec3 viewDir = normalize(u_viewPos - v_fragPos);
	vec3 reflectLightDir = reflect(-lightDir, norm);
	float specular = pow(max(dot(viewDir, reflectLightDir), 0.0), 32);
	vec4 specularLighting = vec4(diff * specularStrength * specular * lightColor, 1.0) / distance;
	
	// Final color calculation.
	color = (ambient + (diffuseLighting + specularLighting) * u_useComplexLighting) * textureColor * v_color;
	color.a = textureColor.a;

	// Weight function
	//float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	
	// store pixel color accumulation
	accum = vec4(color.rgb, color.a) / 10;

	// store pixel revealage threshold
	reveal = color.a;

};