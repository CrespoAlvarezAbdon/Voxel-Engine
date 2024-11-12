#version 450 core

//layout(location = 0) out vec4 color; // Final result.

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * 0.1 * 500) / (500 + 0.1 - z * (500 - 0.1));	
}

void main()
{             
    gl_FragDepth = gl_FragCoord.z;
    //color = vec4(vec3(LinearizeDepth( gl_FragCoord.z) / 500), 1.0); // perspective
}