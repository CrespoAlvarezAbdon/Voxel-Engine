#version 420 core

layout(location = 0) out vec4 color; // Final result.

#define MAX_MATERIALS 256 // TODO. PLACE THIS AS AN UNIFORM THAT GETS ITS VALUE FROM A CONST IN DEFINITIONS.H AND THAT IS ONLY UPDATED. USE UNIFORM BUFFER OBJECTS (UBOs) TO UPDATE THIS VALUE ONCE FOR ALL SHADERS.

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_fragPos;
in vec3 v_normal;
in vec4 v_color;

// Uniforms.
uniform vec3 u_sunLightPos;
uniform vec3 u_viewPos;
uniform sampler2D blockTexture;
uniform int u_renderMode;
uniform int u_useComplexLighting;

// Structs.
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

// Local variables.
vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
vec4 textureColor = vec4(0, 0, 0, 0);
vec3 lightColor = vec3(1, 1, 1);
vec3 lightColorTwo = vec3(1, 0, 0);
float specularStrength =  1;
float distance = length(u_sunLightPos - v_fragPos) / 100;

// Main.
void main() {
	
	if (u_renderMode == 0) {
	
		/*
		3D rendering.
		*/

		// Diffuse lighting calculation.
		vec3 norm = normalize(v_normal);
		vec3 lightDir = normalize(u_sunLightPos - v_fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuseLighting = vec4(diff * lightColor, 1.0) / distance;

		// Specular lighting calculation.
		vec3 viewDir = normalize(u_viewPos - v_fragPos);
		vec3 reflectLightDir = reflect(-lightDir, norm);
		float specular = pow(max(dot(viewDir, reflectLightDir), 0.0), 32);
		vec4 specularLighting = vec4(diff * specularStrength * specular * lightColorTwo, 1.0) / distance;

		// Get texture color.
		textureColor = texture(blockTexture, v_TexCoord);
		
		// Discard transparent fragments.
		if (textureColor.a < 0.1)
			discard;

		// Final color calculation.
		color = (ambient + (diffuseLighting + specularLighting) * u_useComplexLighting) * textureColor * v_color;
	
	}
	else {
	
		/*
		2D rendering
		*/
		color = texture(blockTexture, v_TexCoord);
	
	}

};