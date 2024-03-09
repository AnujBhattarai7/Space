#pragma once

#include "VulkanQueues.h"

namespace Space
{
    // Physical Device
    enum PhysicalDeviceType
    {
        PHYSCIAL_DEVICE_TYPE_OTHER,
        PHYSCIAL_DEVICE_TYPE_INEGRATED_GPU,
        PHYSCIAL_DEVICE_TYPE_DISCRETE_GPU,
        PHYSCIAL_DEVICE_TYPE_VIRTUAL_GPU,
        PHYSCIAL_DEVICE_TYPE_CPU,
    };

    struct PhysicalDeviceData
    {
        char _Name[256];
        int _NameSize = 0;
        PhysicalDeviceType _DeviceType;
        // The ID used by Space
        uint32_t _ID;
        uint32_t _DeviceID, _VenderID, _DriverVersion;

        PhysicalDeviceData() {}
    };

    class PhysicalDevice
    {
    public:
        PhysicalDevice() : _DeviceHandle(VK_NULL_HANDLE) {}
        PhysicalDevice(VkPhysicalDevice _Device) { Init(_Device); }
        ~PhysicalDevice() {}

        // Sets up the Device Data
        void SetupData();
        bool Init(VkPhysicalDevice _Device);

        // Getters
        const PhysicalDeviceData &GetData() const { return _Data; }
        const VkPhysicalDeviceFeatures &GetFeatures() const { return _Features; }
        const VkPhysicalDevice GetDeviceHandle() const { return _DeviceHandle; }
        const UUID &GetID() const { return _ID; }

        const bool IsActive() const { return _Active; }
        void SetActiveStatus(bool _Status) { _Active = _Status; }
        const QueueFamilyIndices &GetQueueFamilyIndices() { return _Indicies; }

        operator VkPhysicalDevice() const { return _DeviceHandle; }

    private:
        // Stores the Data of the Physical Device
        PhysicalDeviceData _Data;
        VkPhysicalDeviceFeatures _Features;
        // In future maybe The Device Features too
        //   The main Device Handle
        VkPhysicalDevice _DeviceHandle = VK_NULL_HANDLE;
        // The Status which determines if the Device is to be used in rendering or not
        bool _Active = false;
        QueueFamilyIndices _Indicies;

        UUID _ID;
    };

    void _PrintDeviceInfo(const PhysicalDeviceData &Data);

    std::vector<PhysicalDevice> &_GetPhysicalDevices();
    bool _IsPhyscialDeviceSuitable(PhysicalDevice &_Devices, SurfaceKHR &_Surface, const std::vector<const char *> &_RequiredExtensions);

    bool _CheckPhysicalDeviceExetensions(PhysicalDevice &_Devices, const std::vector<const char *> &_ExtensionNames);
    void _GetPhysicalDeviceExetensions(std::vector<const char *> &_Extensions, PhysicalDevice &_Devices);

    uint32_t _GetPhysicalDeviceCount(VkInstance _Instance);
    void _GetPhysicalDevices(VkInstance _Instance, PhysicalDevice *_Devices, int DeviceCount);

    class ValidationLayersData;

    // The Software Logical Device Interface for the Physical Device(GPU)
    class Device
    {
    public:
        Device() {}
        ~Device();

        void Init(PhysicalDevice &_PDevice, const SurfaceKHR &_Surface, ValidationLayersData &_ValidationLayers);
        void Destroy();

        operator VkDevice() const { return _Device; }

        const UUID &GetID() const { return _ID; }
        const UUID &GetPhysicalDeviceID() const { return _PhysicalDeviceID; }
        VkDevice GetHandle() const { return _Device; }

    private:
        UUID _ID;
        // The ID that represents the ID of the PhysicalDevice that the Device Is Made for
        UUID _PhysicalDeviceID;

        VkDevice _Device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue _RenderQueue = VK_NULL_HANDLE;
    };
}
