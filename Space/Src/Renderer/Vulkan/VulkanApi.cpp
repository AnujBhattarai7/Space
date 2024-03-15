#include "PCH.h"

#include "WindowStack.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Timer.h"
#include "VulkanApi.h"

#include <GLFW/glfw3.h>

namespace Space
{
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

    void _RecordCommandBuffer(const VulkanApiRenderData &_RenderData)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(_RenderData._CommandBuffers, &beginInfo) != VK_SUCCESS)
            SP_CORE_ERROR("failed to begin recording command buffer!");

        vkCmdBeginRenderPass(_RenderData._CommandBuffers, &_RenderData._RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(_RenderData._CommandBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, *_RenderData._pGraphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_RenderData._ViewPortExtent.width);
        viewport.height = static_cast<float>(_RenderData._ViewPortExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_RenderData._CommandBuffers, 0, 1, &viewport);

        VkRect2D scissor{};

        scissor.offset = {0, 0};
        scissor.extent = _RenderData._ViewPortExtent;
        vkCmdSetScissor(_RenderData._CommandBuffers, 0, 1, &scissor);

        vkCmdDraw(_RenderData._CommandBuffers, 3, 1, 0, 0);

        vkCmdEndRenderPass(_RenderData._CommandBuffers);

        if (vkEndCommandBuffer(_RenderData._CommandBuffers) != VK_SUCCESS)
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

        SetupRenderPass();

        SetupGraphicPipeline();
        SetupFrameBuffers();

        SetupCommandPool();
        SetupCommandBuffers();

        SetupSyncObjects();

        return true;
    }

    bool VulkanApi::ShutDown()
    {
        vkDeviceWaitIdle(_Device);

        SP_PROFILE_SCOPE("Vulkan: ShutDown")

        vkDestroySemaphore(_Device, _RenderData._Render, nullptr);
        vkDestroySemaphore(_Device, _RenderData._ImageAvialable, nullptr);
        vkDestroyFence(_Device, _RenderData._InFlight, nullptr);

        vkDestroyCommandPool(_Device, _RenderData._CommandPool, nullptr);

        for (auto framebuffer : _SwapChainFramebuffers)
            vkDestroyFramebuffer(_Device, framebuffer, nullptr);

        vkDestroyPipeline(_Device, _Data._GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_Device, _Data._PipelineLayout, nullptr);
        vkDestroyRenderPass(_Device, _Data._RenderPass, nullptr);

        _Data._SwapChain.Destroy(_Device);
        _Device.Destroy();

        if (enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(_Data._VulkanInstance, _Data._DebugMessenger, nullptr);

        vkDestroySurfaceKHR(_Data._VulkanInstance, _Data._Surface, nullptr);
        vkDestroyInstance(_Data._VulkanInstance, nullptr);
        _Data._VulkanInstance = VK_NULL_HANDLE;

        _Data._ActivePhysicalDevice = nullptr;
        _Data._ActivePhysicalDeviceIndex = -1;

        return _Data._VulkanInstance == VK_NULL_HANDLE;
    }

    void VulkanApi::Update()
    {
    }

    void VulkanApi::SetupRender()
    {
        _RenderData._RenderPassBeginInfo = {};
        _RenderData._RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    }

    void VulkanApi::Render()
    {
        // Wait for PRevious frame to be finished
        vkWaitForFences(_Device, 1, &_RenderData._InFlight, VK_TRUE, UINT64_MAX);
        vkResetFences(_Device, 1, &_RenderData._InFlight);

        uint32_t _ImageIndex = 0;
        vkAcquireNextImageKHR(_Device, _Data._SwapChain, UINT64_MAX, _RenderData._ImageAvialable, VK_NULL_HANDLE, &_ImageIndex);

        _RenderData._CuurentImageIndex = _ImageIndex;
        _RenderData._RenderPassBeginInfo.framebuffer = _SwapChainFramebuffers[_ImageIndex];
        _RenderData._RenderPassBeginInfo.renderPass = *_RenderData._pRenderPass;

        _RenderData._ViewPortExtent.width = _RenderData._ViewPortSize.x;
        _RenderData._ViewPortExtent.height = _RenderData._ViewPortSize.y;

        _RenderData._RenderPassBeginInfo.renderArea.extent = _RenderData._ViewPortExtent;
        _RenderData._RenderPassBeginInfo.renderArea.offset = {0, 0};

        VkClearValue clearColor;
        clearColor.color.float32[0] = _RenderData._ClearColor.x;
        clearColor.color.float32[1] = _RenderData._ClearColor.y;
        clearColor.color.float32[2] = _RenderData._ClearColor.z;
        clearColor.color.float32[3] = _RenderData._ClearColor.w;

        _RenderData._RenderPassBeginInfo.clearValueCount = 1;
        _RenderData._RenderPassBeginInfo.pClearValues = &clearColor;

        vkResetCommandBuffer(_RenderData._CommandBuffers, 0);

        _RecordCommandBuffer(_RenderData);

        VkSemaphore waitSemaphores[] = {_RenderData._ImageAvialable};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo _SubmitInfo{};
        _SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        _SubmitInfo.commandBufferCount = 1;
        _SubmitInfo.pCommandBuffers = &_RenderData._CommandBuffers;

        _SubmitInfo.pWaitDstStageMask = waitStages;
        _SubmitInfo.pWaitSemaphores = waitSemaphores;
        _SubmitInfo.waitSemaphoreCount = 1;

        VkSemaphore signalSemaphores[] = {_RenderData._Render};
        _SubmitInfo.signalSemaphoreCount = 1;
        _SubmitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_Device.AccessQueues(GRAPHICS), 1, &_SubmitInfo, _RenderData._InFlight) != VK_SUCCESS)
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

        vkQueuePresentKHR(_Device.AccessQueues(RENDER), &_PresentInfo);
    }

    void VulkanApi::SetViewPort(const Vec2 &Size)
    {
        _RenderData._ViewPortSize = Size;
    }

    void VulkanApi::SetupFrameBuffers()
    {
        _SwapChainFramebuffers.resize(_Data._SwapChain.GetImageCount());

        for (size_t i = 0; i < _Data._SwapChain.GetImageCount(); i++)
        {
            VkImageView attachments[] = {
                _Data._SwapChain.GetImageViews()[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _Data._RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _Data._SwapChain.GetExtent().width;
            framebufferInfo.height = _Data._SwapChain.GetExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_Device, &framebufferInfo, nullptr, &_SwapChainFramebuffers[i]) != VK_SUCCESS)
                SP_CORE_ERROR("Vulkan: Failed to create Framebuffer!");
        }
        SP_CORE_PRINT("Vulkan: FrameBuffers Setup!!")
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

    void VulkanApi::SetupRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _Data._SwapChain.GetFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(_Device, &renderPassInfo, nullptr, &_Data._RenderPass) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create render pass!\n");
        SP_CORE_PRINT("Vulkan: Render Pass Created!!\n")

        _RenderData._pRenderPass = &_Data._RenderPass;
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

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

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
            VK_DYNAMIC_STATE_SCISSOR};
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
        VkCommandBufferAllocateInfo _Info{};
        _Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        _Info.commandPool = _RenderData._CommandPool;
        _Info.commandBufferCount = 1;
        _Info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(_Device, &_Info, &_RenderData._CommandBuffers) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Command Buffer Setup Failed!!")
        SP_CORE_PRINT("Vulkan: Command Buffer Created!!\n")
    }

    void VulkanApi::SetupSyncObjects()
    {
        VkSemaphoreCreateInfo _SemInfo{};
        _SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo _FenInfo{};
        _FenInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        _FenInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(_Device, &_SemInfo, nullptr, &_RenderData._ImageAvialable) != VK_SUCCESS ||
            vkCreateSemaphore(_Device, &_SemInfo, nullptr, &_RenderData._Render) != VK_SUCCESS ||
            vkCreateFence(_Device, &_FenInfo, nullptr, &_RenderData._InFlight) != VK_SUCCESS)
            SP_CORE_ERROR("failed to create semaphores!");
    }
}
