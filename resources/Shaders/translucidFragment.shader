#version 450 core

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

layout(binding = 0) uniform sampler2D textureAtlas;
layout(binding = 1) uniform sampler2D depthMap;
layout(binding = 2) uniform sampler2D shadowColor;
layout(binding = 3) uniform sampler2D translucentDepthMap;

#define MAX_MATERIALS 256 // TODO. MAKE THIS DYNAMIC.
#define MAX_DIRECTIONAL_LIGHTS 256
#define MAX_POINT_LIGHTS 256
#define MAX_SPOT_LIGHTS 256
#define CHUNK_SIZE 16 // TODO. UNIFY THIS DEFINITIONS THAT ARE ALSO IN CPP IN A CUSTOM FILE FORMAT THAT THE ENGINE 'COMPILES'

// This are input varyings.
in vec2 v_TexCoord;
in vec3 v_pos;
in vec4 v_color;
in vec4 v_LightSpacePos;
in vec4 v_blockLightColor;
in vec2 v_baryCoords;
in vec4 v_mixedVertexColorData;
flat in int v_materialIndex;

// Uniforms.
uniform vec3 u_viewPos;
uniform int u_renderMode;
uniform int u_useComplexLighting;
uniform int u_useDynamicLights;

// Structs.
struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 shininess;
};

struct DirectionalLightPacked {
    ivec4 params; // x = ambient, y = diffuse, z = specular, w unused
};

