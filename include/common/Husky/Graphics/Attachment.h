#pragma once

#include <Husky/Types.h>
#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/Color.h>

namespace Husky::Graphics
{
    enum class AttachmentLoadOperation
    {
        DontCare,
        Load,
        Clear,
    };

    enum class AttachmentStoreOperation
    {
        DontCare,
        Store,
        //Resolve,
    };

    struct AttachmentTextureInfo
    {
        Texture* texture = nullptr;
        int32 mipmapLevel = 0;
        int32 slice = 0;
        int32 depthPlane = 0;
    };

    struct Attachment
    {
        int32 index = -1;
        AttachmentTextureInfo output;
        AttachmentTextureInfo resolve;
        Format format = Format::Undefined;
        AttachmentLoadOp loadOp = AttachmentLoadOp::DontCare;
        AttachmentStoreOp storeOp = AttachmentStoreOp::DontCare;
        // TODO Multisampling
    };

    struct ColorAttachment : public Attachment
    {
        ColorSNorm clearValue;
    }

    struct DepthStencilAttachment : public Attachment
    {
        float32 depthClearValue = 1.0f;
        uint32 stencilClearValue = 0;
    };
}

