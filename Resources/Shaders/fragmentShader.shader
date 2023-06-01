#version 330 core

layout(location = 0) out vec4 color; // Final result.

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_fragPos;
in vec3 v_normal;

// Uniforms.
uniform vec3 u_sunLightPos;
uniform vec3 u_viewPos;
uniform sampler2D u_Texture;
uniform int u_renderMode;
uniform int u_useComplexLighting;

// Local variables.
vec4 ambient = vec4(0.6, 0.6, 0.6, 1);
vec3 lightColor = vec3(1, 1, 1);
float specularStrength = 1;
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
		if (diff > 0) { // SEE IF THIS IS NECESSARRY.

			specular = pow(clamp(dot(viewDir, reflectLightDir), 0.0, 1.0), 32) / (distance / 10);

		}

		vec4 specularLighting = vec4(specularStrength * specular * lightColor, 1.0);

		// Final color calculation.
		color = (ambient + (diffuseLighting + specularLighting) * u_useComplexLighting) * texture(u_Texture, v_TexCoord);
	
	}
	else {
	
		/*
		2D rendering
		*/
		color = texture(u_Texture, v_TexCoord);
	
	}

};