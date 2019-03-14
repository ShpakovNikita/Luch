#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"

using namespace metal;

kernel void kernel_main(
    ushort3 gid [[thread_position_in_grid]],
    texturecube<half, access::read> luminanceCube [[texture(0)]],
    texturecube<half, access::write> illuminanceCube [[texture(1)]])
{
    constexpr sampler s;
    ushort face = gid.z;

    half3 illuminance = 0;

    half2 invInputSize = half2(1.0h / luminanceCube.get_width(), 1.0h / luminanceCube.get_height());
    half2 invOutputSize = half2(1.0h / illuminanceCube.get_width(), 1.0h / illuminanceCube.get_height());

    half3 N = CubemapDirection(gid.xy, face, invOutputSize);

    for(ushort f = 0; f < 6; f++)
    {
        for(ushort i = 0; i < luminanceCube.get_width(); i++)
        {
            for(ushort j = 0; j < luminanceCube.get_height(); j++)
            {
                half3 S = CubemapDirection(ushort2(i, j), f, invInputSize);
                half NdotS = saturate(dot(N, S));

                illuminance += luminanceCube.read(ushort2(i, j), f).rgb * NdotS;
            }
        }
    }

    illuminance *= 2.0h * M_PI_H * invInputSize.x * invInputSize.y / 6.0h; 

    illuminanceCube.write(half4(illuminance, 1.0), gid.xy, face);
}
