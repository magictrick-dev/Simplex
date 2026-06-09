#pragma once
#include <utils/defs.hpp>
#include <vulkan/vulkan.h>

namespace spx::vk
{

    class queue_family
    {

        public:
            inline  queue_family() = default;
            inline ~queue_family() = default;

            inline 
            queue_family(size_t index, const VkQueueFamilyProperties &properties)
                : properties(properties), index(index) 
            {

            }

            inline bool has_flags(VkQueueFlags flags) const { return properties.queueFlags & flags; }
            inline bool has_graphics_bit() const    { return properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;     }
            inline bool has_compute_bit() const     { return properties.queueFlags & VK_QUEUE_COMPUTE_BIT;      }
            inline bool has_transfer_bit() const    { return properties.queueFlags & VK_QUEUE_TRANSFER_BIT;     }

        public:
            size_t index;
            VkQueueFamilyProperties properties;


    };

}
