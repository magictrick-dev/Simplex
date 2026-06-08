#pragma once
#include <vulkan/vulkan.h>

#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>


#include <scratch/renderer/vulkan/instance.hpp>
#include <scratch/renderer/vulkan/queue_family.hpp>

namespace spx::vk
{

    class physical_device 
    {

        public:
            inline  physical_device() = default;
            inline ~physical_device() = default;

            inline 
            physical_device(VkInstance vulkan_instance, VkPhysicalDevice physical_device) 
                : device(physical_device)
            {


                // Gets the physical device properties.
                vkGetPhysicalDeviceProperties(this->device, &this->device_properties_1);
                vkGetPhysicalDeviceMemoryProperties(this->device, &this->memory_properties);

                this->driver_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
                this->device_properties_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                this->device_properties_2.pNext = &this->driver_properties;

                vkGetPhysicalDeviceProperties2(this->device, &this->device_properties_2);

                // Gets the queue families and stores them in a neat structure.
                uint32_t queue_families_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(this->device, &queue_families_count, NULL);

                spx::dynamic_array<VkQueueFamilyProperties> queue_family_properties(queue_families_count);
                vkGetPhysicalDeviceQueueFamilyProperties(this->device, &queue_families_count, queue_family_properties.begin());

                for (size_t i = 0; i < queue_family_properties.size(); ++i)
                {
                    this->queue_families.emplace_back(i, queue_family_properties[i]);
                }

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

            inline int64_t
            get_device_score() const
            {
                
                // NOTE(Chris): We will need some sort of mechanism to score the GPU
                //              on the system. The easiest way to score a device is just
                //              look at the device's memory. We will use that for now.
                int64_t device_score = 0;
                device_score += memory_properties.memoryHeaps[0].size;
                
                return device_score;
            }

            inline operator VkPhysicalDevice() 
            { 
                return this->device; 
            }

            inline uint32_t
            get_queue_family_index_with(VkQueueFlags flags) const
            {

                for (const auto& family : this->queue_families)
                {
                    if (family.has_flags(flags))
                    {
                        return family.index;
                    }
                }

                throw std::runtime_error("Failed to find queue family with required flags.");
                return 0;

            }



        public:
            static inline spx::array_view<spx::vk::physical_device>
            get_physical_devices(spx::vk::instance instance)
            {

                static bool initialized = false;
                static spx::dynamic_array<spx::vk::physical_device> physical_devices;
                if (initialized == true) return physical_devices;
                initialized = true;

                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(instance, &device_count, NULL);

                spx::dynamic_array<VkPhysicalDevice> devices(device_count);
                vkEnumeratePhysicalDevices(instance, &device_count, devices.begin());
                for (auto device : devices)
                {
                    physical_devices.emplace_back(instance, device);
                }

                return physical_devices;

            }

            static inline spx::vk::physical_device
            get_optimal_device(spx::vk::instance instance)
            {

                auto physical_devices = spx::vk::physical_device::get_physical_devices(instance);

                spx::vk::physical_device optimal_device;
                int64_t maximum_score = 0;
                for (auto &device : physical_devices)
                {
                    const int64_t device_score = device.get_device_score();
                    if (device_score > maximum_score)
                    {
                        maximum_score = device_score;
                        optimal_device = device;
                    }
                }

                return optimal_device;

            }

        public:
            VkPhysicalDevice device = NULL;
            VkPhysicalDeviceProperties device_properties_1 = {};
            VkPhysicalDeviceProperties2 device_properties_2 = {};
            VkPhysicalDeviceDriverProperties driver_properties = {};
            VkPhysicalDeviceMemoryProperties memory_properties = {};
            spx::dynamic_array<spx::vk::queue_family> queue_families;

    };

}