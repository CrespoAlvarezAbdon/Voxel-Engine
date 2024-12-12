#version 450 core

//layout(location = 0) out vec4 color; // Final result.
layout(binding = 0) uniform sampler2D textureAtlas;

// Input varyings.
in vec2 v_TexCoord; 

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * 0.1 * 500) / (500 + 0.1 - z * (500 - 0.1));	
}

void main()
{
    vec4 textureColor = texture(textureAtlas, v_TexCoord);
    if (textureColor.a < 0.1) // Discard transparent fragments.
        discard;

    gl_FragDepth = gl_FragCoord.z;
    //color = vec4(vec3(LinearizeDepth( gl_FragCoord.z) / 500), 1.0); // perspective
}