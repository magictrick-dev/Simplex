#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <utils/logging.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/array_view.hpp>
#include <simplex/array.hpp>

#include <scratch/renderer/vulkan/instance.hpp>
#include <scratch/renderer/vulkan/physical_device.hpp>

enum EngineResultType
{
    EngineResultType_OK,
    EngineResultType_NotInitialized,
    EngineResultType_InitializationFailed,
};

namespace spx::vk
{

    class vulkan_renderer
    {

        public:
            inline 
            vulkan_renderer()
            {

            }

            inline 
            ~vulkan_renderer()
            {

            }

            inline EngineResultType
            is_initialized() const
            {

                if (this->instance.is_valid())
                {
                    return EngineResultType_NotInitialized;
                }

                return EngineResultType_OK;

            }

            inline EngineResultType
            initialize(GLFWwindow *window)
            {

                try
                {

                    // Set our required extensions and create the vulkan instance.
                    uint32_t glfw_extension_count = 0;
                    const char **glfw_extensions_list = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

                    spx::dynamic_array<const char*> required_extensions;
                    for (uint32_t index = 0; index < glfw_extension_count; ++index)
                    {
                        required_extensions.emplace_back(glfw_extensions_list[index]);
                    }

#                   if defined(__APPLE__)
                        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#                   endif

                    this->instance.create(required_extensions);

                    // Select the physical device, we will let the "get_optimal_device" determine which
                    // physical device we should use.
                    this->physical_device = spx::vk::physical_device::get_optimal_device(this->instance);



                }
                catch (const std::runtime_error& error)
                {
                    spx::logger::dispatch_error_log("Vulkan Error: {}", error.what());
                    return EngineResultType_InitializationFailed;
                }

                return EngineResultType_OK;

            }

            inline void
            deinitialize()
            {

                this->instance.destroy();

            }

        private:
            spx::vk::instance instance;
            spx::vk::physical_device physical_device;

            struct
            {
                VkPhysicalDevice physical;
                VkDevice logical;
            } main_device;

        private:
            GLFWwindow *window;

    };

}