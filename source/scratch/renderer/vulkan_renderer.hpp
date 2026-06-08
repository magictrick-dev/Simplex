#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <utils/logging.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/array_view.hpp>
#include <simplex/array.hpp>

#include <scratch/renderer/vulkan/instance.hpp>
#include <scratch/renderer/vulkan/physical_devices.hpp>

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

                    // Create the instance.
                    uint32_t glfw_extension_count = 0;
                    const char **glfw_extensions_list = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
                    spx::array_view<const char *> glfw_required_extensions { glfw_extensions_list, glfw_extension_count };

                    this->instance.create(glfw_required_extensions);

                    // Now get the physical device.
                    this->get_physical_device();

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
            inline void 
            get_physical_device()
            {

                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(this->instance, &device_count, NULL);

                if (device_count == 0)
                {
                    throw std::runtime_error("Failed to get a viable physical device.");
                }

                if (device_count > 32)
                {
                    // TODO(Chris): Fall back to a dynamic_array instead.
                    //              Fairly unlikely that there is more than 32 physical devices on a system.
                    throw std::runtime_error("Way too many physical devices!");
                }

                spx::static_array<VkPhysicalDevice, 32> devices { device_count };
                vkEnumeratePhysicalDevices(this->instance, &device_count, devices.begin());

                int64_t max_device_score = 0;
                for (auto &device : devices)
                {

                    const spx::vk::vulkan_physical_device current { this->instance, device };
                    spx::logger::dispatch_diagnostic_log("Vulkan physical device found: {}", current.get_qualified_name().data());

                    const size_t current_score = current.get_device_score();
                    if (current_score > max_device_score)
                    {
                        max_device_score = current_score;
                        this->physical_device = current;
                    }

                }

                if (max_device_score < 0)
                {
                    throw std::runtime_error("No suitable vulkan physical device found.");
                }

            }

        private:
            spx::vk::vulkan_instance instance;
            spx::vk::vulkan_physical_device physical_device;

            struct
            {
                VkPhysicalDevice physical;
                VkDevice logical;
            } main_device;

        private:
            GLFWwindow *window;

    };

}