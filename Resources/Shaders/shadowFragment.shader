#version 450 core

layout(binding = 0) uniform sampler2D textureAtlas;

// Input varyings.
in vec2 v_TexCoord; 

void main()
{
    vec4 textureColor = texture(textureAtlas, v_TexCoord);
    
    if (textureColor.a < 0.1) // Discard transparent fragments.
        discard;

    gl_FragDepth = gl_FragCoord.z;
}