#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"

using namespace metal;

kernel void kernel_main(
    ushort3 gid [[thread_position_in_grid]],
    texturecube<half, access::read> luminanceMap [[texture(0)]],
    texturecube<half, access::write> irradianceMap [[texture(1)]])
{
    constexpr sampler s;
    ushort face = gid.z;

    half3 irradiance = 0;

    half2 invInputSize = half2(1.0h / luminanceMap.get_width(), 1.0h / luminanceMap.get_height());
    half2 invOutputSize = half2(1.0h / irradianceMap.get_width(), 1.0h / irradianceMap.get_height());

    half3 N = CubemapDirection(gid.xy, face, invOutputSize);

    for(ushort f = 0; f < 6; f++)
    {
        for(ushort i = 0; i < luminanceMap.get_width(); i++)
        {
            for(ushort j = 0; j < luminanceMap.get_height(); j++)
            {
                half3 S = CubemapDirection(ushort2(i, j), f, invInputSize);
                half NdotS = max(dot(N, S), 0.0h);

                irradiance += luminanceMap.read(ushort2(i, j), f).rgb * NdotS;
            }
        }
    }

    irradiance *= 2.0h * M_PI_H * invInputSize.x * invInputSize.y / 6.0h; 

    irradianceMap.write(half4(irradiance, 1.0), gid.xy, face);
}
