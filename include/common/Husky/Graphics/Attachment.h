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
        // TODO Multisampling
    };

    struct ColorAttachment : public Attachment
    {
        ColorSNorm32 clearValue;
        AttachmentLoadOperation colorLoadOperation = AttachmentLoadOperation::DontCare;
        AttachmentStoreOperation colorStoreOperation = AttachmentStoreOperation::DontCare;
    };

    struct DepthStencilAttachment : public Attachment
    {
        float32 depthClearValue = 1.0f;
        uint32 stencilClearValue = 0;
        AttachmentLoadOperation depthLoadOperation = AttachmentLoadOperation::DontCare;
        AttachmentStoreOperation depthStoreOperation = AttachmentStoreOperation::DontCare;
        AttachmentLoadOperation stencilLoadOperation = AttachmentLoadOperation::DontCare;
        AttachmentStoreOperation stencilStoreOperation = AttachmentStoreOperation::DontCare;
    };
}

