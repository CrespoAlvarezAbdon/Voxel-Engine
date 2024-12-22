#version 450 core

layout(location = 0) out vec4 shadowColor; // Final result.
layout(binding = 0) uniform sampler2D textureAtlas;

// Input varyings.
in vec2 v_TexCoord; 

void main()
{
    vec4 textureColor = texture(textureAtlas, v_TexCoord);
    
    if (textureColor.a < 0.1) // Discard opaque and transparent fragments.
        discard;

    if (textureColor.a < 0.9)
    {
        shadowColor = textureColor; // Reduce color intensity by half.
    }
        
    gl_FragDepth = gl_FragCoord.z;
}