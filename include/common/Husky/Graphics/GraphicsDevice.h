#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class GraphicsDevice : public BaseObject
    {
    public:
        virtual ~GraphicsDevice() = 0 {};

        virtual GraphicsResultRefPtr<Texture> CreateTexture(
            const TextureCreateInfo& createInfo) = 0;

        virtual GraphicsResultRefPtr<Buffer> CreateBuffer(
            const BufferCreateInfo& createInfo,
            void* initialData = nullptr) = 0;

        virtual GraphicsResultRefPtr<PipelineLayout> CreatePipelineLayout(
            const PipelineLayoutCreateInfo& createInfo) = 0;
    };
}
