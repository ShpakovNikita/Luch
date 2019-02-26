#include <Luch/Vulkan/VulkanInstance.h>

namespace Luch::Vulkan
{
    VkDebugReportCallbackEXT vkDebugCallback;
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };
    bool enableValidationLayers = true;

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        VkDebugReportFlagsEXT /*flags*/,
        VkDebugReportObjectTypeEXT /*objType*/,
        uint64_t /*obj*/,
        size_t /*location*/,
        int32_t /*code*/,
        const char* /*layerPrefix*/,
        const char* msg,
        void* /*userData*/)
    {
        printf("VULKAN VALIDATION: %s\n", msg);
        return VK_FALSE;
    }

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    bool VulkanInstance::SetupDebugCallback() {
        if (!enableValidationLayers)
        {
            return true;
        }

        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = VulkanDebugCallback;

        if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &vkDebugCallback) != VK_SUCCESS) {
            printf("failed to set up debug callback!\n");
            return false;
        }

        return true;
    }

    bool VulkanInstance::CheckValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    bool VulkanInstance::Init()
    {
        if (enableValidationLayers && !CheckValidationLayerSupport()) {
            printf("Error: validation layers requested, but not available!\n");
            return false;
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Luch Sample App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Luch";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        // printf("Found %d extensions:\n", extensionCount);

        std::vector<VkExtensionProperties> extensionsProps(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsProps.data());
        for (const auto& extension : extensionsProps) {
            supportedExtensions.push_back(extension.extensionName);
        }

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = supportedExtensions.size();
        createInfo.ppEnabledExtensionNames = supportedExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        int createResult = VK_FALSE;
        createResult = vkCreateInstance(&createInfo, nullptr, &instance);
        if (createResult != VK_SUCCESS)
        {
            LUCH_ASSERT_MSG(false, "Failed to create Vulkan Instance");
            return false;
        }

        if (!SetupDebugCallback())
        {
            return false;
        }

        return true;
    }
}
