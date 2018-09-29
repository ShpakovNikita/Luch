#include <Husky/Metal/MetalShaderLibrary.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalShaderProgram.h>
#include <Husky/Assert.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalShaderLibrary::MetalShaderLibrary(
        MetalGraphicsDevice* device,
        mtlpp::Library aLibrary)
        : ShaderLibrary(device)
        , library(aLibrary)
    {
    }

    GraphicsResultRefPtr<ShaderProgram> MetalShaderLibrary::CreateShaderProgram(
        ShaderStage stage,
        const String& name)
    {
        auto mtlDevice = static_cast<MetalGraphicsDevice*>(GetGraphicsDevice());
        // TODO error
        auto mtlFunction = library.NewFunction(name.c_str());
        HUSKY_ASSERT(mtlFunction);
        return { GraphicsResult::Success, MakeRef<MetalShaderProgram>(mtlDevice, mtlFunction) };
    }
}
