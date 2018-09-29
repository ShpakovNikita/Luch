#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#define MAX_LIGHTS_COUNT 8

#define LIGHT_POINT 0
#define LIGHT_SPOT 1
#define LIGHT_DIRECTIONAL 2

struct Light
{
    float4 positionWS;
    float4 directionWS;
    float4 positionVS;
    float4 directionVS;
    float4 color;
    bool enabled;
    int type;
    float spotlightAngle;
    float range;
    float intensity;
    float padding0;
    float padding1;
    float padding2;
};

struct LightingResult
{
    float3 diffuse = float3(0);
    float3 specular = float3(0);
};

struct CameraUniform
{
    float4x4 view;
    float4x4 inverseView;
    float4x4 projection;
    float4x4 inverseProjection;
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    float4 positionWS;
};

struct LightsUniform
{
    Light lights[MAX_LIGHTS_COUNT];
};

float D_GGX(float NdotH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float den = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (M_PI_F * den * den);
}

float G_CookTorranceGGX(float3 V, float3 N, float3 H, float3 L)
{
    float VdotH = dot(V, H);
    float NdotH = dot(N, N);
    float NdotV = dot(N, V);
    float NdotL = dot(N, L);

    float intermediate = min(NdotV, NdotL);

    float g = 2 * NdotH * intermediate / VdotH;

    return clamp(g, 0.0, 1.0);
}

float3 F_Schlick(float cosTheta, float3 F0)
{
    return F0 + (1 - F0) * pow(1 - cosTheta, 5);
}

float3 DiffuseLighting(float3 color, float3 L, float3 N)
{
    float NdotL = max(dot(N, L), 0.0);
    return color * NdotL;
}

float3 SpecularLighting(float3 color, float3 V, float3 L, float3 N, float3 F, float roughness)
{
    float3 H = normalize(V + L);

    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float NdotL = clamp(dot(N, L), 0.0, 1.0);

    float den = 4 * NdotV * NdotL + 0.001;
    float D = D_GGX(NdotH, roughness);
    float G = G_CookTorranceGGX(V, N, H, L);
    float3 spec = D * F * G / den;

    return color * spec;
}

float Attenuation(Light light, float d)
{
    return 1.0f - smoothstep(light.range * 0.75f, light.range, d);
}

