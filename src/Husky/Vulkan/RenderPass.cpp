#include <Husky/Vulkan/RenderPass.h.>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Format.h>
#include <Husky/Vulkan/SampleCount.h>

namespace Husky::Vulkan
{

RenderPass::RenderPass(GraphicsDevice* aDevice, vk::RenderPass aRenderPass, int32 aAttachmentCount)
        : device(aDevice)
        , renderPass(aRenderPass)
        , attachmentCount(aAttachmentCount)
    {
    }

    RenderPass::RenderPass(RenderPass && other)
        : device(other.device)
        , renderPass(other.renderPass)
        , attachmentCount(other.attachmentCount)
    {
        other.device = nullptr;
        other.renderPass = nullptr;
    }

    RenderPass& RenderPass::operator=(RenderPass && other)
    {
        Destroy();

        device = other.device;
        renderPass = other.renderPass;
        attachmentCount = other.attachmentCount;

        other.device = nullptr;
        other.renderPass = nullptr;

        return *this;
    }

    RenderPass::~RenderPass()
    {
        Destroy();
    }

    void RenderPass::Destroy()
    {
        if (device)
        {
            device->DestroyRenderPass(this);
        }
    }

    vk::AttachmentDescription RenderPassCreateInfo::ToVulkanAttachmentDescription(const Attachment* attachment)
    {
        vk::AttachmentDescription vkAttachment;
        vkAttachment.setFormat(ToVulkanFormat(attachment->GetFormat()));
        vkAttachment.setLoadOp(attachment->GetLoadOp());
        vkAttachment.setStoreOp(attachment->GetStoreOp());
        vkAttachment.setStencilLoadOp(attachment->GetStencilLoadOp());
        vkAttachment.setStencilStoreOp(attachment->GetStencilStoreOp());
        vkAttachment.setInitialLayout(attachment->GetInitialLayout());
        vkAttachment.setFinalLayout(attachment->GetFinalLayout());
        vkAttachment.setSamples(ToVulkanSampleCount(attachment->GetSampleCount()));
        return vkAttachment;
    }

    RenderPassCreateInfo::VulkanSubpassDescription RenderPassCreateInfo::ToVulkanSubpassDescription(const SubpassDescription& subpassDescription)
    {
        VulkanSubpassDescription vkSubpassDescription;
        
        vkSubpassDescription.inputAttachments.reserve(subpassDescription.inputAttachments.size());
        for (auto inputAttachment : subpassDescription.inputAttachments)
        {
            vk::AttachmentReference vkAttachmentReference;
            vkAttachmentReference.setAttachment(inputAttachment.attachment->GetIndex());
            vkAttachmentReference.setLayout(inputAttachment.layout);
            vkSubpassDescription.inputAttachments.push_back(vkAttachmentReference);
        }

        vkSubpassDescription.colorAttachments.reserve(subpassDescription.colorAttachments.size());
        vkSubpassDescription.resolveAttachments.reserve(subpassDescription.colorAttachments.size());

        for (auto attachment : subpassDescription.colorAttachments)
        {
            vk::AttachmentReference vkColorAttachmentReference;
            vk::AttachmentReference vkResolveAttachmentReference;

            vkColorAttachmentReference.setAttachment(attachment.colorAttachment.attachment->GetIndex());
            vkColorAttachmentReference.setLayout(attachment.colorAttachment.layout);

            if (attachment.resolveAttachment.attachment != nullptr)
            {
                vkResolveAttachmentReference.setAttachment(attachment.resolveAttachment.attachment->GetIndex());
                vkResolveAttachmentReference.setLayout(attachment.resolveAttachment.layout);
            }
            else
            {
                vkResolveAttachmentReference.setAttachment(VK_ATTACHMENT_UNUSED);
            }

            vkSubpassDescription.colorAttachments.push_back(vkColorAttachmentReference);
            vkSubpassDescription.resolveAttachments.push_back(vkResolveAttachmentReference);
        }

        vkSubpassDescription.preserveAttachments.reserve(subpassDescription.preserveAttachments.size());
        for (auto preserveAttachment : subpassDescription.preserveAttachments)
        {
            vkSubpassDescription.preserveAttachments.push_back(preserveAttachment->GetIndex());
        }

        vkSubpassDescription.subpass.setInputAttachmentCount((int32)vkSubpassDescription.inputAttachments.size());
        vkSubpassDescription.subpass.setPInputAttachments(vkSubpassDescription.inputAttachments.data());

        vkSubpassDescription.subpass.setColorAttachmentCount((int32)vkSubpassDescription.colorAttachments.size());
        vkSubpassDescription.subpass.setPColorAttachments(vkSubpassDescription.colorAttachments.data());

        vkSubpassDescription.subpass.setPreserveAttachmentCount((int32)vkSubpassDescription.resolveAttachments.size());
        vkSubpassDescription.subpass.setPResolveAttachments(vkSubpassDescription.resolveAttachments.data());

        vkSubpassDescription.subpass.setPreserveAttachmentCount((int32)vkSubpassDescription.preserveAttachments.size());
        vkSubpassDescription.subpass.setPPreserveAttachments(vkSubpassDescription.preserveAttachments.data());

        vkSubpassDescription.subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

        return vkSubpassDescription;
    }

    vk::SubpassDependency RenderPassCreateInfo::ToVulkanSubpassDependency(const SubpassDependency& subpassDependency)
    {
        vk::SubpassDependency vkSubpassDependecy;

        if (subpassDependency.byRegion)
        {
            vkSubpassDependecy.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
        }

        vkSubpassDependecy.setSrcSubpass(subpassDependency.srcSubpass->GetIndex());
        vkSubpassDependecy.setDstSubpass(subpassDependency.dstSubpass->GetIndex());
        vkSubpassDependecy.setSrcStageMask(subpassDependency.srcStageMask);
        vkSubpassDependecy.setDstStageMask(subpassDependency.dstStageMask);
        vkSubpassDependecy.setSrcAccessMask(subpassDependency.srcAccessMask);
        vkSubpassDependecy.setDstAccessMask(subpassDependency.dstAccessMask);

        return vkSubpassDependecy;
    }

    RenderPassCreateInfo::VulkanRenderPassCreateInfo RenderPassCreateInfo::ToVulkanCreateInfo(const RenderPassCreateInfo& createInfo)
    {
        VulkanRenderPassCreateInfo result;
        auto& vkci = result.createInfo;

        result.attachments.reserve(createInfo.attachments.size());
        for (auto attachment : createInfo.attachments)
        {
            result.attachments.push_back(ToVulkanAttachmentDescription(attachment));
        }

        result.subpasses.reserve(createInfo.subpasses.size());
        result.subpassesInfo.reserve(createInfo.subpasses.size());

        for (auto& subpass : createInfo.subpasses)
        {
            auto subpassInfo = ToVulkanSubpassDescription(subpass);
            result.subpasses.push_back(subpassInfo.subpass);
            result.subpassesInfo.emplace_back(std::move(subpassInfo));
        }

        result.subpassDependencies.reserve(createInfo.subpassDependencies.size());
        for (auto& dependency : createInfo.subpassDependencies)
        {
            result.subpassDependencies.emplace_back(ToVulkanSubpassDependency(dependency));
        }

        vkci.setSubpassCount((int32)result.subpasses.size());
        vkci.setPSubpasses(result.subpasses.data());

        vkci.setAttachmentCount((int32)result.attachments.size());
        vkci.setPAttachments(result.attachments.data());

        vkci.setDependencyCount((int32)result.subpassDependencies.size());
        vkci.setPDependencies(result.subpassDependencies.data());

        return result;
    }
}