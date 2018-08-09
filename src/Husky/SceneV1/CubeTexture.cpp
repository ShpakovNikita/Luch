#include <Husky/SceneV1/CubeTexture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/Image.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/CommandPool.h>

namespace Husky::SceneV1
{
    CubeTexture::CubeTexture(
        const RefPtr<Sampler>& aSampler,
        const String& aName)
        : sampler(aSampler)
        , name(aName)
    {
    }
}
