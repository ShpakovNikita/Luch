#include <Luch/Metal/MetalShaderLibrary.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalShaderProgram.h>
#include <Luch/Assert.h>

namespace Luch::Metal
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
