#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/array_view.hpp>

#include <utils/logging.hpp>

namespace spx::vk
{

    class instance
    {

        public:
            inline  instance() = default;
            inline ~instance() = default;
        
            bool create(spx::array_view<const char *> required_extensions, spx::array_view<const char *> required_layers);
            void destroy();
            bool is_valid() const;

            inline operator VkInstance() const { return this->handle; }
        
        public:
            VkInstance handle = NULL;
            VkDebugUtilsMessengerEXT debug_messenger = NULL;
            VkApplicationInfo application_info = {};
            VkInstanceCreateInfo instance_create_info = {};


    };

}

