#pragma once
#include <utils/defs.hpp>
#include <vulkan/vulkan.h>

#include <simplex/renderer/vulkan/instance.hpp>
#include <simplex/renderer/vulkan/physical_device.hpp>
#include <simplex/array_view.hpp>

namespace spx::vk
{

    class logical_device
    {

        public:
            inline  logical_device() = default;
            inline ~logical_device() = default;

            void create(spx::vk::instance instance, 
                        spx::vk::physical_device physical_device,
                        spx::array_view<const char*> required_extensions);

            void destroy();

            inline operator VkDevice() { return this->device; }

        public:
            VkDevice device = NULL;
            VkQueue graphics_queue = NULL;
            VkPhysicalDeviceFeatures2           device_10_features = {};
            VkPhysicalDeviceVulkan11Features    device_11_features = {};
            VkPhysicalDeviceVulkan12Features    device_12_features = {};
            VkPhysicalDeviceVulkan13Features    device_13_features = {};
            VkPhysicalDeviceVulkan14Features    device_14_features = {};

    };

}
