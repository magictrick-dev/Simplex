#pragma once
#include <utils/defs.hpp>
#include <vulkan/vulkan.h>
#include <simplex/platform/window.hpp>
#include <simplex/renderer/renderer_utils.hpp>
#include <simplex/renderer/vulkan/instance.hpp>

namespace spx::vk
{

    class surface
    {

        public:
            inline  surface() = default;
            inline ~surface() = default;

            void create(spx::vk::instance instance, spx::window_interface *window);
            void destroy();

            inline operator VkSurfaceKHR() { return this->handle; }

        private:
            spx::vk::instance instance;
            VkSurfaceKHR handle;

    };

}
