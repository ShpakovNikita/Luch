#pragma once

#include <Husky/BaseApplication.h>
#include <Husky/GeometryGenerator.h>
#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/GLSLShaderCompiler.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/PhysicalDevice.h>
#include <Husky/Vulkan/Surface.h>
#include <Husky/Vulkan/Swapchain.h>
#include "VulkanAllocator.h"

#ifdef _WIN32
#include <Husky/Platform/Win32/WndProcDelegate.h>
#endif

#pragma pack(push)
#pragma pack(1)
struct Uniform
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct FrameResources
{
    Husky::Vulkan::Framebuffer framebuffer;
    //Husky::Vulkan::DescriptorPool descriptorPool;
    Husky::Vulkan::CommandPool graphicsCommandPool;
    
    Husky::Vulkan::CommandBuffer commandBuffer;
    Husky::Vulkan::Image depthStencilBuffer;
    Husky::Vulkan::ImageView depthStencilBufferView;
    Husky::Vulkan::Buffer uniformBuffer;
    Husky::Vulkan::RenderPass renderPass;
    Husky::Vulkan::Pipeline pipeline;
    Husky::Vulkan::PipelineLayout pipelineLayout;
    Husky::Vulkan::IndexBuffer indexBuffer;
    Husky::Vulkan::VertexBuffer vertexBuffer;
    Husky::Vulkan::Fence fence;
    Husky::Vulkan::Semaphore semaphore;
    Husky::Vulkan::DescriptorPool descriptorPool;
    Husky::Vulkan::DescriptorSet descriptorSet;
};

struct GraphicsContext
{
    Husky::Vulkan::PhysicalDevice physicalDevice;
    Husky::Vulkan::Surface surface;
    Husky::Vulkan::GraphicsDevice device;
    Husky::Vulkan::Swapchain swapchain;
    Husky::Vulkan::CommandPool presentCommandPool;
    Husky::Vulkan::DescriptorSetBinding uniformBufferBinding;
    Husky::Vulkan::DescriptorSetLayout descriptorSetLayout;
    Husky::Vulkan::GLSLShaderCompiler shaderCompiler;
    Husky::Vulkan::GLSLShaderCompiler::Bytecode vertexShaderBytecode;
    Husky::Vulkan::GLSLShaderCompiler::Bytecode fragmentShaderBytecode;
    Husky::Vulkan::ShaderModule vertexShaderModule;
    Husky::Vulkan::ShaderModule fragmentShaderModule;
    Husky::GeometryGenerator geometryGenerator;
    Husky::MeshData boxData;
    Husky::Vector<FrameResources> frameResources;
    Uniform uniform;
    Husky::UniquePtr<Husky::glTF::glTF> root;
};

class SampleApplication
    : public Husky::BaseApplication
    , private Husky::Vulkan::VulkanDebugDelegate
#ifdef _WIN32
    , private Husky::Platform::Win32::WndProcDelegate
#endif
{
public:
    SampleApplication() = default;
    bool Initialize(const Husky::Vector<Husky::String>& args) override;
    bool Deinitialize() override;
    void Run() override;
    void Draw();

    const Husky::String& GetApplicationName() const
    {
        static Husky::String applicationName = "Sample";
        return applicationName;
    }

    const Husky::Version& GetApplicationVersion() const
    {
        static Husky::Version applicationVersion{0, 1, 0};
        return applicationVersion;
    }

    const Husky::String& GetMainWindowTitle() const
    {
        static Husky::String windowTitle = "Sample";
        return windowTitle;
    }
private:

    struct CommandPoolCreateResult
    {
        vk::ResultValue<vk::CommandPool> graphicsCommandPool;
        vk::ResultValue<vk::CommandPool> presentCommandPool;
        vk::ResultValue<vk::CommandPool> computeCommandPool;

        Husky::Vector<vk::CommandPool> uniqueCommandPools;
    };

    vk::ResultValue<vk::Instance> CreateVulkanInstance(const vk::AllocationCallbacks& allocationCallbacks);
    vk::ResultValue<vk::DebugReportCallbackEXT> CreateDebugCallback(vk::Instance& instance, const vk::AllocationCallbacks& allocationCallbacks);
    void DestroyDebugCallback(vk::Instance& instance, vk::DebugReportCallbackEXT& callback, const vk::AllocationCallbacks& allocationCallbacks);

    vk::PhysicalDevice ChoosePhysicalDevice(const Husky::Vector<vk::PhysicalDevice>& devices);

#ifdef _WIN32
    LRESULT WndProc(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    ) override;

    std::tuple<HINSTANCE, HWND> CreateMainWindow(const Husky::String& title, Husky::int32 width, Husky::int32 height);
#endif

    vk::ResultValue<vk::Image> CreateDepthStencilBufferForSwapchain(
        vk::Device& device,
        vk::Format format,
        Husky::uint32 graphicsQueueFamilyIndex,
        const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
        const Husky::Vulkan::SwapchainCreateInfo& swapchainCreateInfo,
        const vk::AllocationCallbacks& allocationCallbacks
    );

    vk::ResultValue<vk::ImageView> CreateDepthStencilBufferViewForSwapchain(
        vk::Device& device,
        vk::Format format,
        vk::Image& depthStencilBuffer,
        const vk::AllocationCallbacks& allocationCallbacks
    );

    Husky::Vector<const Husky::char8*> GetRequiredInstanceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetRequiredDeviceExtensionNames() const;
    Husky::Vector<const Husky::char8*> GetValidationLayerNames() const;

    vk::Bool32 DebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char * pLayerPrefix,
        const char * pMessage) override;

    VulkanAllocator allocator;

    vk::AllocationCallbacks allocationCallbacks;
    vk::Instance instance;
    vk::DebugReportCallbackEXT debugCallback;

    std::unique_ptr<GraphicsContext> graphicsContext;

#if _WIN32
    HWND hWnd = nullptr;
    HINSTANCE hInstance = nullptr;
#endif

    Husky::int32 width = 800;
    Husky::int32 height = 600;
    Husky::float32 minDepth = 0.0;
    Husky::float32 maxDepth = 1.0;
    Husky::int32 frameIndex = 0;
};