#include "SampleApplication.h"

#include <Husky/Vulkan.h>
#include <Husky/Math/Math.h>

#include <Husky/VectorTypes.h>

#include <iostream>

using namespace Husky;
using namespace Husky::Vulkan;

static VkBool32 StaticDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64 object,
    size_t location,
    int32 messageCode,
    const char8 * pLayerPrefix,
    const char8 * pMessage,
    void* userData)
{
    return static_cast<VulkanDebugDelegate*>(userData)->DebugCallback(flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

static const char8* vertexShaderSource =
"layout (binding = 0) uniform buffer\n"
"{\n"
"    mat4 mvp;\n"
"};\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 normal;\n"
"layout (location = 2) in vec2 inTexCoord;\n"
"layout (location = 0) out vec2 outTexCoord;\n"
"void main()\n"
"{\n"
"   outTexCoord = inTexCoord;\n"
"   gl_Position = buffer.mvp * position;\n"
"}\n";

static const char8* fragmentShaderSource =
"layout (location = 0) in vec2 texCoord;\n"
"layout (location = 0) out vec4 outColor;\n"
"void main()\n"
"{\n"
"   outColor = (texCoord.x, texCoord.y, 0.0f, 0.0f);\n"
"}\n";

#ifdef _WIN32

static LRESULT CALLBACK StaticWindowProc(
    _In_ HWND   hwnd,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    using namespace Husky::Platform::Win32;

    static_assert(sizeof(LPARAM) >= sizeof(WndProcDelegate*));

    auto additionalData = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    WndProcDelegate* wndProcDelegate = reinterpret_cast<WndProcDelegate*>(additionalData);

    if (wndProcDelegate != nullptr)
    {
        return wndProcDelegate->WndProc(hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
#endif

bool SampleApplication::Initialize(const Vector<String>& args)
{
    graphicsContext = std::make_unique<GraphicsContext>();

    allocationCallbacks = allocator.GetAllocationCallbacks();

    auto [createInstanceResult, createdInstance] = CreateVulkanInstance(allocationCallbacks);
    if (createInstanceResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    instance = createdInstance;

#ifdef _DEBUG
    auto [createDebugCallbackResult, createdDebugCallback] = CreateDebugCallback(instance, allocationCallbacks);
    if(createDebugCallbackResult != vk::Result::eSuccess)
    {
        // TODO
    }

    debugCallback = createdDebugCallback;
#endif

    auto [enumeratePhysicalDevicesResult, physicalDevices] = instance.enumeratePhysicalDevices();
    if (enumeratePhysicalDevicesResult != vk::Result::eSuccess || physicalDevices.empty())
    {
        // TODO
        return false;
    }

    graphicsContext->physicalDevice = { ChoosePhysicalDevice(physicalDevices), allocationCallbacks };

#if _WIN32
    std::tie(hInstance, hWnd) = CreateMainWindow(GetMainWindowTitle(), width, height);

    if (hWnd == nullptr)
    {
        // TODO
        return false;
    }

    auto [createSurfaceResult, createdSurface] = Surface::CreateWin32Surface(instance, hInstance, hWnd, allocationCallbacks);
    if (createSurfaceResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    graphicsContext->surface = std::move(createdSurface);

    ShowWindow(hWnd, SW_SHOW);
#endif

    GlslShaderCompiler::Bytecode vertexShaderBytecode;

    bool vertexShaderCompiled = graphicsContext->shaderCompiler.TryCompileShader(ShaderStage::Vertex, vertexShaderSource, vertexShaderBytecode);
    HUSKY_ASSERT(vertexShaderCompiled, "Vertex shader failed to compile");

    GlslShaderCompiler::Bytecode fragmentShaderBytecode;
    bool fragmentShaderCompiled = graphicsContext->shaderCompiler.TryCompileShader(ShaderStage::Fragment, fragmentShaderSource, fragmentShaderBytecode);
    HUSKY_ASSERT(fragmentShaderCompiled, "Fragment shader failed to compile");

    auto [chooseQueuesResult, queueIndices] = graphicsContext->physicalDevice.ChooseDeviceQueues(&graphicsContext->surface);
    if (chooseQueuesResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    auto [createDeviceResult, createdDevice] = graphicsContext->physicalDevice.CreateDevice(std::move(queueIndices), GetRequiredDeviceExtensionNames());
    if (createDeviceResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    graphicsContext->device = std::move(createdDevice);
    auto& device = graphicsContext->device;
    auto& indices = graphicsContext->device.GetQueueIndices();

    auto[createdPresentCommandPoolResult, createdPresentCommandPool] = device.CreateCommandPool(indices.presentQueueFamilyIndex, true, false);
    if (createdPresentCommandPoolResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    graphicsContext->presentCommandPool = std::move(createdPresentCommandPool);

    auto [swapchainChooseCreateInfoResult, swapchainCreateInfo] = Swapchain::ChooseSwapchainCreateInfo(width, height, &graphicsContext->physicalDevice, &graphicsContext->surface);
    if (swapchainChooseCreateInfoResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    auto [createSwapchainResult, createdSwapchain] = device.CreateSwapchain(swapchainCreateInfo, &graphicsContext->surface);
    if (createSwapchainResult != vk::Result::eSuccess)
    {
        // TODO
        return false;
    }

    graphicsContext->swapchain = std::move(createdSwapchain);

    auto depthStencilFormat = vk::Format::eD24UnormS8Uint;

    vk::ImageCreateInfo depthBufferCreateInfo;
    depthBufferCreateInfo.setFormat(depthStencilFormat);
    depthBufferCreateInfo.setArrayLayers(1);
    depthBufferCreateInfo.setImageType(vk::ImageType::e2D);
    depthBufferCreateInfo.setExtent({ (uint32)swapchainCreateInfo.width, (uint32)swapchainCreateInfo.height, 1 });
    depthBufferCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    depthBufferCreateInfo.setMipLevels(1);
    depthBufferCreateInfo.setQueueFamilyIndexCount(1);
    depthBufferCreateInfo.setPQueueFamilyIndices(&indices.graphicsQueueFamilyIndex);
    depthBufferCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
    depthBufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    depthBufferCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    depthBufferCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);

    constexpr auto uniformBufferSize = sizeof(Mat4x4);

    graphicsContext->boxData = graphicsContext->geometryGenerator.CreateBox("box01", { 0, 0, 0 }, 1, 1, 1, 0);
    
    auto indicesCount = (int32)graphicsContext->boxData.indices16.size();
    auto indexBufferSize = indicesCount * sizeof(uint16);

    auto verticesCount = (int32)graphicsContext->boxData.vertices.size();
    auto vertexSize = sizeof(Vertex);

    Attachment colorAttachment;
    colorAttachment
        .SetFormat(graphicsContext->swapchain.GetFormat())
        .SetInitialLayout(vk::ImageLayout::eGeneral)
        .SetLoadOp(vk::AttachmentLoadOp::eClear)
        .SetStoreOp(vk::AttachmentStoreOp::eStore)
        .SetSampleCount(SampleCount::e1)
        .SetFinalLayout(vk::ImageLayout::eGeneral);

    Attachment depthAttachment;
    depthAttachment
        .SetFormat(FromVulkanFormat(depthStencilFormat))
        .SetInitialLayout(vk::ImageLayout::eGeneral)
        .SetLoadOp(vk::AttachmentLoadOp::eClear)
        .SetStoreOp(vk::AttachmentStoreOp::eStore)
        .SetStencilLoadOp(vk::AttachmentLoadOp::eClear)
        .SetStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .SetSampleCount(SampleCount::e1)
        .SetFinalLayout(vk::ImageLayout::eGeneral);

    SubpassDescription subpass;
    subpass
        .AddColorAttachment(&colorAttachment, vk::ImageLayout::eColorAttachmentOptimal)
        .WithDepthStencilAttachment(&depthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    RenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo
        .AddAttachment(&colorAttachment)
        .AddAttachment(&depthAttachment)
        .AddSubpass(std::move(subpass));

    PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    //pipelineLayoutCreateInfo
    //    .WithNSetLayouts(1)
    //    .AddSetLayout(&graphicsContext->descriptorSetLayout);

    graphicsContext->frameResources.reserve(swapchainCreateInfo.imageCount);

    PipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.bindingDescriptions =
    {
        {0, sizeof(Vertex), vk::VertexInputRate::eVertex}
    };
    vertexInputState.attributeDescriptions = 
    {
        {
            0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)
        },
        {
            1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)
        },
        {
            2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)
        }
    };

    GraphicsPipelineCreateInfo pipelineState;

    pipelineState.inputAssemblyState.topology = graphicsContext->boxData.primitiveTopology;
    
    pipelineState.viewportState.viewports = { {0.0f, 0.0f, (float32)width, (float32)height, 0.0f, 1.0f} };
    pipelineState.viewportState.scissors = { {{0, 0}, {(uint32)width, (uint32)height}} };

    pipelineState.rasterizationState.cullMode = vk::CullModeFlagBits::eBack;

    pipelineState.depthStencilState.depthTestEnable = true;
    pipelineState.depthStencilState.depthWriteEnable = true;
    pipelineState.depthStencilState.depthCompareOp = vk::CompareOp::eLess;

    for (int32 i = 0; i < swapchainCreateInfo.imageCount; i++)
    {
        auto& frameResources = graphicsContext->frameResources.emplace_back();

        auto[createdGraphicsCommandPoolResult, createdGraphicsCommandPool] = device.CreateCommandPool(indices.graphicsQueueFamilyIndex, false, false);
        if (createdGraphicsCommandPoolResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        frameResources.graphicsCommandPool = std::move(createdGraphicsCommandPool);

        auto[createBufferResult, createdBuffer] = device.CreateBuffer(uniformBufferSize, indices.graphicsQueueFamilyIndex, vk::BufferUsageFlagBits::eUniformBuffer, true);
        if (createBufferResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        frameResources.uniformBuffer = std::move(createdBuffer);

        auto[createVertexBufferResult, createdVertexBuffer] = device.CreateVertexBuffer(vertexSize, verticesCount, indices.graphicsQueueFamilyIndex, true);
        if (createVertexBufferResult != vk::Result::eSuccess)
        {
            return false;
        }

        {
            auto[mapVerticesResult, verticesMemory] = createdVertexBuffer.GetUnderlyingBuffer().MapMemory(indexBufferSize, 0);
            if (mapVerticesResult != vk::Result::eSuccess)
            {
                return false;
            }

            memcpy(verticesMemory, graphicsContext->boxData.vertices.data(), vertexSize * verticesCount);

            createdVertexBuffer.GetUnderlyingBuffer().UnmapMemory();
        }

        frameResources.vertexBuffer = std::move(createdVertexBuffer);

        auto[createIndexBufferResult, createdIndexBuffer] = device.CreateIndexBuffer(IndexType::UInt16, indicesCount, indices.graphicsQueueFamilyIndex, true);
        if (createIndexBufferResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        {
            auto[mapIndicesResult, indicesMemory] = createdIndexBuffer.GetUnderlyingBuffer().MapMemory(indexBufferSize, 0);
            if (mapIndicesResult != vk::Result::eSuccess)
            {
                return false;
            }
            memcpy(indicesMemory, graphicsContext->boxData.indices16.data(), indexBufferSize);

            createdIndexBuffer.GetUnderlyingBuffer().UnmapMemory();
        }

        frameResources.indexBuffer = std::move(createdIndexBuffer);

        auto[createDepthStencilBufferResult, createdDepthStencilBuffer] = device.CreateImage(depthBufferCreateInfo);
        if (createDepthStencilBufferResult != vk::Result::eSuccess)
        {
            return false;
        }

        frameResources.depthStencilBuffer = std::move(createdDepthStencilBuffer);

        auto[createDepthStencilBufferViewResult, createdDepthStencilBufferView] = device.CreateImageView(&frameResources.depthStencilBuffer);
        if (createDepthStencilBufferViewResult != vk::Result::eSuccess)
        {
            return false;
        }

        frameResources.depthStencilBufferView = std::move(createdDepthStencilBufferView);

        auto[createRenderPassResult, createdRenderPass] = device.CreateRenderPass(renderPassCreateInfo);
        if (createRenderPassResult != vk::Result::eSuccess)
        {
            return false;
        }

        frameResources.renderPass = std::move(createdRenderPass);

        FramebufferCreateInfo framebufferCreateInfo(&frameResources.renderPass, swapchainCreateInfo.width, swapchainCreateInfo.height, 1);
        framebufferCreateInfo
            .AddAtachment(&colorAttachment, graphicsContext->swapchain.GetImageView(i))
            .AddAtachment(&depthAttachment, &frameResources.depthStencilBufferView);

        auto [createFramebufferResult, createdFramebuffer] = device.CreateFramebuffer(framebufferCreateInfo);
        if (createFramebufferResult != vk::Result::eSuccess)
        {
            return false;
        }

        frameResources.framebuffer = std::move(createdFramebuffer);

        auto [createPipelineLayoutResult, createdPipelineLayout] = device.CreatePipelineLayout(pipelineLayoutCreateInfo);
        if (createPipelineLayoutResult != vk::Result::eSuccess)
        {
            return false;
        }

        frameResources.pipelineLayout = std::move(createdPipelineLayout);

        GraphicsPipelineCreateInfo pipelineStateCopy{ pipelineState };
        pipelineStateCopy.renderPass = &frameResources.renderPass;
        pipelineStateCopy.layout = &frameResources.pipelineLayout;

        auto[createPipelineResult, createdPipeline] = device.CreateGraphicsPipeline(pipelineStateCopy);
        if (createPipelineResult != vk::Result::eSuccess)
        {
            return false;
        }

        frameResources.pipeline = std::move(createdPipeline);
    }

    return true;
}

bool SampleApplication::Deinitialize()
{
    graphicsContext.release();
    DestroyDebugCallback(instance, debugCallback, allocationCallbacks);
    instance.destroy(allocationCallbacks);
    return true;
}

void SampleApplication::Run()
{
    int32 frameResourceIndex = frameIndex % graphicsContext->frameResources.size();
    auto &frameResources = graphicsContext->frameResources[frameResourceIndex];

    while (true) {} 
}

vk::ResultValue<vk::Instance> SampleApplication::CreateVulkanInstance(const vk::AllocationCallbacks& allocationCallbacks)
{
    auto requiredExtensions = GetRequiredInstanceExtensionNames();
    auto validationLayers = GetValidationLayerNames();

    auto applicationName = GetApplicationName();

    vk::ApplicationInfo applicationInfo;
    applicationInfo.setApiVersion(VK_MAKE_VERSION(1, 0, 56));
    applicationInfo.setApplicationVersion(VK_MAKE_VERSION(0, 1, 0));
    applicationInfo.setPApplicationName(applicationName.c_str());
    applicationInfo.setEngineVersion(VK_MAKE_VERSION(0, 1, 0));
    applicationInfo.setPEngineName("Husky Engine");

    vk::InstanceCreateInfo ci;
    ci.setPApplicationInfo(&applicationInfo);
    ci.setEnabledLayerCount((int32)validationLayers.size());
    ci.setPpEnabledLayerNames(validationLayers.data());
    ci.setEnabledExtensionCount((int32)requiredExtensions.size());
    ci.setPpEnabledExtensionNames(requiredExtensions.data());

    return vk::createInstance(ci, allocationCallbacks);
}

vk::ResultValue<vk::DebugReportCallbackEXT> SampleApplication::CreateDebugCallback(
    vk::Instance& instance,
    const vk::AllocationCallbacks& allocationCallbacks)
{
    VkDebugReportCallbackCreateInfoEXT ci;
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ci.pNext = nullptr;

    ci.flags =
          VK_DEBUG_REPORT_INFORMATION_BIT_EXT
        | VK_DEBUG_REPORT_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_ERROR_BIT_EXT
        | VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    ci.pfnCallback = StaticDebugCallback;
    ci.pUserData = static_cast<VulkanDebugDelegate*>(this);

    const VkAllocationCallbacks& callbacks = allocationCallbacks;

    PFN_vkCreateDebugReportCallbackEXT procAddr = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
    VkDebugReportCallbackEXT callback;
    vk::Result result = static_cast<vk::Result>(procAddr(instance, &ci, &callbacks, &callback));

    vk::DebugReportCallbackEXT debugReportCallback{ callback };
    return { result, debugReportCallback };
}

void SampleApplication::DestroyDebugCallback(
    vk::Instance& instance,
    vk::DebugReportCallbackEXT& callback,
    const vk::AllocationCallbacks& allocationCallbacks)
{
    const VkAllocationCallbacks& callbacks = allocationCallbacks;
    PFN_vkDestroyDebugReportCallbackEXT procAddr = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));
    procAddr(instance, callback, &callbacks);
}

vk::PhysicalDevice SampleApplication::ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices)
{
    return devices[0];
}

#ifdef _WIN32

LRESULT SampleApplication::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

std::tuple<HINSTANCE, HWND> SampleApplication::CreateMainWindow(const Husky::String& title, int32 width, int32 height)
{
    static const TCHAR* className = TEXT("MAIN_WINDOW");

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = StaticWindowProc;
    wc.cbWndExtra = sizeof(Husky::Platform::Win32::WndProcDelegate*);
    wc.lpszClassName = className;

    RegisterClass(&wc);

    // TODO string conversion
    HWND window = CreateWindowEx(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (window)
    {
        using namespace Husky::Platform::Win32;
        static_assert(sizeof(LONG_PTR) >= sizeof(WndProcDelegate*));
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<WndProcDelegate*>(this)));

        ShowWindow(window, SW_NORMAL);
    }

    return { hInstance, window };
}

#endif

//vk::ResultValue<vk::Image> SampleApplication::CreateDepthStencilBufferForSwapchain(
//    vk::Device& device,
//    vk::Format format,
//    uint32 graphicsQueueFamilyIndex,
//    const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
//    const SwapchainCreateInfo& swapchainCreateInfo,
//    const vk::AllocationCallbacks& allocationCallbacks)
//{
//    vk::ImageCreateInfo ci;
//    ci.setFormat(format);
//    ci.setArrayLayers(1);
//    ci.setImageType(vk::ImageType::e2D);
//    ci.setExtent({ swapchainCreateInfo.width, swapchainCreateInfo.height, 1 });
//    ci.setInitialLayout(vk::ImageLayout::eUndefined);
//    ci.setMipLevels(1);
//    ci.setQueueFamilyIndexCount(1);
//    ci.setPQueueFamilyIndices(&graphicsQueueFamilyIndex);
//    ci.setSamples(vk::SampleCountFlagBits::e1);
//    ci.setSharingMode(vk::SharingMode::eExclusive);
//    ci.setTiling(vk::ImageTiling::eOptimal);
//    ci.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc);
//
//    auto [createImageResult, createdImage] = device.createImage(ci, allocationCallbacks);
//    if (createImageResult != vk::Result::eSuccess)
//    {
//        return { createImageResult, createdImage };
//    }
//
//    auto memoryRequirements = device.getImageMemoryRequirements(createdImage);
//    auto memoryType = ChooseMemoryType(physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//
//    vk::MemoryAllocateInfo memoryAllocInfo;
//    memoryAllocInfo.setAllocationSize(memoryRequirements.size);
//    memoryAllocInfo.setMemoryTypeIndex(memoryType);
//
//    auto [allocateResult, allocatedMemory] = device.allocateMemory(memoryAllocInfo);
//    if (allocateResult != vk::Result::eSuccess)
//    {
//        return { allocateResult, createdImage };
//    }
//
//    auto bindResult = device.bindImageMemory(createdImage, allocatedMemory, 0);
//    if (bindResult != vk::Result::eSuccess)
//    {
//        return { bindResult, createdImage };
//    }
//
//    return { vk::Result::eSuccess, createdImage };
//}

vk::ResultValue<vk::ImageView> SampleApplication::CreateDepthStencilBufferViewForSwapchain(
    vk::Device& device,
    vk::Format format,
    vk::Image& depthStencilBuffer,
    const vk::AllocationCallbacks& allocationCallbacks)
{
    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);
    subresourceRange.setLayerCount(1);
    subresourceRange.setLevelCount(1);

    vk::ImageViewCreateInfo ci;
    ci.setImage(depthStencilBuffer);
    ci.setFormat(format);
    ci.setViewType(vk::ImageViewType::e2D);
    ci.setSubresourceRange(subresourceRange);

    return device.createImageView(ci, allocationCallbacks);
}

Vector<const char8*> SampleApplication::GetRequiredInstanceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;

    requiredExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    requiredExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if _WIN32
    requiredExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    return requiredExtensionNames;
}

Husky::Vector<const Husky::char8*> SampleApplication::GetRequiredDeviceExtensionNames() const
{
    Vector<const char8*> requiredExtensionNames;

    requiredExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return requiredExtensionNames;
}

Vector<const char8*> SampleApplication::GetValidationLayerNames() const
{
    return {"VK_LAYER_LUNARG_standard_validation"};
}

VkBool32 SampleApplication::DebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char * pLayerPrefix,
    const char * pMessage)
{
    std::cout << pLayerPrefix << " | " << pMessage << "\n";
    return VK_TRUE;
}
