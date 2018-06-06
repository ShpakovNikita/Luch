#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.1415926538

#define LIGHT_POINT 0
#define LIGHT_SPOT 1
#define LIGHT_DIRECTIONAL 2 

struct Light
{
    vec4 positionWS;
    vec4 directionWS;
    vec4 positionVS;
    vec4 directionVS;
    vec4 color;
    bool enabled;
    int type;
    float spotlightAngle;
    float range;
    float intensity;
    vec3 padding;
};

struct LightingResult
{
    vec3 diffuse;
    vec3 specular;
};

layout (set = 0, binding = 0) buffer LightBufferObject
{
    //int lightCount;
    Light lights[];
} lights;

layout (set = 1, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
    vec3 positionWS;
} camera;

layout (set = 3, binding = 0) uniform sampler2D baseColorMap;         // RGB - color, A - opacity
layout (set = 3, binding = 1) uniform sampler2D metallicRoughnessMap; // R - metallic, G - roughness, BA unused
layout (set = 3, binding = 2) uniform sampler2D normalMap;            // RGB - XYZ, A - unused
layout (set = 3, binding = 3) uniform sampler2D occlusionMap;         // greyscale, 
layout (set = 3, binding = 4) uniform sampler2D emissiveMap;          // RGB - light color, A unused

layout(push_constant)
uniform MaterialPushConstants
{
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    bool hasBaseColorTexture;
    bool hasMetallicRoughnessTexture;
    bool hasNormalTexture;
    bool hasOcclusionTexture;
    bool hasEmissiveTexture;
    bool isAlphaMask;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength;
} material;

layout (location = 0) in vec3 inPositionVS;
layout (location = 1) in vec3 inNormalVS;
layout (location = 2) in vec3 inTangentVS;
layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

// http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx

float ChiGCX(float v)
{
    return v > 0 ? 1 : 0;
}

// n - normal
// h - half-vector
float GCXDistribution(vec3 N, vec3 H, float roughness)
{
    float NdotH = dot(N, H);
    float roughness2 = roughness * roughness;
    float NdotH2 = NdotH * NdotH;
    float den = NdotH2 * roughness + (1 - NdotH2);
    return (ChiGCX(NdotH)* roughness2) / (PI * den * den);
}

float GCXPartialGeometryTerm(vec3 v, vec3 n, vec3 h, float roughness)
{
    float cVdotH = clamp(dot(v, h), 0.0, 1.0);
    float cVdotN = clamp(dot(v, n), 0.0, 1.0);
    float chi = ChiGCX(cVdotH / cVdotN);
    float VdotH2 = cVdotH * cVdotH;
    float tan2 = (1 - VdotH2) / VdotH2;
    return (chi * 2) / (1 + sqrt(1 + roughness * roughness *tan2));
}

vec3 FresnelSchlick(float cosinus, vec3 F0)
{
    return F0 + (1 - F0)*pow(1 - cosinus, 5);
}

vec3 DiffuseLighting(vec3 color, vec3 L, vec3 N)
{
    float NdotL = max(dot(N, L), 0.0);
    return color * NdotL;
}

vec3 SpecularLighting(vec3 color, vec3 V, vec3 L, vec3 N, vec3 H, vec3 F0, float roughness)
{
    float NdotL = max(dot(N, L), 0.0);
    float denominator = 4 * dot(V, N) * NdotL;
    vec3 result = vec3(1.0, 1.0, 1.0);
    result *= GCXDistribution(N, H, roughness);
    result *= FresnelSchlick(NdotL, F0);
    result *= GCXPartialGeometryTerm(V, N, H, roughness);
    result /= denominator;
    return result;
}

LightingResult ApplyDirectionalLight(Light light, vec3 V, vec3 N, vec3 F0, float roughness)
{
    LightingResult result;

    vec3 L = normalize(-camera.view*light.directionWS).xyz;
    vec3 H = normalize(L + V);
    vec3 color = light.color.xyz;

    result.diffuse = DiffuseLighting(color, L, N) * light.intensity;
    result.specular = SpecularLighting(color, V, L, N, H, F0, roughness) * light.intensity;

    return result;
}

void main()
{
    vec3 positionVS = inPositionVS;
    vec3 normalVS = inNormalVS;
    vec3 tangentVS = inTangentVS;
    vec2 texCoord = inTexCoord;

    // Alpha masking
    if(material.isAlphaMask)
    {
        bool isCutoff = texture(baseColorMap, texCoord).a < material.alphaCutoff;
        if(material.hasBaseColorTexture && isCutoff)
        {
            discard;
        }
    }

    // TODO normal mapping
    //vec3 N = material.hasNormalTexture ? texture(normalMap, texCoord).xyz : normalize(normalVS);
    vec3 N = normalize(normalVS);

    // we calculate everything in view space, so camera is at 0
    vec3 V = normalize(-positionVS);

    vec4 baseColor = texture(baseColorMap, texCoord);

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;

    if(material.hasMetallicRoughnessTexture)
    {
        vec4 metallicRoughness = texture(metallicRoughnessMap, texCoord);
        metallic *= metallicRoughness.b;
        roughness *= clamp(metallicRoughness.g, 0.04, 1.0);
    }

    vec3 F0 = vec3(0.04);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColor.rgb, metallic);

    LightingResult lightingResult = { vec3(0), vec3(0) };

    //for(int i = 0; i < lights.lightCount; i++)
    for(int i = 0; i < 1; i++)
    {
        Light light = lights.lights[i];
        if(!light.enabled)
        {
            //continue;
        }

        LightingResult intermediateResult = { vec3(0), vec3(0) };

        switch(light.type)
        {
        case LIGHT_DIRECTIONAL:
            intermediateResult = ApplyDirectionalLight(light, V, N, F0, roughness);
            break;
        default:
            discard;
        }

        //lightingResult.diffuse += (1 - metallic)*intermediateResult.diffuse;
        lightingResult.diffuse += intermediateResult.diffuse;
        lightingResult.specular += intermediateResult.specular;
    }

    vec3 resultColor = baseColor.xyz * (lightingResult.diffuse + lightingResult.specular);

    Light light = lights.lights[0];
    vec3 L = normalize(-camera.view*light.directionWS).xyz;
    float NdotL = max(dot(N, L), 0.0);
    
    //outColor = vec4(lights.lights[0].color.xyz, 1.0);

    //outColor = vec4(NdotL, 0.0, 0.0, 1.0);
    outColor = vec4(baseColor.xyz * lightingResult.diffuse, 1.0);
}
