#pragma once

#include "UUID.h"
#include "VulkanHeader.h"
#include "VulkanQueues.h"

namespace Space
{
    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    enum PhysicalDeviceType
    {
        DISCRETE_GPU,
        INTEGRATED_GPU,
        VIRTUAL_GPU,
        CPU,
    };

    struct PhysicalDeviceData
    {
        char _Name[256];
        int _NameSize = 0;
        uint32_t _ApiVersion, _VendorID, _DriverVersion, _DeviceID;
        PhysicalDeviceType _DeviceType;

        PhysicalDeviceData() {}
    };

    PhysicalDeviceData _SetupPhysicalDeviceData(VkPhysicalDevice _Device);
    void _PrintPhysicalDeviceInfo(const PhysicalDeviceData &_Device);

    class PhysicalDevice
    {
    public:
        PhysicalDevice(VkPhysicalDevice Device) { Init(Device); }
        PhysicalDevice() {}
        ~PhysicalDevice() {}

        void Init(VkPhysicalDevice Device);

        const PhysicalDeviceData &GetData() const { return _Data; }
        VkPhysicalDevice GetHandle() const { return _Device; }
        const UUID &GetID() const { return _ID; }
        const QueueFamilyIndices &GetQueueFamilyIndices() const { return _Indices; }

        void SetQueueFamilyInidices(VkSurfaceKHR _Surface) { _Indices = _GetDeviceQueueFamilies(_Device, _Surface); }

        operator VkPhysicalDevice() const { return _Device; }

    private:
        UUID _ID;

        VkPhysicalDevice _Device;
        PhysicalDeviceData _Data{};
        QueueFamilyIndices _Indices;
    };

    class VulkanDevice
    {
    public:
        VulkanDevice(PhysicalDevice &PDevice) { Init(PDevice); }
        VulkanDevice() {}
        ~VulkanDevice();

        void Destroy();
        void Init(PhysicalDevice &PDevice);

        VkDevice GetHandle() const { return _Device; }
        const UUID &GetID() const { return _ID; }

        operator VkDevice() const { return _Device; }

        VkQueue AccessQueues(int i) { return _Queues[i]; }
        PhysicalDevice &GetPhysicalDevice() { return *_PDevice; }

    private:
        PhysicalDevice *_PDevice = nullptr;
        VkDevice _Device = VK_NULL_HANDLE;

        VkQueue _Queues[SIZE];
        UUID _ID;
    };
} // namespace Spacec
