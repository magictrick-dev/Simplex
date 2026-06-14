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

            inline bool has_flags(VkQueueFlags flags, bool required_presentation = false) const 
            { 
                if (required_presentation == true && supports_presentation == false) return false;
                return properties.queueFlags & flags; 
            }

            inline bool has_graphics_bit() const    { return properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;     }
            inline bool has_compute_bit() const     { return properties.queueFlags & VK_QUEUE_COMPUTE_BIT;      }
            inline bool has_transfer_bit() const    { return properties.queueFlags & VK_QUEUE_TRANSFER_BIT;     }
            inline bool has_presentation_capabilities() const { return this->supports_presentation; }

        public:
            size_t index;
            bool supports_presentation;
            VkQueueFamilyProperties properties;


    };

}