LightingResult ApplyDirectionalLight(Light light, float3 V, float3 N, float3 F0, float metallic, float roughness)
{
    LightingResult result;

    float3 color = light.color.xyz;
    float3 L = light.directionVS.xyz;
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    float3 F = F_Schlick(NdotV, F0);
    float3 kD = (1 - metallic)*(float3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity;

    return result;
}

LightingResult ApplyPointlLight(Light light, float3 V, float3 P, float3 N, float3 F0, float metallic, float roughness)
{
    LightingResult result;

    float3 color = light.color.xyz;

    float3 L = light.positionVS.xyz - P.xyz;
    float dist = length(L);
    L = L/dist;
    float attenuation = Attenuation(light, dist);

    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    float3 F = F_Schlick(NdotV, F0);
    float3 kD = (1 - metallic)*(float3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity * attenuation;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity * attenuation;

    return result;
}

float SpotCone(Light light, float3 L)
{
    float minCos = cos(light.spotlightAngle);
    float maxCos = mix(minCos, 1, 0.5f);
    float cosAngle = dot(light.directionVS.xyz, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult ApplySpotLight(Light light, float3 V, float3 P, float3 N, float3 F0, float metallic, float roughness)
{
    LightingResult result;

    float3 color = light.color.xyz;

    float3 L = light.positionVS.xyz - P;
    float dist = length(L);
    L = L/dist;

    float attenuation = Attenuation(light, dist);
    float spotIntensity = SpotCone(light, L);

    float NdotL = clamp(dot(N, L), 0.0, 1.0);

    float3 F = F_Schlick(NdotL, F0);
    float3 kD = (1 - metallic)*(float3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity * attenuation * spotIntensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity * attenuation * spotIntensity;

    return result;
}

float3 ExtractNormal(float3 normalTS, float3 T, float3 N)
{
    float3 result = normalTS * 2 - float3(1.0);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(TBN * result);
}

// These functions are for projection matrices that look like
// ??  ??  ??  ??
// ??  ??  ??  ??
//  0   0   A   B
//  0   0   C   0

// VS - view space
// CS - clip space
// NDC - normalized device coordinates
float DepthToNDC(float depth, float minDepth, float maxDepth)
{
    float result = depth - minDepth;
    result /= maxDepth - minDepth;
    return result;
}

float HomogenousCS(float depthNDC, float A, float B, float C)
{
    float result = B;
    result /= (depthNDC - A / C);
    return result;
}

float2 FragCoordToNDC(float2 fragCoord, float2 size)
{
    float2 pd = 2 * fragCoord / size - float2(1.0);
    return pd;
}

float4 PositionNDCtoCS(float2 positionNDC, float depthNDC, float wCS)
{
    return float4(float3(positionNDC, depthNDC) * wCS, wCS);
}

float4 PositionCStoVS(float4 positionCS, float4x4 inverseProjection)
{
    return inverseProjection * positionCS;
}

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
};

fragment float4 fp_main(
    VertexOut in [[stage_in]],
    device CameraUniform& camera [[buffer(0)]],
    device LightsUniform& lights [[buffer(1)]],
    texture2d<float> baseColorMap [[texture(0)]],
    texture2d<float> normalMap [[texture(1)]],
    depth2d<float> depthBuffer [[texture(2)]],
    sampler baseColorSampler [[sampler(0)]],
    sampler normalMapSampler [[sampler(1)]],
    sampler depthBufferSampler [[sampler(2)]])
{
    float2 texCoord = in.texCoord;

    float4 normalMapSample = normalMap.sample(normalMapSampler, texCoord);
    float2 compressedNormal = normalMapSample.xy * 2 - float2(1.0);
    float normalZ = sqrt(1 - compressedNormal.x * compressedNormal.x - compressedNormal.y * compressedNormal.y);
    float3 N = float3(compressedNormal.x, compressedNormal.y, normalZ);

    float4 baseColorSample = baseColorMap.sample(baseColorSampler, texCoord);

    float metallic = normalMapSample.z;
    float roughness = normalMapSample.w;

    float3 F0 = float3(0.04);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColorSample.rgb, metallic);

    // Depth buffer MUST BE the same size as MRTs
    float2 framebufferSize = float2(depthBuffer.get_width(), depthBuffer.get_height());
    float2 fragCoord = in.position.xy;
    float depth = depthBuffer.sample(depthBufferSampler, texCoord);

    float A = camera.projection[2][2];
    float B = camera.projection[3][2];
    float C = camera.projection[2][3];

    float depthNDC = DepthToNDC(depth, 0, 1);
    float wCS = HomogenousCS(depthNDC, A, B, C);
    float2 xyNDC = FragCoordToNDC(fragCoord, framebufferSize);
    float4 positionCS = PositionNDCtoCS(xyNDC, depthNDC, wCS);

    float3 P = PositionCStoVS(positionCS, camera.inverseProjection).xyz;
    float3 eyePosVS = float3(0); // in view space eye is at origin
    float3 V = normalize(eyePosVS - P);

    LightingResult lightingResult = { float3(0), float3(0) };

    for(int i = 0; i < MAX_LIGHTS_COUNT; i++)
    {
        Light light = lights.lights[i];
        if(!light.enabled)
        {
            continue;
        }

        LightingResult intermediateResult = { float3(0), float3(0) };

        switch(light.type)
        {
        case LIGHT_DIRECTIONAL:
            intermediateResult = ApplyDirectionalLight(light, V, N, F0, metallic, roughness);
            break;
        case LIGHT_POINT:
            intermediateResult = ApplyPointlLight(light, V, P, N, F0, metallic, roughness);
            break;
        case LIGHT_SPOT:
            intermediateResult = ApplySpotLight(light, V, P, N, F0, metallic, roughness);
            break;
        default:
            discard_fragment();
        }

        lightingResult.diffuse += intermediateResult.diffuse;
        lightingResult.specular += intermediateResult.specular;
    }

    float3 resultColor = baseColorSample.xyz * (lightingResult.diffuse + lightingResult.specular);

    return float4(resultColor, 1.0);
}
