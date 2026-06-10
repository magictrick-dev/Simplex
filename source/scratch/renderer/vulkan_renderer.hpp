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
#include <scratch/renderer/vulkan/logical_device.hpp>

#include <string>
#include <iomanip>
#include <sstream>

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

                    required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#                   if defined(__APPLE__)
                        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#                   endif

                    spx::dynamic_array<const char*> required_layers;
                    required_layers.emplace_back("VK_LAYER_KHRONOS_validation");

                    this->instance.create(required_extensions, required_layers);

                    // Select the physical device, we will let the "get_optimal_device" determine which
                    // physical device we should use.
                    this->physical_device = spx::vk::physical_device::get_optimal_device(this->instance);
                    spx::logger::dispatch_diagnostic_log(
                        "Selected {} as the physical device.",
                        this->physical_device.get_qualified_name().c_str()
                    );

                    // Create the logical device now.
                    spx::dynamic_array<const char*> required_device_extensions;
#                   if defined(__APPLE__)
                        required_device_extensions.emplace_back("VK_KHR_portability_subset");
#                   endif

                    this->logical_device.create(this->instance, this->physical_device, required_device_extensions);


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

                this->logical_device.destroy();
                this->instance.destroy();

            }

        private:

        private:
            spx::vk::instance instance;
            spx::vk::physical_device physical_device;
            spx::vk::logical_device logical_device;

        private:
            GLFWwindow *window;

    };

}