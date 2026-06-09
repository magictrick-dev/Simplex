#pragma once
#include <utils/defs.hpp>
#include <vulkan/vulkan.h>

#include <scratch/renderer/vulkan/instance.hpp>
#include <scratch/renderer/vulkan/physical_device.hpp>

namespace spx::vk
{

    class logical_device
    {

        public:
            inline  logical_device() = default;
            inline ~logical_device() = default;

            inline void 
            create(spx::vk::instance instance, spx::vk::physical_device physical_device)
            {

                uint32_t flags_needed = VK_QUEUE_GRAPHICS_BIT; // Just graphics bit for now.
                uint32_t queue_index = physical_device.get_queue_family_index_with(flags_needed);
                real32_t priority = 1.0f; // Normalized queue priority, 1.0f is highest.

                VkDeviceQueueCreateInfo queue_create_info = {};
                queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_create_info.queueFamilyIndex = queue_index;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &priority;

                this->device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                this->device_11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                this->device_12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                this->device_13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                this->device_14_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;

                void *features_chain = NULL;
                if (physical_device.supports_version(VK_API_VERSION_1_4))
                {
                    this->device_14_features.pNext = features_chain;
                    features_chain = &this->device_14_features;
                }

                if (physical_device.supports_version(VK_API_VERSION_1_3))
                {
                    this->device_13_features.pNext = features_chain;
                    features_chain = &this->device_13_features;
                }

                if (physical_device.supports_version(VK_API_VERSION_1_2))
                {
                    this->device_12_features.pNext = features_chain;
                    features_chain = &this->device_12_features;
                }

                if (physical_device.supports_version(VK_API_VERSION_1_1))
                {
                    this->device_11_features.pNext = features_chain;
                    features_chain = &this->device_11_features;
                }

                this->device_features.pNext = features_chain;

                // NOTE(Chris): We opt-in to only what we need as the validation layers catch it
                //              rather than what is actually available.
                //vkGetPhysicalDeviceFeatures2(physical_device, &device_features);

                VkDeviceCreateInfo device_create_info = {};
                device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                device_create_info.queueCreateInfoCount = 1;
                device_create_info.pQueueCreateInfos = &queue_create_info;
                device_create_info.enabledExtensionCount = 0;
                device_create_info.ppEnabledExtensionNames = NULL;
                device_create_info.enabledLayerCount = 0;
                device_create_info.ppEnabledLayerNames = NULL;
                device_create_info.pEnabledFeatures = NULL;
                device_create_info.pNext = &this->device_features;

                // Create the device now.
                VkResult result = vkCreateDevice(physical_device, &device_create_info, NULL, &this->device);
                if (result != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create logical device.");
                }

                // Then fetch the queue we created.
                vkGetDeviceQueue(this->device, queue_index, 0, &this->graphics_queue);

            }

            inline void 
            destroy()
            {

                vkDestroyDevice(this->device, NULL);
                this->device = NULL;

            }

            inline operator VkDevice() { return this->device; }

        public:
            VkDevice device = NULL;
            VkQueue graphics_queue = NULL;
            VkPhysicalDeviceFeatures2 device_features = {};
            VkPhysicalDeviceVulkan11Features device_11_features = {};
            VkPhysicalDeviceVulkan12Features device_12_features = {};
            VkPhysicalDeviceVulkan13Features device_13_features = {};
            VkPhysicalDeviceVulkan14Features device_14_features = {};

    };

}