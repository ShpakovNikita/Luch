#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant constexpr int MAX_LIGHTS_COUNT = 8;

enum LightType
{
    LIGHT_POINT = 0,
    LIGHT_SPOT = 1,
    LIGHT_DIRECTIONAL = 2,
};

enum LightState
{
    LIGHT_DISABLED = 0,
    LIGHT_ENABLED = 1,
};

struct Light
{
    float4 positionWS;
    float4 directionWS;
    float4 color;
    int state;
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

struct LightingParamsUniform
{
    int lightCount;
    int padding0;
    int padding1;
    int padding2;
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

    return saturate(g);
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

    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float NdotL = saturate(dot(N, L));

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

LightingResult ApplyDirectionalLight(
    CameraUniform camera,
    Light light,
    float3 V,
    float3 N,
    float3 F0,
    float metallic,
    float roughness)
{
    LightingResult result;

    float3 color = light.color.xyz;
    float3 directionVS = (camera.view * light.directionWS).xyz;
    float3 L = directionVS.xyz;
    float NdotV = saturate(dot(N, V));

    float3 F = F_Schlick(NdotV, F0);
    float3 kD = (1 - metallic)*(float3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity;

    return result;
}

LightingResult ApplyPointlLightImpl(
    Light light,
    float3 L,
    float dist,
    float3 V,
    float3 P,
    float3 N,
    float3 F0,
    float metallic,
    float roughness)
{
    LightingResult result;

    float3 color = light.color.xyz;
    float attenuation = Attenuation(light, dist);

    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    float3 F = F_Schlick(NdotV, F0);
    float3 kD = (1 - metallic)*(float3(1.0) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity * attenuation;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity * attenuation;

    return result;
}

LightingResult ApplyPointlLight(
    CameraUniform camera,
    Light light,
    float3 V,
    float3 P,
    float3 N,
    float3 F0,
    float metallic,
    float roughness)
{
    float3 positionVS = (camera.view * light.positionWS).xyz;
    float3 L = positionVS.xyz - P;
    float dist = length(L);
    L = L/dist;

    return ApplyPointlLightImpl(light, L, dist, V, P, N, F0, metallic, roughness);
}

float SpotCone(float spotlightAngle, float3 directionVS, float3 L)
{
    float minCos = cos(spotlightAngle);
    float maxCos = mix(minCos, 1, 0.5f);
    float cosAngle = dot(directionVS, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult ApplySpotLight(
    CameraUniform camera,
    Light light,
    float3 V,
    float3 P,
    float3 N,
    float3 F0,
    float metallic,
    float roughness)
{
    LightingResult result;

    float3 positionVS = (camera.view * light.positionWS).xyz;
    float3 directionVS = (camera.view * light.directionWS).xyz;
    float3 L = positionVS - P;
    float dist = length(L);
    L = L/dist;

    LightingResult pointLighting = ApplyPointlLightImpl(light, L, dist, V, P, N, F0, metallic, roughness);
    float spotIntensity = SpotCone(light.spotlightAngle, directionVS, L);

    result.diffuse = pointLighting.diffuse * spotIntensity;
    result.specular = pointLighting.specular * spotIntensity;

    return result;
}

float DepthToNDC(float depth, float minDepth, float maxDepth)
{
    return (depth - minDepth) / (maxDepth - minDepth);
}

float2 FragCoordToNDC(float2 fragCoord, float2 size)
{
    float2 pd = 2 * fragCoord / size - float2(1.0);
    return float2(pd.x, -pd.y);
}

// These functions are for projection matrices that look like
// ??  ??  ??  ??
// ??  ??  ??  ??
//  0   0   A   B
//  0   0   C   0

// VS - view space
// CS - clip space
// NDC - normalized device coordinates
float HomogenousCoordinate(float depthNDC, float A, float B, float C)
{
    float result = B;
    result /= (depthNDC - A / C);
    return result;
}

float4 PositionNDCtoCS(float3 positionNDC, float w)
{
    return float4(positionNDC * w, w);
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

float3 UncompressNormal(float2 normalMapSample)
{
    // This function unpacks a _view space_ normal
    // Normal is packed by using just its xy view space coordinates
    // z always looks towards camera (-Z) since we are in view space
    float2 normalXY = normalMapSample.xy * 2 - float2(1.0);
    float normalZ = sqrt(saturate(1 - length_squared(normalXY)));
    return float3(normalXY.xy, -normalZ);
}

float3 PackVector(float3 v)
{
    return v*0.5 + float3(0.5);
}

struct FragmentOut
{
    float4 diffuse [[color(0)]];
    float4 specular [[color(1)]];
};

fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    device CameraUniform& camera [[buffer(0)]],
    device LightingParamsUniform* lightingParams [[buffer(1)]],
    device Light* lights [[buffer(2)]],
    texture2d<float> baseColorMap [[texture(0)]],
    texture2d<float> normalMap [[texture(1)]],
    depth2d<float> depthBuffer [[texture(2)]],
    sampler baseColorSampler [[sampler(0)]],
    sampler normalMapSampler [[sampler(1)]],
    sampler depthBufferSampler [[sampler(2)]])
{
    float2 texCoord = in.texCoord;

    float4 baseColorSample = baseColorMap.sample(baseColorSampler, texCoord);
    float4 normalMapSample = normalMap.sample(normalMapSampler, texCoord);

    float3 N = UncompressNormal(normalMapSample.xy);
    float metallic = normalMapSample.z;
    float roughness = normalMapSample.w;

    float3 F0 = float3(0.04);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColorSample.rgb, metallic);

    // Depth buffer MUST BE the same size as MRTs
    float2 attachmentSize = float2(depthBuffer.get_width(), depthBuffer.get_height());
    float2 positionSS = in.position.xy;
    float depth = depthBuffer.sample(depthBufferSampler, texCoord);
    float2 xyNDC = FragCoordToNDC(positionSS, attachmentSize);
    float3 positionNDC = float3(xyNDC, depth);

//    float A = camera.projection[2][2];
//    float B = camera.projection[3][2];
//    float C = camera.projection[2][3];
//    float w = HomogenousCoordinate(depth, A, B, C);
//    float4 positionCS = PositionNDCtoCS(positionNDC, w);
//    float4 intermediateP = camera.inverseProjection * positionCS;
//    float3 P = intermediateP.xyz;

    float4 intermediatePosition = camera.inverseProjection * float4(xyNDC, depth, 1.0);
    float3 P = intermediatePosition.xyz / intermediatePosition.w;
    float3 eyePosVS = float3(0); // in view space eye is at origin
    float3 V = normalize(eyePosVS - P);

    LightingResult lightingResult;

    for(int i = 0; i < lightingParams.lightCount; i++)
    {
        Light light = lights[i];

        LightingResult intermediateResult;

        switch(light.type)
        {
        case LIGHT_DIRECTIONAL:
            intermediateResult = ApplyDirectionalLight(camera, light, V, N, F0, metallic, roughness);
            break;
        case LIGHT_POINT:
            intermediateResult = ApplyPointlLight(camera, light, V, P, N, F0, metallic, roughness);
            break;
        case LIGHT_SPOT:
            intermediateResult = ApplySpotLight(camera, light, V, P, N, F0, metallic, roughness);
            break;
        default:
            intermediateResult = { NAN, NAN };
        }

        lightingResult.diffuse += intermediateResult.diffuse;
        lightingResult.specular += intermediateResult.specular;
    }

    FragmentOut result;

    result.diffuse = float4(lightingResult.diffuse, 1.0);
    result.specular = float4(lightingResult.specular, 1.0);

    return result;
}