struct DirectionalLight {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct PointLightPacked {
    ivec4 params; // x = ambient, y = diffuse, z = specular, w = maxDistance
};

struct PointLight {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float maxDistance;
    float padding1;
    float padding2;
    float padding3;
};

struct SpotLight {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutOffAngle;
    float outerCutOffAngle;
    float maxDistance;
    float padding;
};

struct LightInstance {
    vec3 pos;
	float padding1;
    vec3 dir;
	float lightTypeIndex;
    mat4 MVP;
};

struct ChunkDynamicLightData {
    uint isSolid[1458]; // 1458 32-bit unsigned integers = 5832 bytes = 18x18x18, which is the size used in the isOpaque array of a chunk
};

// UBOs.
layout(std140, binding = 1) uniform Materials {
    Material materials[MAX_MATERIALS];
};

layout(std140, binding = 2) uniform DirectionalLightsPacked {
    DirectionalLightPacked directionalLightsPacked[MAX_DIRECTIONAL_LIGHTS];
};

layout(std140, binding = 3) uniform PointLights {
    PointLight pointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 4) uniform SpotLights {
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

// SSBOs.
layout(std430, binding = 1) buffer DirectionalLightsInstances {
    LightInstance directionalLightsInstances[];
};

layout(std430, binding = 2) buffer PointLightsInstances {
    LightInstance pointLightsInstances[];
};

layout(std430, binding = 3) buffer SpotLightsInstances {
    LightInstance spotLightsInstances[];
};

layout(std430, binding = 4) buffer ChunkDynamicData {
    ChunkDynamicLightData chunkDynamicLightData;
};

// Variables.
float shadow = 0.0;
float translucentShadow = 0.0;
vec4 coloredShadow = vec4(0.0);
vec4 color;

// Functions.
vec4 unpackColor(int packedColor) {

    // Extract each 8-bit chunk
    int r = int( packedColor        & 0xFFu);
    int g = int((packedColor >>  8) & 0xFFu);
    int b = int((packedColor >> 16) & 0xFFu);
    int a = int((packedColor >> 24) & 0xFFu);

    // Reinterpret as signed 8-bit (convert >127 to negative)
    r = (r > 127) ? r - 256 : r;
    g = (g > 127) ? g - 256 : g;
    b = (b > 127) ? b - 256 : b;
    a = (a > 127) ? a - 256 : a;

    // Normalize to [-1,1]
    return vec4(r, g, b, a) / 127.0;

}

// Shadow values are expected here to go from 1 (full bright) to 0 (full shadow)
vec4 CalcDirLight(DirectionalLight light, LightInstance lightInstance, vec3 n, vec3 viewDir, float shadow, float translucentShadow, float hitDirLightModifier, Material material) {

    // Specular shading calculations.
    vec3 reflectDir = reflect(lightInstance.dir, n);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x); // Remember that 'shininess' is a vec4 for padding but 'x' is the actual shininess value.

    vec4 ambient = light.ambient * material.ambient;
    vec4 diffuse  = light.diffuse * hitDirLightModifier * material.diffuse;
    vec4 specular = light.specular * spec * material.specular;

    diffuse *= shadow;
    diffuse *= translucentShadow;
    specular *= (shadow < 0.75 || translucentShadow < 0.75) ? 0 : 1;

    return (ambient + diffuse + specular);

}

int floorMod(int a, int b) {

    return a - b * int(floor(double(a) / b));

}

float floorMod(float a, float b) {

    return a - b * floor(a / b);

}

vec3 globalCoordsToChunkRelCoords(vec3 global) {

    vec3 chunkRelPos = global;
    chunkRelPos.x = int(floor(chunkRelPos.x));
    chunkRelPos.y = int(floor(chunkRelPos.y));
    chunkRelPos.z = int(floor(chunkRelPos.z));
    chunkRelPos.x = int(floorMod(int(chunkRelPos.x >= 0 ? chunkRelPos.x : CHUNK_SIZE + chunkRelPos.x), CHUNK_SIZE));
    chunkRelPos.y = int(floorMod(int(chunkRelPos.y >= 0 ? chunkRelPos.y : CHUNK_SIZE + chunkRelPos.y), CHUNK_SIZE));
    chunkRelPos.z = int(floorMod(int(chunkRelPos.z >= 0 ? chunkRelPos.z : CHUNK_SIZE + chunkRelPos.z), CHUNK_SIZE));
    return chunkRelPos;

}

uint GetVoxelSolid(int x, int y, int z)
{
    int byteIndex = x * 324 + y * 18 + z;

    uint uintIndex = uint(byteIndex) >> 2u;
    uint byteOffset = uint(byteIndex) & 3u;

    uint packedData = chunkDynamicLightData.isSolid[uintIndex];

    return (packedData >> (byteOffset * 8u)) & 0xFFu;
}

vec4 CalcPointLight(PointLight light, LightInstance lightInstance, vec3 n, vec3 viewDir, Material material) {

    // First, check if the fragment is actually hit by this light with DDA algorithm.
    vec3 chunkLightPos = globalCoordsToChunkRelCoords(lightInstance.pos);
    vec3 chunkPos = globalCoordsToChunkRelCoords(v_pos);

    vec3 blockToLightDir = chunkLightPos - chunkPos;

    float step = max(abs(blockToLightDir.x), max(abs(blockToLightDir.y), abs(blockToLightDir.z)));
    float stepX = blockToLightDir.x / step;
    float stepY = blockToLightDir.y / step;
    float stepZ = blockToLightDir.z / step;

    vec3 currentPos = chunkPos;

    int voxelX = int(currentPos.x); // TODO. WHY NO ROUND HERE?
    int voxelY = int(currentPos.y);
    int voxelZ = int(currentPos.z);

    bool hitByLight = false;
    for(int i = 0; i <= int(step) && !hitByLight; i++) {

        if (voxelX < 0 || voxelX >= 16 || voxelY < 0 || voxelY >= 16 || voxelZ < 0 || voxelZ >= 16)
            break;

        if (GetVoxelSolid(voxelX, voxelY, voxelZ) == 1)
            break;

        if(voxelX == chunkLightPos.x && voxelY == chunkLightPos.y && voxelZ == chunkLightPos.z)
            hitByLight = true;

        currentPos.x += stepX;
        currentPos.y += stepY;
        currentPos.z += stepZ;

        voxelX = int(round(currentPos.x));
        voxelY = int(round(currentPos.y));
        voxelZ = int(round(currentPos.z));

    }

    // Then, if actually hit by the light, apply it.
    if(hitByLight)
    {
        vec3 lightDir = normalize(lightInstance.pos - v_pos);

        // Diffuse shading calculations.
        float diff = max(dot(n, lightDir), 0.0);

        // Specular shading calculations.
        vec3 reflectDir = reflect(-lightDir, n);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x);

        // Get light attenuation.
        float distance = length(lightInstance.pos - v_pos);
        float attenuation = max(1.0 - distance / (light.maxDistance), 0.0);

        vec4 ambient = light.ambient * material.ambient;
        vec4 diffuse = light.diffuse * diff * material.diffuse;
        vec4 specular = light.specular * spec * material.specular;
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        return (ambient + diffuse + specular);
    }
    else
    {
        return vec4(0.0);
    }
}

