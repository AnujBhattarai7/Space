#include "PCH.h"

#include "WindowStack.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Timer.h"
#include "VulkanApi.h"
#include "VertexBuffers.h"

#include <GLFW/glfw3.h>

namespace Space
{
    static std::vector<PhysicalDevice> _PhysicalDevices;
    static int _CurrentFrameIndex = 0;
    static int _InFlightFrames = 2;

    static VulkanDevice _Device;

    static VkVertexInputBindingDescription GetDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Vertex::_Pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Vertex::_Color);

        return attributeDescriptions;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        if (!requiredExtensions.empty())
            for (auto i : requiredExtensions)
                SP_CORE_PRINT("Device Does Not Support: " << i)

        return requiredExtensions.empty();
    }

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice _Device, VkSurfaceKHR _Surface)
    {
        QueueFamilyIndices indices = _GetDeviceQueueFamilies(_Device, _Surface);

        bool extensionsSupported = checkDeviceExtensionSupport(_Device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            auto swapChainSupport = _GetSwapChainSupportDetails(_Surface, _Device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    const VulkanDevice &GetActiveDevice()
    {
        return _Device;
    }

    void _RecordCommandBuffer(const VulkanApiRenderData &_RenderData)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(_RenderData._CommandBuffers[_CurrentFrameIndex], &beginInfo) != VK_SUCCESS)
            SP_CORE_ERROR("failed to begin recording command buffer!");

        vkCmdBeginRenderPass(_RenderData._CommandBuffers[_CurrentFrameIndex], &_RenderData._RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_RenderData._ViewPortExtent.width);
        viewport.height = static_cast<float>(_RenderData._ViewPortExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_RenderData._CommandBuffers[_CurrentFrameIndex], 0, 1, &viewport);

        VkRect2D scissor{};

        scissor.offset = {0, 0};
        scissor.extent = _RenderData._ViewPortExtent;
        vkCmdSetScissor(_RenderData._CommandBuffers[_CurrentFrameIndex], 0, 1, &scissor);

        vkCmdBindPipeline(_RenderData._CommandBuffers[_CurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, *_RenderData._pGraphicsPipeline);

        VkBuffer VBOs[] = {_RenderData._pCurrentVBuffer->GetHandle()};
        VkDeviceSize Offsets[] = {0};

        vkCmdBindVertexBuffers(_RenderData._CommandBuffers[_CurrentFrameIndex], 0, 1, VBOs, Offsets);

        vkCmdDraw(_RenderData._CommandBuffers[_CurrentFrameIndex],
                  static_cast<uint32_t>(_RenderData._pCurrentVBuffer->GetSize()), 1, 0, 0);

        vkCmdEndRenderPass(_RenderData._CommandBuffers[_CurrentFrameIndex]);

        if (vkEndCommandBuffer(_RenderData._CommandBuffers[_CurrentFrameIndex]) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to record command buffer!");
    }

    std::vector<const char *> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        return extensions;
    }

    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    static std::vector<char> readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            SP_CORE_ERROR("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VkShaderModule VulkanApi::CreateShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    bool VulkanApi::Init(VulkanApiVersion _V)
    {
        SP_PROFILE_SCOPE("Vulkan: Initialization: ")
        SetupInstance(_V);
        SetupDebugMessenger();

        SetupSurface();

        SetupActivePhysicalDevice();

        _Device.Init(*_Data._ActivePhysicalDevice);

        _Data._SwapChain.Init(_Data._Surface, _Device);
        _Data._RenderPass.Init(_Device, _Data._SwapChain);
        _RenderData._pRenderPass = &_Data._RenderPass;
        _Data._SwapChain._InitFrameBuffers(_Device, _Data._RenderPass);

        SetupGraphicPipeline();

        SetupCommandPool();
        SetupCommandBuffers();

        SetupSyncObjects();

        return true;
    }

    bool VulkanApi::ShutDown()
    {
        _RenderData._pCurrentVBuffer = nullptr;

        SP_PROFILE_SCOPE("Vulkan: ShutDown")

        for (auto &i : _RenderData._Render)
            vkDestroySemaphore(_Device, i, nullptr);
        for (auto &i : _RenderData._ImageAvialable)
            vkDestroySemaphore(_Device, i, nullptr);
        for (auto &i : _RenderData._InFlight)
            vkDestroyFence(_Device, i, nullptr);

        vkDestroyCommandPool(_Device, _RenderData._CommandPool, nullptr);

        vkDestroyPipeline(_Device, _Data._GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_Device, _Data._PipelineLayout, nullptr);

        _Data._RenderPass.Destroy(_Device);
        _Data._SwapChain.Destroy(_Device);
        _Device.Destroy();

        if (enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(_Data._VulkanInstance, _Data._DebugMessenger, nullptr);

        vkDestroySurfaceKHR(_Data._VulkanInstance, _Data._Surface, nullptr);
        vkDestroyInstance(_Data._VulkanInstance, nullptr);
        _Data._VulkanInstance = VK_NULL_HANDLE;

        _Data._ActivePhysicalDevice = nullptr;
        _Data._ActivePhysicalDeviceIndex = -1;

        _RenderData._pRenderPass = nullptr;
        _RenderData._pGraphicsPipeline = nullptr;

        return _Data._VulkanInstance == VK_NULL_HANDLE;
    }

    void VulkanApi::Update()
    {
    }

    void VulkanApi::SetupRender()
    {
        _CurrentFrameIndex = (_CurrentFrameIndex + 1) % _InFlightFrames;

        _RenderData._RenderPassBeginInfo = {};
        _RenderData._RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    }

    void VulkanApi::Render(const VertexBuffer *_VB)
    {
        // Wait for PRevious frame to be finished
        vkWaitForFences(_Device, 1, &_RenderData._InFlight[_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

        uint32_t _ImageIndex = 0;
        VkResult _Result = vkAcquireNextImageKHR(_Device, _Data._SwapChain, UINT64_MAX, _RenderData._ImageAvialable[_CurrentFrameIndex], VK_NULL_HANDLE, &_ImageIndex);

        if (_Result == VK_ERROR_OUT_OF_DATE_KHR)
            _Data._SwapChain.Recreate(_Data._Surface, _Device, _RenderData._pRenderPass->GetHandle());
        else if (_Result != VK_SUCCESS && _Result != VK_SUBOPTIMAL_KHR)
            SP_CORE_ERROR("Vulkan: Failed to Acquire Swap Chain Image!!")

        vkResetFences(_Device, 1, &_RenderData._InFlight[_CurrentFrameIndex]);

        _RenderData._CuurentImageIndex = _ImageIndex;
        _RenderData._RenderPassBeginInfo.framebuffer = _Data._SwapChain.GetFrameBuffers()[_ImageIndex];
        _RenderData._RenderPassBeginInfo.renderPass = _RenderData._pRenderPass->GetHandle();

        VkClearValue clearColor;
        clearColor.color.float32[0] = _RenderData._ClearColor.x;
        clearColor.color.float32[1] = _RenderData._ClearColor.y;
        clearColor.color.float32[2] = _RenderData._ClearColor.z;
        clearColor.color.float32[3] = _RenderData._ClearColor.w;

        _RenderData._RenderPassBeginInfo.clearValueCount = 1;
        _RenderData._RenderPassBeginInfo.pClearValues = &clearColor;

        _RenderData._ViewPortExtent.width = _RenderData._ViewPortSize.x;
        _RenderData._ViewPortExtent.height = _RenderData._ViewPortSize.y;

        _RenderData._RenderPassBeginInfo.renderArea.extent = _RenderData._ViewPortExtent;
        _RenderData._RenderPassBeginInfo.renderArea.offset = {0, 0};

        vkResetCommandBuffer(_RenderData._CommandBuffers[_CurrentFrameIndex], 0);
        _RenderData._pCurrentVBuffer = _VB;

        _RecordCommandBuffer(_RenderData);

        VkSemaphore waitSemaphores[] = {_RenderData._ImageAvialable[_CurrentFrameIndex]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo _SubmitInfo{};
        _SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        _SubmitInfo.commandBufferCount = 1;
        _SubmitInfo.pCommandBuffers = &_RenderData._CommandBuffers[_CurrentFrameIndex];

        _SubmitInfo.pWaitDstStageMask = waitStages;
        _SubmitInfo.pWaitSemaphores = waitSemaphores;
        _SubmitInfo.waitSemaphoreCount = 1;

        VkSemaphore signalSemaphores[] = {_RenderData._Render[_CurrentFrameIndex]};
        _SubmitInfo.signalSemaphoreCount = 1;
        _SubmitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_Device.AccessQueues(GRAPHICS), 1, &_SubmitInfo, _RenderData._InFlight[_CurrentFrameIndex]) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to submit draw command buffer!");

        VkPresentInfoKHR _PresentInfo{};
        _PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        _PresentInfo.waitSemaphoreCount = 1;
        _PresentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_Data._SwapChain};

        _PresentInfo.swapchainCount = 1;
        _PresentInfo.pSwapchains = swapChains;
        _PresentInfo.pImageIndices = &_ImageIndex;
        _PresentInfo.pResults = nullptr; // Optional

        _Result = vkQueuePresentKHR(_Device.AccessQueues(RENDER), &_PresentInfo);
        if (_Result == VK_ERROR_OUT_OF_DATE_KHR || _Result == VK_SUBOPTIMAL_KHR || _RenderData._FrameBufferRecreate)
            _Data._SwapChain.Recreate(_Data._Surface, _Device, _RenderData._pRenderPass->GetHandle());
        else if (_Result != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to Acquire Swap Chain Image!!")

        _RenderData._FrameBufferRecreate = false;
    }

    void VulkanApi::SetViewPort(const Vec2 &Size)
    {
        _RenderData._ViewPortSize = Size;
    }

    void VulkanApi::SetClearColor(const Vec4 &ClearColor)
    {
        _RenderData._ClearColor = ClearColor;
    }

    void VulkanApi::SetFramesToRender(int n)
    {
        // _RenderData._FramesNumber = n;

        // if (n > _RenderData._CommandBuffers.size())
        // {
        //     SetupCommandBuffers();
        //     SetupSyncObjects();
        // }
    }

    void VulkanApi::Stop()
    {
        vkDeviceWaitIdle(_Device);
    }

    void VulkanApi::SetupInstance(VulkanApiVersion _V)
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
            SP_CORE_ERROR("Vulkan: validation layers requested, but not available!");

        uint32_t _ExtCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &_ExtCount, nullptr);
        VkExtensionProperties _Prop[_ExtCount];
        vkEnumerateInstanceExtensionProperties(nullptr, &_ExtCount, _Prop);

        SP_CORE_PRINT("Vulkan: " << _ExtCount << " Extensions Supported!")
        SP_CORE_PRINT("Vulkan: Supported Extensions: ")
        for (auto &i : _Prop)
            SP_CORE_PRINT(i.extensionName)

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        if (_V == VULKAN_1_1)
        {
            appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 1, 0);
            appInfo.apiVersion = VK_API_VERSION_1_1;
            appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 1, 0);
        }
        else if (_V == VULKAN_1_2)
        {
            appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;
            appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);
        }
        else if (_V == VULKAN_1_3)
        {
            appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;
            appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
        }

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();

        std::cout << "\n";
        SP_CORE_PRINT("Vulkan Enabled Extensions: ")
        for (auto &i : extensions)
            SP_CORE_PRINT(i)
        std::cout << "\n";

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &_Data._VulkanInstance) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create instance!");
    }

    void VulkanApi::SetupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(_Data._VulkanInstance, &createInfo, nullptr, &_Data._DebugMessenger) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to set up debug messenger!");

        SP_CORE_PRINT("Vulkan: Setup Debug Mesenger")
    }

    void VulkanApi::SetupSurface()
    {
        if (glfwCreateWindowSurface(_Data._VulkanInstance, (GLFWwindow *)WindowStack::GetRenderWindow().GetWindow(), nullptr, &_Data._Surface) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create window surface!");
    }

    void VulkanApi::SetupActivePhysicalDevice()
    {
        uint32_t DeviceCount = 0;
        vkEnumeratePhysicalDevices(_Data._VulkanInstance, &DeviceCount, nullptr);

        if (DeviceCount == 0)
            SP_CORE_ERROR("Vulkan: 0 Physical Devices Found!!")

        SP_CORE_PRINT(DeviceCount << " Physical Devices Found\n")

        VkPhysicalDevice _Devices[DeviceCount];
        vkEnumeratePhysicalDevices(_Data._VulkanInstance, &DeviceCount, _Devices);

        _PhysicalDevices.reserve(DeviceCount);

        for (auto &i : _Devices)
            _PhysicalDevices.emplace_back(i);

        for (size_t i = 0; i < DeviceCount; i++)
        {
            if (_IsPhysicalDeviceSuitable(_Devices[i], _Data._Surface))
            {
                _Data._ActivePhysicalDevice = &_PhysicalDevices[i];
                _Data._ActivePhysicalDeviceIndex = i;
                break;
            }
        }

        if (_Data._ActivePhysicalDevice == nullptr)
            SP_CORE_PRINT("No Proper Vulkan Supported Physical Device FounD!!")

        SP_CORE_PRINT("Active Physical Device: ")
        _PrintPhysicalDeviceInfo(_Data._ActivePhysicalDevice->GetData());
        _Data._ActivePhysicalDevice->SetQueueFamilyInidices(_Data._Surface);
    }

    uint32_t VulkanApi::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(*_Data._ActivePhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        SP_CORE_ERROR("Vertex Buffer Suitable Memory Type not Found!!")
    }

    void VulkanApi::SetupGraphicPipeline()
    {
        SP_PROFILE_SCOPE("Vulkan: Graphics Pipeline")

        auto vertShaderCode = readFile("../../../../Assets/Vert.spv");
        auto fragShaderCode = readFile("../../../../Assets/Frag.spv");

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        // Shader Stage Info
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        // Create Shader Stages
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        auto bindingDescription = GetDescription();
        auto attributeDescriptions = GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(_Device, &pipelineLayoutInfo, nullptr, &_Data._PipelineLayout) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = _Data._PipelineLayout;
        pipelineInfo.renderPass = _Data._RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_Data._GraphicsPipeline) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create Graphics Pipeline!");

        _RenderData._pGraphicsPipeline = &_Data._GraphicsPipeline;

        vkDestroyShaderModule(_Device, fragShaderModule, nullptr);
        vkDestroyShaderModule(_Device, vertShaderModule, nullptr);
    }

    void VulkanApi::SetupCommandPool()
    {
        auto _Families = _GetDeviceQueueFamilies(*_Data._ActivePhysicalDevice, _Data._Surface);

        VkCommandPoolCreateInfo _CreateInfo{};
        _CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        _CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        _CreateInfo.queueFamilyIndex = _Families._Indicies[GRAPHICS];

        if (vkCreateCommandPool(_Device, &_CreateInfo, nullptr, &_RenderData._CommandPool) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Command Pool Setup Failed!!")
        SP_CORE_PRINT("Vulkan: Command Pool Created!!\n")
    }

    void VulkanApi::SetupCommandBuffers()
    {
        if (_RenderData._CommandBuffers.size() > _InFlightFrames)
            vkFreeCommandBuffers(_Device, _RenderData._CommandPool, _RenderData._CommandBuffers.size(), _RenderData._CommandBuffers.data());

        _RenderData._CommandBuffers.resize(_InFlightFrames);

        VkCommandBufferAllocateInfo _Info{};
        _Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        _Info.commandPool = _RenderData._CommandPool;
        _Info.commandBufferCount = _InFlightFrames;
        _Info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(_Device, &_Info, _RenderData._CommandBuffers.data()) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Command Buffer Setup Failed!!")
        SP_CORE_PRINT("Vulkan: Command Buffer Created!!\n")
    }

    void VulkanApi::SetupSyncObjects()
    {
        // If Less num of Objects availbe
        if (_RenderData._ImageAvialable.size() > _InFlightFrames)
        {
            for (auto &i : _RenderData._Render)
                vkDestroySemaphore(_Device, i, nullptr);
            for (auto &i : _RenderData._ImageAvialable)
                vkDestroySemaphore(_Device, i, nullptr);
            for (auto &i : _RenderData._InFlight)
                vkDestroyFence(_Device, i, nullptr);
        }

        _RenderData._ImageAvialable.resize(_InFlightFrames);
        _RenderData._InFlight.resize(_InFlightFrames);
        _RenderData._Render.resize(_InFlightFrames);

        VkSemaphoreCreateInfo _SemInfo{};
        _SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo _FenInfo{};
        _FenInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        _FenInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < _InFlightFrames; i++)
            if (vkCreateSemaphore(_Device, &_SemInfo, nullptr, &_RenderData._ImageAvialable[i]) != VK_SUCCESS ||
                vkCreateSemaphore(_Device, &_SemInfo, nullptr, &_RenderData._Render[i]) != VK_SUCCESS ||
                vkCreateFence(_Device, &_FenInfo, nullptr, &_RenderData._InFlight[i]) != VK_SUCCESS)
                SP_CORE_ERROR("failed to create semaphores!");
    }
}
