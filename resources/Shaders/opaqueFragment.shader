#version 420 core

layout(location = 0) out vec4 color; // Final result.

#define MAX_MATERIALS 256 // TODO. PLACE THIS AS AN UNIFORM THAT GETS ITS VALUE FROM A CONST IN DEFINITIONS.H AND THAT IS ONLY UPDATED. USE UNIFORM BUFFER OBJECTS (UBOs) TO UPDATE THIS VALUE ONCE FOR ALL SHADERS.

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_pos;
in vec4 v_color;
flat in int v_materialIndex;

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
    vec3 shininess;
}; 

// UBOs.
layout(std140, binding = 1) uniform Materials {
    Material materials[MAX_MATERIALS];
};

// Local variables.
vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1);
vec4 textureColor = vec4(0, 0, 0, 0);
vec4 lightColor = vec4(1, 1, 1, 1);
float specularStrength =  1;
float distance = length(u_sunLightPos - v_pos) / 100;

// Main.
void main() {
	
	if (u_renderMode == 0) {
	
		/*
		3D rendering.
		*/

		Material material = materials[v_materialIndex];

		// Ambient lighting color.
		vec4 ambientLighting = ambientColor * vec4(material.ambient, 1.0);

		// Diffuse lighting calculation.
		vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
		vec3 lightDir = normalize(u_sunLightPos - v_pos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuseLighting = lightColor * vec4(diff * material.diffuse, 1.0) / max(distance, 1.0);

		// Specular lighting calculation + normal calculation.
		vec3 viewDir = normalize(u_viewPos - v_pos);
		vec3 reflectLightDir = reflect(-lightDir, norm);
		float specular = pow(max(dot(viewDir, reflectLightDir), 0.0), material.shininess.x);
		vec4 specularLighting = lightColor * vec4(diff * specularStrength * specular * material.specular, 1.0) / distance;

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