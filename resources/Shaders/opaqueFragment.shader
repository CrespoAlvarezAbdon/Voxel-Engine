#version 420 core

layout(location = 0) out vec4 color; // Final result.

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

// Local variables.
vec4 ambient = vec4(0.8, 0.8, 0.8, 1);
vec4 textureColor = vec4(0, 0, 0, 0);
vec3 lightColor = vec3(1, 1, 1);
float specularStrength = 2;
float distance = length(u_sunLightPos - v_fragPos);

// Main.
void main() {
	
	if (u_renderMode == 0) {
	
		/*
		3D rendering.
		*/

		// Diffuse lighting calculation.
		vec3 norm = normalize(vec3((v_normal.x - 511) / 511, (v_normal.y - 511) / 511, (v_normal.z - 511) / 511));
		vec3 lightDir = normalize(u_sunLightPos - v_fragPos);
		float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
		vec4 diffuseLighting = vec4(diff * lightColor, 1.0) / (distance / 10);

		// Specular lighting calculation.
		vec3 viewDir = normalize(u_viewPos - v_fragPos);
		vec3 reflectLightDir = lightDir;

		float specular = 0;

		specular = pow(clamp(dot(viewDir, reflectLightDir), 0.0, 1.0), 32) / (distance / 10);

		vec4 specularLighting = vec4(specularStrength * specular * lightColor, 1.0);

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