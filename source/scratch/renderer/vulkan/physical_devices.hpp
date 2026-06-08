#pragma once
#include <vulkan/vulkan.h>
#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>

#include <scratch/renderer/vulkan/instance.hpp>

namespace spx::vk
{

    class vulkan_physical_device
    {

        public:
            inline  vulkan_physical_device() = default;
            inline ~vulkan_physical_device() = default;

            inline 
            vulkan_physical_device(VkInstance vulkan_instance, VkPhysicalDevice physical_device) 
                : device(physical_device)
            {


                vkGetPhysicalDeviceProperties(this->device, &this->device_properties_1);
                vkGetPhysicalDeviceMemoryProperties(this->device, &this->memory_properties);

                this->driver_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
                this->device_properties_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                this->device_properties_2.pNext = &this->driver_properties;

                vkGetPhysicalDeviceProperties2(this->device, &this->device_properties_2);

            }

            inline spx::string_view<char> 
            get_device_name() const
            {
                return this->device_properties_1.deviceName;
            }

            inline spx::string_view<char>
            get_driver_name() const
            {
                return this->driver_properties.driverName;
            }

            inline spx::string_view<char>
            get_driver_version() const
            {
                return this->driver_properties.driverInfo;
            }

            inline spx::dynamic_string<char>
            get_qualified_name() const
            {
                spx::dynamic_string<char> result;
                result += this->get_device_name();
                result += " ";
                result += this->get_driver_name();
                result += " ";
                result += this->get_driver_version();
                return std::move(result);
            }

            inline size_t
            get_device_score() const
            {
                
                // NOTE(Chris): We will need some sort of mechanism to score the GPU
                //              on the system. The easiest way to score a device is just
                //              look at the device's memory. We will use that for now.
                int64_t device_score = 0;
                device_score += memory_properties.memoryHeaps[0].size;
                
                return device_score;
            }

        public:
            static inline spx::array_view<spx::vk::vulkan_physical_device>
            get_physical_devices(spx::vk::vulkan_instance instance)
            {

                static bool initialized = false;
                static spx::dynamic_array<spx::vk::vulkan_physical_device> physical_devices;
                if (initialized == true) return physical_devices;
                initialized = true;

                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(instance, &device_count, NULL);

                return physical_devices;

            }

        public:
            VkPhysicalDevice device = NULL;
            VkPhysicalDeviceProperties device_properties_1 = {};
            VkPhysicalDeviceProperties2 device_properties_2 = {};
            VkPhysicalDeviceDriverProperties driver_properties = {};
            VkPhysicalDeviceMemoryProperties memory_properties = {};

    };

}