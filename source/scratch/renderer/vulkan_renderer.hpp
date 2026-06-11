#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <utils/logging.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/array_view.hpp>
#include <simplex/array.hpp>

#include <scratch/renderer/renderer_interface.hpp>
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

    class vulkan_renderer : public renderer_interface
    {

        public:
            inline  vulkan_renderer() { }
            inline ~vulkan_renderer() { }

        protected:
            inline virtual RendererResultType 
            internal_initialize() override 
            {

                // Get the window and the required instance extensions for the window and
                // append them as our default list of extensions.
                spx::window_interface *window = this->get_window();

                uint32_t instance_extensions_count = 0;
                const char **instance_extensions = NULL;
                window->get_vulkan_instance_extensions(&instance_extensions, &instance_extensions_count);

                spx::dynamic_array<const char*> required_extensions;
                for (uint32_t index = 0; index < instance_extensions_count; ++index)
                {
                    required_extensions.emplace_back(instance_extensions[index]);
                }

                // Add additional instance extensions as needed.
                required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#               if defined(__APPLE__)
                    required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#               endif

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
#               if defined(__APPLE__)
                    required_device_extensions.emplace_back("VK_KHR_portability_subset");
#               endif

                this->logical_device.create(this->instance, this->physical_device, required_device_extensions);

                return RendererResultType_OK;

            }

            inline virtual RendererResultType 
            internal_frame_begin() override 
            {

                return RendererResultType_OK;
            }
            
            inline virtual RendererResultType 
            internal_render_begin() override 
            {

                return RendererResultType_OK;
            }

            inline virtual RendererResultType 
            internal_render_end() override 
            {

                return RendererResultType_OK;
            }

            inline virtual RendererResultType 
            internal_frame_end() override 
            {

                return RendererResultType_OK;
            }

            inline virtual RendererResultType 
            internal_deinitialize() override 
            {

                this->logical_device.destroy();
                this->instance.destroy();

                return RendererResultType_OK;

            }

        private:
            spx::vk::instance instance;
            spx::vk::physical_device physical_device;
            spx::vk::logical_device logical_device;

    };

}