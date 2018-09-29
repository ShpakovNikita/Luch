#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define MAX_LIGHTS_COUNT 8

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

layout (set = 0, binding = 0) uniform LightBufferObject
{
    Light lights[MAX_LIGHTS_COUNT];
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

float D_GGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float den = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * den * den);
}

float G_CookTorranceGGX(vec3 V, vec3 N, vec3 H, vec3 L)
{
    float VdotH = dot(V, H);
    float NdotH = dot(N, N);
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);
    
    float intermediate = min(NdotV, NdotL);

    float g = 2 * NdotH * intermediate / VdotH;

    return clamp(g, 0.0, 1.0);
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0)*pow(1 - cosTheta, 5);
}

vec3 DiffuseLighting(vec3 color, vec3 L, vec3 N)
{
    float NdotL = max(dot(N, L), 0.0);
    return color * NdotL;
}

vec3 SpecularLighting(vec3 color, vec3 V, vec3 L, vec3 N, vec3 F, float roughness)
{
    vec3 H = normalize(V + L);

    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float NdotL = clamp(dot(N, L), 0.0, 1.0);

    float den = 4 * NdotV * NdotL + 0.001;
    float D = D_GGX(NdotH, roughness);
    float G = G_CookTorranceGGX(V, N, H, L);
    vec3 spec = D * F * G / den;

    return color * spec;
}

float Attenuation(Light light, float d)
{
    return 1.0f - smoothstep(light.range * 0.75f, light.range, d);
}

LightingResult ApplyDirectionalLight(Light light, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
    LightingResult result;

    vec3 color = light.color.xyz;

    vec3 L = normalize(camera.view*light.directionWS).xyz;

    float NdotL = clamp(dot(N, L), 0.0, 1.0);

    vec3 F = F_Schlick(NdotL, F0);
    vec3 kD = (1 - metallic)*(vec3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity;

    return result;
}

LightingResult ApplyPointlLight(Light light, vec3 V, vec3 P, vec3 N, vec3 F0, float metallic, float roughness)
{
    LightingResult result;

    vec3 color = light.color.xyz;

    vec3 L = (camera.view*light.positionWS).xyz - P;
    float dist = length(L);
    L = L/dist;
    float attenuation = Attenuation(light, dist);

    float NdotL = clamp(dot(N, L), 0.0, 1.0);

    vec3 F = F_Schlick(NdotL, F0);
    vec3 kD = (1 - metallic)*(vec3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity * attenuation;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity * attenuation;

    return result;
}

float SpotCone(Light light, vec3 L)
{
    float minCos = cos(light.spotlightAngle);
    float maxCos = mix(minCos, 1, 0.5f);
    float cosAngle = dot(light.directionVS.xyz, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult ApplySpotLight(Light light, vec3 V, vec3 P, vec3 N, vec3 F0, float metallic, float roughness)
{
    LightingResult result;

    vec3 color = light.color.xyz;

    vec3 L = light.positionVS.xyz - P;
    float dist = length(L);
    L = L/dist;

    float attenuation = Attenuation(light, dist);
    float spotIntensity = SpotCone(light, L);

    float NdotL = clamp(dot(N, L), 0.0, 1.0);

    vec3 F = F_Schlick(NdotL, F0);
    vec3 kD = (1 - metallic)*(vec3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity * attenuation * spotIntensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity * attenuation * spotIntensity;

    return result;
}

vec3 ExtractNormal(vec3 normalTS, vec3 T, vec3 N)
{
    vec3 result = normalTS * 2 - vec3(1.0);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * normalTS);
}

void main()
{
    vec3 positionVS = inPositionVS;
    vec3 normalVS = inNormalVS;
#ifdef HAS_TANGENT
    vec3 tangentVS = inTangentVS;
#else
    vec3 q1 = dFdx(inPositionVS);
    vec3 q2 = dFdy(inPositionVS);
    vec2 st1 = dFdx(inTexCoord);
    vec2 st2 = dFdy(inTexCoord);
    vec3 tangentVS = normalize(q1 * st2.t - q2 * st1.t);
#endif
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
    vec3 N;
    if(material.hasNormalTexture)
    {
        vec3 normalTS = texture(normalMap, texCoord).xyz;
        N = ExtractNormal(normalTS, tangentVS, normalVS);
    }
    else
    {
        N = normalize(normalVS);
    }

    N = normalize(normalVS);

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

    //metallic = 0.0;

    vec3 F0 = vec3(0.04);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColor.rgb, metallic);

    LightingResult lightingResult = { vec3(0), vec3(0) };

    for(int i = 0; i < MAX_LIGHTS_COUNT; i++)
    {
        Light light = lights.lights[i];
        if(!light.enabled)
        {
            continue;
        }

        LightingResult intermediateResult = { vec3(0), vec3(0) };

        switch(light.type)
        {
        case LIGHT_DIRECTIONAL:
            intermediateResult = ApplyDirectionalLight(light, V, N, F0, metallic, roughness);
            break;
        case LIGHT_POINT:
            intermediateResult = ApplyPointlLight(light, V, positionVS, N, F0, metallic, roughness);
            break;
        case LIGHT_SPOT:
            intermediateResult = ApplyPointlLight(light, V, positionVS, N, F0, metallic, roughness);
            break;
        default:
            discard;
        }

        lightingResult.diffuse += intermediateResult.diffuse;
        lightingResult.specular += intermediateResult.specular;
    }

    vec3 resultColor = baseColor.xyz * (lightingResult.diffuse + lightingResult.specular);
    
    outColor = vec4(resultColor, baseColor.a);
}
