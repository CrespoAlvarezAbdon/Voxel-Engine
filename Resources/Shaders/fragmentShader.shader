#version 330 core

layout(location = 0) out vec4 color; // Final result.

// This are input varyings.
in vec2 v_TexCoord; 
in vec3 v_fragPos;
in vec3 v_normal;

uniform vec3 u_sunLightPos;
uniform sampler2D u_Texture;
vec4 ambient = vec4(0, 0, 0, 1);
vec3 lightColor = vec3(1, 1, 1);

void main()
{
	
	vec3 norm = normalize(vec3((v_normal.x - 511) / 511, (v_normal.y - 511) / 511, (v_normal.z - 511) / 511));
	vec3 lightDir = normalize(u_sunLightPos - v_fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuseLighting = vec4(diff * lightColor, 1.0f);

	color = (ambient + diffuseLighting) * texture(u_Texture, v_TexCoord);

};