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
        Resolve,
    };

    struct Attachment
    {
        int32 index = -1;
        Format format = Format::Undefined;
        int32 sampleCount = 1;
        AttachmentLoadOp loadOp = AttachmentLoadOp::DontCare;
        AttachmentStoreOp storeOp = AttachmentStoreOp::DontCare;
    };

    struct ColorAttachment : public Attachment
    {
        ColorSNorm clearValue;
    }

    struct DepthStencilAttachment : public Attachment
    {
        float32 depthClearValue;
        uint32 stencilClearValue;
    };
}