void ShadowCalculation(vec4 fragPosLightSpace, vec3 n, LightInstance lightInstance) {

    if(u_useComplexLighting == 1) {

        float bias = 0.001;
        vec3 displacedPos = fragPosLightSpace.xyz;
        vec3 projCoords = displacedPos.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;

        //vec3 normal = normalize(n);
        //vec3 lightDir = normalize(lightInstance.pos - v_pos);
        //float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0001);
        //float bias = 0.001 * (1.0 - dot(normal, lightDir)) + 0.0001;

        float closestDepth = texture(depthMap, projCoords.xy).r; 
        float closestTranslucentDepth = texture(translucentDepthMap, projCoords.xy).r;
        float currentDepth = projCoords.z;
        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        translucentShadow += currentDepth - bias > closestTranslucentDepth ? 1.0 : 0.0;

        // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
        if(projCoords.z > 1.0) {

            shadow = 0.0;
            translucentShadow = 0.0;

        }
            
        // Get shadow's color.
        if (translucentShadow == 1.0 && shadow == 0.0) {

            coloredShadow = texture(shadowColor, projCoords.xy);

        }

    }

}

// Main.
void main() {

    /*
    3D rendering.
    */

    if (u_renderMode == 1)
    {

        // Get material.
        Material material = materials[v_materialIndex];
        LightInstance lightInstance = directionalLightsInstances[0];
        DirectionalLightPacked lightPacked = directionalLightsPacked[int(lightInstance.lightTypeIndex)];
        DirectionalLight light;
        light.ambient = unpackColor(lightPacked.params[0]);
        light.diffuse = unpackColor(lightPacked.params[1]);
        light.specular = unpackColor(lightPacked.params[2]);

        // Initialize and declare function variables.
        color = vec4(0.0);
        vec4 albedo = texture(textureAtlas, v_TexCoord) * v_color;
        vec3 norm = normalize(cross(dFdx(v_pos), dFdy(v_pos)));
        vec3 viewDir = normalize(u_viewPos - v_pos);
        float hitDirLightModifier = max(dot(norm, normalize(-lightInstance.dir)), 0.0);
        bool isTransparent = albedo.a < 0.1;
        
        // Discard transparent fragments.
        if (isTransparent)
            discard;

        // Apply shadows.
        ShadowCalculation(v_LightSpacePos, norm, lightInstance);

        // Apply lights.
        vec4 blockLit = vec4(0.0);
        if(u_useDynamicLights == 0)
            blockLit = (v_blockLightColor + v_baryCoords.x * v_baryCoords.y * v_mixedVertexColorData);
        vec4 dirLight = CalcDirLight(light, lightInstance, norm, viewDir, (1 - shadow), (1 - translucentShadow), hitDirLightModifier, material) * u_useComplexLighting;
            
        // Final color calculation.
        color = albedo * dirLight + blockLit;
        color.a = albedo.a;

        if(translucentShadow == 1.0 && hitDirLightModifier > 0.5)
        {
            color += coloredShadow / 4 * u_useComplexLighting;
        }

        // Weight function
        //float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
        
        // store pixel color accumulation
        accum = vec4(color.rgb, color.a) / 10;

        // store pixel revealage threshold
        reveal = color.a;

    }
    else if (u_renderMode == 2)
    {

    }

};