#version 330 core

layout(location = 0) out vec4 color; // Final result.

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_fragPos;
in vec3 v_normal;

// Uniforms.
uniform vec3 u_sunLightPos;
uniform vec3 u_camPos;
uniform sampler2D u_Texture;
uniform int u_renderMode;

// Local variables.
vec4 ambient = vec4(0.25, 0.25, 0.25, 1);
vec3 lightColor = vec3(1, 1, 1);
float specularStrength = 0.5;
float distance = length(u_sunLightPos - v_fragPos);

// Main.
void main()
{
	
	if (u_renderMode == 0) {
	
		/*
		3D rendering.
		*/

		// Diffuse lighting calculation.
		vec3 norm = normalize(vec3((v_normal.x - 511) / 511, (v_normal.y - 511) / 511, (v_normal.z - 511) / 511));
		vec3 lightDir = normalize(u_sunLightPos - v_fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuseLighting = vec4(diff * lightColor, 1.0) / (distance / 10);

		// Specular lighting calculation.
		vec3 viewDir = normalize(u_camPos - v_fragPos);
		vec3 reflectLightDir = reflect(-lightDir, norm);

		float specular = 0;
		if (diff > 0)
			specular = pow(max(dot(viewDir, reflectLightDir), 0.0), 32);

		vec4 specularLighting = vec4(specular * specularStrength * lightColor, 1.0) / (distance / 10);

		// Final color calculation.
		color = (ambient + diffuseLighting + specularLighting) * texture(u_Texture, v_TexCoord);
	
	}
	else {
	
		/*
		2D rendering
		*/
		color = texture(u_Texture, v_TexCoord);
	
	}

};