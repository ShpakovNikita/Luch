#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"

using namespace metal;

kernel void kernel_main(
    ushort3 gid [[thread_position_in_grid]],
    texturecube<half, access::read> luminance [[texture(0)]],
    texturecube<half, access::write> irradiance [[texture(1)]])
{
    constexpr sampler s;
    ushort face = gid.z;

    half3 result = 0;

    half2 invInputSize = half2(1.0h / luminance.get_width(), 1.0h / luminance.get_height());
    half2 invOutputSize = half2(1.0h / irradiance.get_width(), 1.0h / irradiance.get_height());

    half3 N = CubemapDirection(gid.xy, face, invOutputSize);

    for(ushort f = 0; f < 6; f++)
    {
        for(ushort i = 0; i < luminance.get_width(); i++)
        {
            for(ushort j = 0; j < luminance.get_height(); j++)
            {
                half3 S = CubemapDirection(ushort2(i, j), f, invInputSize);
                half NdotS = max(dot(N, S), 0.0h);

                result += luminance.read(ushort2(i, j), f).rgb * NdotS;
            }
        }
    }

    result *= 2.0h * M_PI_H * invInputSize.x * invInputSize.y / 6.0h; 

    irradiance.write(half4(result, 1.0), gid.xy, face);
}
